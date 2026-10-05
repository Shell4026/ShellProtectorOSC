#pragma once
#include <string>

// Persistent settings stored in save.sav next to the exe
class Settings
{
public:
	bool Load();
	void Save() const;
public:
	std::string password;
	int keyIdx = 0;
	int port = 9000;
	int refreshRate = 150;
	bool bParameterMultiplexing = false;
	bool bStartAndHide = false;
};
