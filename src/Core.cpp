#include "PCH.h"
#include "Core.h"
#include "Settings.h"
#include "SHA256.h"
#include "UserKey.h"
#include "AvatarConfigScanner.h"
#include "Path.h"
#include "AutoStart.h"

#include <iostream>
#include <cmath>
#include <string>
#include <thread>
#include <memory>
#include <array>
#include <map>
#include <vector>
#include <chrono>

static auto LoadTrayIcon() -> Tray::Icon
{
	const auto iconPath = Path::GetExeDir() / "icon.ico";
#if _WIN32
	// Load with the wide-char API so non-ASCII install paths work
	return Tray::Icon(static_cast<HICON>(LoadImageW(nullptr, iconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE)));
#else
	return Tray::Icon(iconPath.string());
#endif
}

Core::Core() :
	tray("Shell Protector OSC", LoadTrayIcon())
{
	bAutoStart = AutoStart::IsEnabled();

	Settings settings;
	if (!settings.Load())
		osc.AddLog("Can't load save file");
	else
	{
		for (int i = 0; i < settings.password.size(); ++i)
		{
			password[i] = settings.password[i];
		}
		keyIdx = settings.keyIdx;
		port = settings.port;
		bParameterMultiplexing = settings.bParameterMultiplexing;
		refreshRate = settings.refreshRate;
		bStartAndHide = settings.bStartAndHide;
		if (bStartAndHide)
			bHideWindow = true;
	}
	std::cout << "Load save file\n";
}

Core::~Core()
{
	tray.exit();

	if (bSave)
	{
		Settings settings;
		settings.password = password;
		settings.keyIdx = keyIdx;
		settings.port = port;
		settings.bParameterMultiplexing = bParameterMultiplexing;
		settings.refreshRate = refreshRate;
		settings.bStartAndHide = bStartAndHide;
		settings.Save();
	}

	if(oscThread.joinable())
		oscThread.join();

	std::cout << "End\n";
}

void Core::Init()
{
	std::cout << "Start...\n";
	osc.Init("127.0.0.1", port);
	std::cout << "OSC Init\n";

	StartOSCThread();
	InitTray();

	if (bStartAndHide)
		StartOSC();
}

void Core::StartOSC()
{
	bStart.store(true, std::memory_order_release);
}
void Core::StopOSC()
{
	bStart.store(false, std::memory_order_release);
}

void Core::Shutdown()
{
	bStop.store(true, std::memory_order_release);
	bStart.store(false, std::memory_order_release);
}

auto Core::LockSettings() -> std::unique_lock<std::mutex>
{
	return std::unique_lock<std::mutex>(settingsMutex);
}

auto Core::GetOSC() const -> const OSC&
{
	return osc;
}
auto Core::GetOSC() -> OSC&
{
	return osc;
}
bool Core::IsStartAndHide() const
{
	return bStartAndHide;
}
bool Core::IsShowLog() const
{
	return bShowLog;
}
bool Core::IsStarting() const
{
	return bStart.load(std::memory_order_acquire);
}
bool Core::IsFinish() const
{
	return bStop.load(std::memory_order_acquire);
}
bool Core::IsHideWindow() const
{
	return bHideWindow;
}
int Core::GetProtectedAvatarCount() const
{
	return protectedAvatarCount.load(std::memory_order_acquire);
}

namespace
{
	// The parameters one avatar listens to, and the key bytes to send to them
	struct Target
	{
		std::string lockAddr;
		std::array<std::string, 4> switchAddrs;
		std::string multiplexedKeyAddr;
		std::vector<std::string> keyAddrs; // Without multiplexing, one per key byte
		std::vector<uint8_t> keyBytes;
	};

	auto MakeTarget(const std::string& prefix, const UserKey& key, int keyLen) -> Target
	{
		Target target;
		target.lockAddr = prefix + key.ObfuscateParameter("encrypt_lock");
		for (int bit = 0; bit < static_cast<int>(target.switchAddrs.size()); ++bit)
			target.switchAddrs[bit] = prefix + key.ObfuscateParameter("encrypt_switch" + std::to_string(bit));
		target.multiplexedKeyAddr = prefix + key.ObfuscateParameter("pkey");
		for (int i = 0; i < keyLen; ++i)
		{
			target.keyAddrs.push_back(prefix + key.ObfuscateParameter("saved_key" + std::to_string(i)));
			target.keyBytes.push_back(key.GetKeyByte(i));
		}
		return target;
	}

	// Avatars encrypted with ShellProtector 2.7.0 or earlier: plain names and key = password ^ SHA256(password)
	auto MakeLegacyTarget(const std::string& prefix, const std::string& password, int keyLen) -> Target
	{
		Target target;
		target.lockAddr = prefix + "encrypt_lock";
		for (int bit = 0; bit < static_cast<int>(target.switchAddrs.size()); ++bit)
			target.switchAddrs[bit] = prefix + "encrypt_switch" + std::to_string(bit);
		target.multiplexedKeyAddr = prefix + "pkey";

		SHA256 sha;
		sha.update(password);
		std::unique_ptr<uint8_t[]> digest(sha.digest());
		for (int i = 0; i < keyLen; ++i)
		{
			char c = static_cast<std::size_t>(i) < password.size() ? password[i] : 0; // Characters after the terminator count as 0
			target.keyAddrs.push_back(prefix + "pkey" + std::to_string(i));
			target.keyBytes.push_back(static_cast<uint8_t>(c ^ digest[i]));
		}
		return target;
	}

	auto EncodeKeyByte(uint8_t value) -> float
	{
		const float factor = 10000.0f;
		float pwd = 1 - value / 128.0f;
		return -(roundf(pwd * factor) / factor); //Rounding to 4 digits
	}
}

void Core::StartOSCThread()
{
	oscThread = std::thread([&]
	{
		const std::string paramPrefix = "/avatar/parameters/";
		AvatarConfigScanner scanner;
		// Derived keys by salt. PBKDF2 is slow on purpose, so keep them until the password changes.
		std::map<std::string, UserKey> userKeys;
		std::string derivedKey;
		int derivedKeyLen = -1;

		while (!bStop.load(std::memory_order_acquire))
		{
			if (bStart.load(std::memory_order_acquire))
			{
				// Snapshot the settings so the UI thread can edit them while sending
				std::string key;
				int keyLen, rate, oscPort;
				bool multiplexing;
				{
					std::lock_guard<std::mutex> lock(settingsMutex);
					key = password; // Up to the null terminator
					keyLen = keyLength;
					rate = refreshRate;
					oscPort = port;
					multiplexing = bParameterMultiplexing;
				}

				if (key != derivedKey || keyLen != derivedKeyLen)
				{
					userKeys.clear();
					derivedKey = key;
					derivedKeyLen = keyLen;
				}

				std::vector<Target> targets;
				for (const auto& salt : scanner.Scan())
				{
					auto it = userKeys.find(salt);
					if (it == userKeys.end())
					{
						const auto start = std::chrono::steady_clock::now();
						it = userKeys.emplace(salt, UserKey::Derive(key, keyLen, salt)).first;
						const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
						osc.AddLog("Derived the key for avatar salt " + salt.substr(0, 8) + "... (" + std::to_string(ms) + "ms)");
					}
					targets.push_back(MakeTarget(paramPrefix, it->second, keyLen));
				}
				protectedAvatarCount.store(static_cast<int>(targets.size()), std::memory_order_release);
				targets.push_back(MakeLegacyTarget(paramPrefix, key, keyLen));

				osc.SetOSCPort(oscPort);

				for (int i = 0; i < keyLen; ++i)
				{
					if (multiplexing)
					{
						for (const auto& target : targets)
						{
							osc.SendOSC(target.lockAddr, true);
							// Switch bit b selects key byte i; 4/8/16 byte keys use 2/3/4 switches
							const int switchCount = keyLen <= 4 ? 2 : (keyLen <= 8 ? 3 : 4);
							for (int bit = switchCount - 1; bit >= 0; --bit)
								osc.SendOSC(target.switchAddrs[bit], ((i >> bit) & 1) == 1);
						}
					}
					///////////////////Send password////////////////
					for (const auto& target : targets)
					{
						osc.AddLog(std::to_string(i) + ":" + std::to_string(target.keyBytes[i]));
						osc.SendOSC(multiplexing ? target.multiplexedKeyAddr : target.keyAddrs[i], EncodeKeyByte(target.keyBytes[i]));
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(rate));
					/////////////////////////////////////////////////
					if (multiplexing)
					{
						for (const auto& target : targets)
							osc.SendOSC(target.lockAddr, false);
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			else
				std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	});
	std::cout << "Thread Init\n";
}

void Core::InitTray()
{
	tray.addEntry(Tray::Button("Show window", [&]()
	{
		bHideWindow = false;
	}));

	tray.addEntry(Tray::Button("Exit", [&]()
	{
		Shutdown();
	}));
	std::thread trayThread = std::thread([&]()
	{
		tray.run();
	});
	trayThread.detach();
}
