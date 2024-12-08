#include "PCH.h"
#include "Core.h"
#include "SHA256.h"

#include <iostream>
#include <cmath>
#include <string>
#include <thread>

void Core::StartOSCThread()
{
	oscThread = std::thread([&]
	{
		const float factor = pow(10.0f, 4);
		std::string osc_addr = "";

		auto switch0 = [&](int n)
			{
				bool s = false;
				osc_addr = "/avatar/parameters/encrypt_switch0";
				s = (n & 0b0001) == 1 ? true : false;
				osc.SendOSC(osc_addr, s);
			};
		auto switch1 = [&](int n)
			{
				bool s = false;
				osc_addr = "/avatar/parameters/encrypt_switch1";
				s = (n & 0b0010) == 2 ? true : false;
				osc.SendOSC(osc_addr, s);
			};
		auto switch2 = [&](int n)
			{
				bool s = false;
				osc_addr = "/avatar/parameters/encrypt_switch2";
				s = (n & 0b0100) == 4 ? true : false;
				osc.SendOSC(osc_addr, s);
			};
		auto switch3 = [&](int n)
			{
				bool s = false;
				osc_addr = "/avatar/parameters/encrypt_switch3";
				s = (n & 0b1000) == 8 ? true : false;
				osc.SendOSC(osc_addr, s);
			};

		while (!stop.load(std::memory_order_acquire))
		{
			if (start.load(std::memory_order_acquire))
			{
				osc.SetOSCPort(port);
				SHA256 sha;
				sha.update(password);
				uint8_t* digest = sha.digest();

				for (int i = 0; i < keyLength; ++i)
				{
					if (bParameterMultiplexing)
					{
						std::string osc_addr = "/avatar/parameters/encrypt_lock";
						osc.SendOSC(osc_addr, true);
						switch (keyLength)
						{
						default: //fall down
							switch3(i);
						case 8:
							switch2(i);
						case 4:
							switch1(i);
							switch0(i);
							break;
						}
					}
					///////////////////Send password////////////////
					if (password[i] == 0)
					{
						for (int j = i + 1; j < sizeof(password); ++j)
							password[j] = 0;
					}
					float pwd;
					unsigned char var = password[i] ^ digest[i];
					osc.AddLog(std::to_string(i) + ":" + std::to_string(var));
					pwd = 1 - var / 128.0f;
					pwd = -(roundf(pwd * factor) / factor); //Rounding to 4 digits

					if (bParameterMultiplexing)
						osc_addr = "/avatar/parameters/pkey";
					else
						osc_addr = "/avatar/parameters/pkey" + std::to_string(i);
					osc.SendOSC(osc_addr, pwd);
					std::this_thread::sleep_for(std::chrono::milliseconds(refreshRate));
					/////////////////////////////////////////////////
					if (bParameterMultiplexing)
					{
						osc_addr = "/avatar/parameters/encrypt_lock";
						osc.SendOSC(osc_addr, false);
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
	std::thread tray_thread = std::thread([&]()
	{
		tray.run();
	});
	tray_thread.detach();
}

Core::Core() :
	tray("Shell Protector OSC", "icon.ico")
{
	Load loader;
	if (!loader.LoadFile())
		osc.AddLog("Can't load save file");
	else
	{
		for (int i = 0; i < loader.password.size(); ++i)
		{
			password[i] = loader.password[i];
		}
		keyIdx = loader.keyIdx;
		port = loader.port;
		bParameterMultiplexing = loader.bParameterMultiplexing;
		refreshRate = loader.refreshRate;
		bStartAndHide = loader.bStartAndHide;
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
		saver.password = password;
		saver.keyIdx = keyIdx;
		saver.port = port;
		saver.bParameterMultiplexing = bParameterMultiplexing;
		saver.refreshRate = refreshRate;
		saver.bStartAndHide = bStartAndHide;
		saver.SaveFile();
	}

	if(oscThread.joinable())
		oscThread.join();

	std::cout << "End\n";
}

void Core::Init()
{
	std::cout << "Start...\n";
	OSC osc;
	osc.Init("127.0.0.1", port);
	std::cout << "OSC Init\n";

	StartOSCThread();
	InitTray();

	if (bStartAndHide)
		StartOSC();
}

void Core::StartOSC()
{
	start.store(true, std::memory_order_release);
}
void Core::StopOSC()
{
	start.store(false, std::memory_order_release);
}

void Core::Shutdown()
{
	stop.store(true, std::memory_order_release);
	start.store(false, std::memory_order_release);
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
	return start.load(std::memory_order_acquire);
}
bool Core::IsFinish() const
{
	return stop.load(std::memory_order_acquire);
}
bool Core::IsHideWindow() const
{
	return bHideWindow;
}