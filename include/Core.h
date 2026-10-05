#pragma once
#include "OSC.h"

#include "tray.hpp"

#include <thread>
#include <mutex>
#include <atomic>
class Core
{
public:
	Core();
	~Core();

	void Init();
	void StartOSC();
	void StopOSC();
	void Shutdown();

	auto LockSettings() -> std::unique_lock<std::mutex>;

	auto GetOSC() const -> const OSC&;
	auto GetOSC() -> OSC&;

	bool IsStartAndHide() const;
	bool IsShowLog() const;
	bool IsStarting() const;
	bool IsFinish() const;
	bool IsHideWindow() const;
private:
	void StartOSCThread();
	void InitTray();
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
	bool bAutoStart = false;
private:
	std::atomic_bool bStop = false;
	std::atomic_bool bStart = false;

	std::mutex settingsMutex;

	OSC osc;

	std::thread oscThread;

	Tray::Tray tray;
};
