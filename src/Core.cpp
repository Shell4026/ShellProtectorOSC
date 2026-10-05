#include "PCH.h"
#include "Core.h"
#include "Settings.h"
#include "SHA256.h"
#include "ParameterObfuscator.h"
#include "Path.h"
#include "AutoStart.h"

#include <iostream>
#include <cmath>
#include <string>
#include <thread>
#include <memory>
#include <array>

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

void Core::StartOSCThread()
{
	oscThread = std::thread([&]
	{
		const float factor = pow(10.0f, 4);
		const std::string paramPrefix = "/avatar/parameters/";
		std::string oscAddr = "";
		std::string lockAddr = "";
		std::array<std::string, 4> switchAddrs;

		// Sends bit `bit` of n to encrypt_switch<bit>
		auto sendSwitch = [&](int bit, int n)
			{
				osc.SendOSC(switchAddrs[bit], ((n >> bit) & 1) == 1);
			};

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

				ParameterObfuscator obfuscator(key);
				lockAddr = paramPrefix + obfuscator.Obfuscate("encrypt_lock");
				for (int bit = 0; bit < static_cast<int>(switchAddrs.size()); ++bit)
					switchAddrs[bit] = paramPrefix + obfuscator.Obfuscate("encrypt_switch" + std::to_string(bit));

				osc.SetOSCPort(oscPort);
				SHA256 sha;
				sha.update(key);
				std::unique_ptr<uint8_t[]> digest(sha.digest());

				for (int i = 0; i < keyLen; ++i)
				{
					if (multiplexing)
					{
						osc.SendOSC(lockAddr, true);
						switch (keyLen)
						{
						default: //fall down
							sendSwitch(3, i);
						case 8:
							sendSwitch(2, i);
						case 4:
							sendSwitch(1, i);
							sendSwitch(0, i);
							break;
						}
					}
					///////////////////Send password////////////////
					char c = static_cast<std::size_t>(i) < key.size() ? key[i] : 0; // Characters after the terminator count as 0
					float pwd;
					unsigned char var = c ^ digest[i];
					osc.AddLog(std::to_string(i) + ":" + std::to_string(var));
					pwd = 1 - var / 128.0f;
					pwd = -(roundf(pwd * factor) / factor); //Rounding to 4 digits

					if (multiplexing)
						oscAddr = paramPrefix + obfuscator.Obfuscate("pkey");
					else
						oscAddr = paramPrefix + "pkey" + std::to_string(i);
					osc.SendOSC(oscAddr, pwd);
					std::this_thread::sleep_for(std::chrono::milliseconds(rate));
					/////////////////////////////////////////////////
					if (multiplexing)
						osc.SendOSC(lockAddr, false);
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
