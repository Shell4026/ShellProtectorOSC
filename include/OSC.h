#pragma once
#include <string>
#include <vector>
#include <deque>

#include "SFML/Network.hpp"

class OSC
{
private:
	sf::UdpSocket socket;

	std::string addr;
	std::deque<std::string> logs;
	unsigned int port;
public:
	int max_log = 100;
	bool log_lock = false;
public:
	OSC();
	auto GetOSCData(const std::string& oscAddr, float v)->std::vector<unsigned char>;
	auto GetOSCData(const std::string& oscAddr, bool v)->std::vector<unsigned char>;
	auto GetOSCData(const std::string& oscAddr, int v)->std::vector<unsigned char>;

	void Init(const std::string& address, unsigned int port);
	void Init(std::string&& address, unsigned int port);

	void SetOSCPort(int port);

	bool SendOSC(std::string oscAddr, float v);
	bool SendOSC(std::string oscAddr, bool v);
	bool SendOSC(std::string oscAddr, int v);

	auto GetLogs() -> const std::deque<std::string>&;
	void AddLog(const std::string& str);
	void ClearLogs();
};

