#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

// Finds the UserKey salts of protected avatars in
// - the OSC configs VRChat writes for each avatar it loads (LocalLow/VRChat/VRChat/OSC/usr_*/Avatars/avtr_*.json)
// - LocalLow/ShellProtector/salts.txt, which Unity appends to on every build, so avatars that VRChat
//   has never loaded (for example in Gesture Manager before the first upload) are found too
// It also finds avatars encrypted with ShellProtector 2.7.0 or earlier by their plain parameter names.
// Unchanged files are not read again.
class AvatarConfigScanner
{
	struct Entry
	{
		std::filesystem::file_time_type time;
		std::vector<std::string> salts;
		int legacySwitchCount = 0;
	};
public:
	struct Result
	{
		// Each salt once, most recently written config first
		std::vector<std::string> salts;
		// Number of encrypt_switch* parameters in the most recently written config of a 2.7.0 or earlier avatar,
		// 0 if there is none
		int legacySwitchCount = 0;
	};

	auto Scan() -> Result;

	static auto GetLocalLowDir() -> std::filesystem::path;
	static auto GetOSCConfigDir() -> std::filesystem::path;
	static auto GetSaltListFile() -> std::filesystem::path;
private:
	void ScanFile(const std::filesystem::path& file, std::unordered_map<std::wstring, Entry>& scanned);
	static void Parse(const std::filesystem::path& file, Entry& entry);
private:
	std::unordered_map<std::wstring, Entry> cache;
};
