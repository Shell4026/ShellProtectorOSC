#include "PCH.h"
#include "AvatarConfigScanner.h"
#include "UserKey.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <utility>
#if _WIN32
#include <shlobj.h>
#endif

namespace fs = std::filesystem;

auto AvatarConfigScanner::Scan() -> Result
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

	Result result;
	std::vector<std::pair<fs::file_time_type, std::string>> found;
	const Entry* legacy = nullptr;
	for (const auto& [path, entry] : cache)
	{
		for (const auto& salt : entry.salts)
			found.emplace_back(entry.time, salt);
		if (entry.legacySwitchCount > 0 && (legacy == nullptr || entry.time > legacy->time))
			legacy = &entry;
	}
	std::sort(found.begin(), found.end(), [](const auto& a, const auto& b) { return a.first > b.first; });

	for (auto& [time, salt] : found)
	{
		if (std::find(result.salts.begin(), result.salts.end(), salt) == result.salts.end())
			result.salts.push_back(std::move(salt));
	}
	if (legacy != nullptr)
		result.legacySwitchCount = legacy->legacySwitchCount;
	return result;
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
	{
		Entry entry{ time };
		Parse(file, entry);
		scanned.emplace(key, std::move(entry));
	}
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

void AvatarConfigScanner::Parse(const fs::path& file, Entry& entry)
{
	std::ifstream stream(file, std::ios::binary);
	if (!stream)
		return;
	std::stringstream buffer;
	buffer << stream.rdbuf();
	const std::string text = buffer.str();

	const std::string prefix = UserKey::SALT_PARAMETER_PREFIX;
	for (std::size_t pos = text.find(prefix); pos != std::string::npos; pos = text.find(prefix, pos + 1))
	{
		std::string salt = text.substr(pos + prefix.size(), UserKey::SALT_HEX_LENGTH);
		if (UserKey::IsValidSalt(salt) && std::find(entry.salts.begin(), entry.salts.end(), salt) == entry.salts.end())
			entry.salts.push_back(std::move(salt));
	}

	// The name appears as "name": "encrypt_switch0" and in the address /avatar/parameters/encrypt_switch0.
	// Newer avatars obfuscate their parameter names, so only 2.7.0 and earlier have it.
	const std::string switchName = "encrypt_switch";
	for (std::size_t pos = text.find(switchName); pos != std::string::npos; pos = text.find(switchName, pos + 1))
	{
		if (pos == 0 || (text[pos - 1] != '"' && text[pos - 1] != '/'))
			continue;
		std::size_t end = pos + switchName.size();
		int index = 0;
		bool hasDigit = false;
		while (end < text.size() && std::isdigit(static_cast<unsigned char>(text[end])) && index < 100)
		{
			index = index * 10 + (text[end++] - '0');
			hasDigit = true;
		}
		if (hasDigit && (end == text.size() || text[end] == '"') && index + 1 > entry.legacySwitchCount)
			entry.legacySwitchCount = index + 1;
	}
}
