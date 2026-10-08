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
	bool IsShowAdvanced() const;
	bool IsStarting() const;
	bool IsFinish() const;
	bool IsHideWindow() const;
	// Avatars whose salt was found in the VRChat OSC configs
	int GetProtectedAvatarCount() const;

	// Every key byte comes from the password (ShellProtector.KeySize on the Unity side)
	static constexpr int KEY_LENGTH = 16;
private:
	void StartOSCThread();
	void InitTray();
public:
	char password[KEY_LENGTH + 1] = "";
	int refreshRate = 150;
	int port = 9000;
	char ip[64] = "127.0.0.1";

	bool bShowLog = false;
	bool bShowAdvanced = false;
	bool bParameterMultiplexing = true;
	bool bSave = true;
	bool bStartAndHide = false;
	bool bHideWindow = false;
	bool bAutoStart = false;
private:
	std::atomic_bool bStop = false;
	std::atomic_bool bStart = false;
	std::atomic_int protectedAvatarCount = 0;

	std::mutex settingsMutex;

	OSC osc;

	std::thread oscThread;

	Tray::Tray tray;
};
