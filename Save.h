#pragma once
#include <string>
class Save
{
public:
	std::string password;
	int key_idx = 0;
public:
	void SaveFile();
};

