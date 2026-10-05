#pragma once

// Registers the program under HKCU\...\CurrentVersion\Run so it starts on Windows login.
class AutoStart
{
public:
	static bool IsEnabled();
	static bool SetEnabled(bool enable);
};
