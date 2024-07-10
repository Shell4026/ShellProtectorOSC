#pragma once
#include <string>
class Save
{
public:
	std::string password;
	int key_idx = 0;
	int port;
	bool parameter_multiplexing;
	int refresh_rate;
public:
	void SaveFile();
};

