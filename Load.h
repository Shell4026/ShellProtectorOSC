#pragma once
#include <string>

class Load
{
public:
	std::string password;
	int key_idx = 0;
public:
	bool LoadFile();
};

