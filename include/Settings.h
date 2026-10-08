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
	int port = 9000;
	int refreshRate = 150;
	bool bParameterMultiplexing = true;
	bool bStartAndHide = false;
	std::string ip = "127.0.0.1";
};
