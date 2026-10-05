#include "PCH.h"
#include "AutoStart.h"
#include "Path.h"

#if _WIN32
static constexpr const wchar_t* RUN_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static constexpr const wchar_t* VALUE_NAME = L"ShellProtectorOSC";

static auto GetCommandLineValue() -> std::wstring
{
	return L"\"" + Path::GetExePath().wstring() + L"\"";
}

bool AutoStart::IsEnabled()
{
	wchar_t value[1024];
	DWORD size = sizeof(value);
	if (RegGetValueW(HKEY_CURRENT_USER, RUN_KEY, VALUE_NAME, RRF_RT_REG_SZ, nullptr, value, &size) != ERROR_SUCCESS)
		return false;
	// Treat as disabled if the registered path points to another (e.g. moved) exe
	return _wcsicmp(value, GetCommandLineValue().c_str()) == 0;
}

bool AutoStart::SetEnabled(bool enable)
{
	HKEY key;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_SET_VALUE, &key) != ERROR_SUCCESS)
		return false;

	LSTATUS result;
	if (enable)
	{
		std::wstring cmd = GetCommandLineValue();
		result = RegSetValueExW(key, VALUE_NAME, 0, REG_SZ,
			reinterpret_cast<const BYTE*>(cmd.c_str()), static_cast<DWORD>((cmd.size() + 1) * sizeof(wchar_t)));
	}
	else
	{
		result = RegDeleteValueW(key, VALUE_NAME);
		if (result == ERROR_FILE_NOT_FOUND)
			result = ERROR_SUCCESS;
	}
	RegCloseKey(key);
	return result == ERROR_SUCCESS;
}
#else
bool AutoStart::IsEnabled()
{
	return false;
}

bool AutoStart::SetEnabled(bool enable)
{
	return false;
}
#endif
