#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

// Finds the UserKey salts of protected avatars in
// - the OSC configs VRChat writes for each avatar it loads (LocalLow/VRChat/VRChat/OSC/usr_*/Avatars/avtr_*.json)
// - LocalLow/ShellProtector/salts.txt, which Unity appends to on every build, so avatars that VRChat
//   has never loaded (for example in Gesture Manager before the first upload) are found too
// Unchanged files are not read again.
class AvatarConfigScanner
{
	struct Entry
	{
		std::filesystem::file_time_type time;
		std::vector<std::string> salts;
	};
public:
	// Returns each salt once, most recently written config first
	auto Scan() -> std::vector<std::string>;

	static auto GetLocalLowDir() -> std::filesystem::path;
	static auto GetOSCConfigDir() -> std::filesystem::path;
	static auto GetSaltListFile() -> std::filesystem::path;
private:
	void ScanFile(const std::filesystem::path& file, std::unordered_map<std::wstring, Entry>& scanned);
	static auto FindSalts(const std::filesystem::path& file) -> std::vector<std::string>;
private:
	std::unordered_map<std::wstring, Entry> cache;
};
