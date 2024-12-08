#pragma once
#include <string>
class Save
{
public:
	std::string password;
	int keyIdx = 0;
	int port;
	int refreshRate;

	bool bParameterMultiplexing;
	bool bStartAndHide;
public:
	void SaveFile();
};

