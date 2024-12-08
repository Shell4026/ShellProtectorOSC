#pragma once
#include "OSC.h"
#include "Load.h"
#include "Save.h"

#include "tray.hpp"

#include <thread>
class Core
{
private:
	std::atomic_bool stop = false;
	std::atomic_bool start = false;
	std::atomic_bool hideWindow = false;

	OSC osc;
	Load loader;
	Save saver;

	std::thread oscThread;
	std::thread trayThread;

	Tray::Tray tray;
public:
	int keyIdx = 0;
	int keyLength = 4;
	char password[100] = "";
	int refreshRate = 150;
	int port = 9000;

	bool bShowLog = false;
	bool bParameterMultiplexing = false;
	bool bSave = true;
	bool bStartAndHide = false;
	bool bHideWindow = false;
private:
	void StartOSCThread();
	void InitTray();
public:
	Core();
	~Core();

	void Init();
	void StartOSC();
	void StopOSC();
	void Shutdown();

	auto GetOSC() const -> const OSC&;
	auto GetOSC() -> OSC&;

	bool IsStartAndHide() const;
	bool IsShowLog() const;
	bool IsStarting() const;
	bool IsFinish() const;
	bool IsHideWindow() const;
};