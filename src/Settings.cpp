#include "PCH.h"
#include "Settings.h"
#include "Path.h"

#include <cstdio>

static auto GetSaveFilePath() -> std::filesystem::path
{
	return Path::GetExeDir() / L"save.sav";
}

bool Settings::Load()
{
	FILE* f;
	_wfopen_s(&f, GetSaveFilePath().c_str(), L"r");
	if (f == NULL)
		return false;

	char str[100];
	if (!feof(f))
	{
		fgets(str, sizeof(str), f);
		password = str;
		password.pop_back(); //remove \n
	}
	if (!feof(f))
	{
		fgets(str, sizeof(str), f);
		keyIdx = std::stoi(str);
	}
	if (!feof(f)) {
		fgets(str, sizeof(str), f);
		port = std::stoi(str);
	}
	if (!feof(f)) {
		fgets(str, sizeof(str), f);
		bParameterMultiplexing = std::stoi(str);
	}
	if (!feof(f)) {
		fgets(str, sizeof(str), f);
		refreshRate = std::stoi(str);
	}
	if (!feof(f)) {
		fgets(str, sizeof(str), f);
		bStartAndHide = std::stoi(str);
	}
	if (!feof(f) && fgets(str, sizeof(str), f)) {
		ip = str;
		while (!ip.empty() && (ip.back() == '\n' || ip.back() == '\r'))
			ip.pop_back();
		if (ip.empty())
			ip = "127.0.0.1";
	}
	fclose(f);
	return true;
}

void Settings::Save() const
{
	FILE* f;
	_wfopen_s(&f, GetSaveFilePath().c_str(), L"w");
	if (f == NULL)
		return;

	fprintf(f, "%s", password.c_str());
	fprintf(f, "\n%d", keyIdx);
	fprintf(f, "\n%d", port);
	fprintf(f, "\n%d", bParameterMultiplexing);
	fprintf(f, "\n%d", refreshRate);
	fprintf(f, "\n%d", bStartAndHide);
	fprintf(f, "\n%s", ip.c_str());
	fclose(f);
}
