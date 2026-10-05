#include "PCH.h"
#include "Settings.h"

#include <cstdio>

bool Settings::Load()
{
	FILE* f;
	fopen_s(&f, "save.sav", "r");
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
	fclose(f);
	return true;
}

void Settings::Save() const
{
	FILE* f;
	fopen_s(&f, "save.sav", "w");
	if (f == NULL)
		return;

	fprintf(f, "%s", password.c_str());
	fprintf(f, "\n%d", keyIdx);
	fprintf(f, "\n%d", port);
	fprintf(f, "\n%d", bParameterMultiplexing);
	fprintf(f, "\n%d", refreshRate);
	fprintf(f, "\n%d", bStartAndHide);
	fclose(f);
}
