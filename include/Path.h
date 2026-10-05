#pragma once
#include <filesystem>

namespace Path
{
	auto GetExePath() -> std::filesystem::path;
	auto GetExeDir() -> std::filesystem::path;
}
