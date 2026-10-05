#include "PCH.h"
#include "Path.h"

auto Path::GetExePath() -> std::filesystem::path
{
#if _WIN32
	std::wstring buffer(MAX_PATH, L'\0');
	while (true)
	{
		DWORD len = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
		if (len == 0)
			return {};
		if (len < buffer.size())
		{
			buffer.resize(len);
			return buffer;
		}
		buffer.resize(buffer.size() * 2); // Path was truncated
	}
#else
	std::error_code ec;
	return std::filesystem::read_symlink("/proc/self/exe", ec);
#endif
}

auto Path::GetExeDir() -> std::filesystem::path
{
	auto exePath = GetExePath();
	if (exePath.empty())
		return std::filesystem::current_path();
	return exePath.parent_path();
}
