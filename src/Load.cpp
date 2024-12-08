#include "PCH.h"
#include "Load.h"

bool Load::LoadFile()
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
