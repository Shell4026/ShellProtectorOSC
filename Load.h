#pragma once
#include <string>

class Load
{
public:
	std::string password;
	int key_idx = 0;
	int port = 9000;
	bool parameter_multiplexing = false;
	int refresh_rate = 150;
	bool start_and_hide = false;
public:
	bool LoadFile();
};

