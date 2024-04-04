#include "Save.h"
#include <cstdio>

void Save::SaveFile()
{
	FILE* f;
	fopen_s(&f, "save.sav", "w");
	if (f == NULL)
		return;

	fprintf(f, password.c_str());
	fprintf(f, "\n%d", key_idx);
	fclose(f);
}
