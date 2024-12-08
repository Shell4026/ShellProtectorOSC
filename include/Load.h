#pragma once
#include <string>

class Load
{
public:
	std::string password;
	int keyIdx = 0;
	int port = 9000;
	int refreshRate = 150;
	bool bParameterMultiplexing = false;
	bool bStartAndHide = false;
public:
	bool LoadFile();
};

