#include "PCH.h"
#include "Save.h"
#include <cstdio>

void Save::SaveFile()
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
