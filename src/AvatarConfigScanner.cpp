#include "PCH.h"
#include "AvatarConfigScanner.h"
#include "UserKey.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <utility>
#if _WIN32
#include <shlobj.h>
#endif

namespace fs = std::filesystem;

auto AvatarConfigScanner::Scan() -> std::vector<std::string>
{
	std::error_code ec;
	std::unordered_map<std::wstring, Entry> scanned;

	const fs::path oscDir = GetOSCConfigDir();
	if (!oscDir.empty() && fs::is_directory(oscDir, ec))
	{
		for (const auto& user : fs::directory_iterator(oscDir, ec))
		{
			const fs::path avatarsDir = user.path() / "Avatars";
			if (!fs::is_directory(avatarsDir, ec))
				continue;

			for (const auto& file : fs::directory_iterator(avatarsDir, ec))
			{
				if (file.path().extension() == ".json")
					ScanFile(file.path(), scanned);
			}
		}
	}

	const fs::path saltList = GetSaltListFile();
	if (!saltList.empty() && fs::is_regular_file(saltList, ec))
		ScanFile(saltList, scanned);

	cache = std::move(scanned); // Drops deleted files

	std::vector<std::pair<fs::file_time_type, std::string>> found;
	for (const auto& [path, entry] : cache)
	{
		for (const auto& salt : entry.salts)
			found.emplace_back(entry.time, salt);
	}
	std::sort(found.begin(), found.end(), [](const auto& a, const auto& b) { return a.first > b.first; });

	std::vector<std::string> salts;
	for (auto& [time, salt] : found)
	{
		if (std::find(salts.begin(), salts.end(), salt) == salts.end())
			salts.push_back(std::move(salt));
	}
	return salts;
}

void AvatarConfigScanner::ScanFile(const fs::path& file, std::unordered_map<std::wstring, Entry>& scanned)
{
	std::error_code ec;
	auto time = fs::last_write_time(file, ec);
	if (ec)
		return;

	const std::wstring key = file.wstring();
	auto cached = cache.find(key);
	if (cached != cache.end() && cached->second.time == time)
		scanned.emplace(key, std::move(cached->second));
	else
		scanned.emplace(key, Entry{ time, FindSalts(file) });
}

auto AvatarConfigScanner::GetLocalLowDir() -> fs::path
{
#if _WIN32
	PWSTR localLow = nullptr;
	fs::path result;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppDataLow, 0, nullptr, &localLow)))
		result = localLow;
	CoTaskMemFree(localLow);
	return result;
#else
	return {};
#endif
}

auto AvatarConfigScanner::GetOSCConfigDir() -> fs::path
{
	const fs::path localLow = GetLocalLowDir();
	return localLow.empty() ? fs::path() : localLow / "VRChat" / "VRChat" / "OSC";
}

auto AvatarConfigScanner::GetSaltListFile() -> fs::path
{
	const fs::path localLow = GetLocalLowDir();
	return localLow.empty() ? fs::path() : localLow / "ShellProtector" / "salts.txt";
}

auto AvatarConfigScanner::FindSalts(const fs::path& file) -> std::vector<std::string>
{
	std::ifstream stream(file, std::ios::binary);
	if (!stream)
		return {};
	std::stringstream buffer;
	buffer << stream.rdbuf();
	const std::string text = buffer.str();

	const std::string prefix = UserKey::SALT_PARAMETER_PREFIX;
	std::vector<std::string> salts;
	for (std::size_t pos = text.find(prefix); pos != std::string::npos; pos = text.find(prefix, pos + 1))
	{
		std::string salt = text.substr(pos + prefix.size(), UserKey::SALT_HEX_LENGTH);
		if (UserKey::IsValidSalt(salt) && std::find(salts.begin(), salts.end(), salt) == salts.end())
			salts.push_back(std::move(salt));
	}
	return salts;
}
