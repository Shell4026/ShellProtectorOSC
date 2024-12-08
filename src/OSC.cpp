#include "PCH.h"
#include "OSC.h"
#include <iostream>
auto OSC::GetOSCData(const std::string& oscAddr, float v) -> std::vector<unsigned char>
{
	std::vector<unsigned char> data;
	std::size_t size = oscAddr.size() + 4 - oscAddr.size() % 4 + 4;
	data.reserve(size);
	for (int i = 0; i < oscAddr.size(); ++i)
		data.push_back(oscAddr[i]);
	for(int i = 0; i < 4 - oscAddr.size() % 4; ++i)
		data.push_back(0);
	data.push_back(',');
	data.push_back('f');
	data.push_back(0);
	data.push_back(0);

	unsigned char* bytes = reinterpret_cast<unsigned char*>(&v);
	for (int i = sizeof(float) - 1; i >= 0; --i)
		data.push_back(bytes[i]);

	return data;
}

auto OSC::GetOSCData(const std::string& oscAddr, bool v) -> std::vector<unsigned char>
{
	std::vector<unsigned char> data;
	std::size_t size = oscAddr.size() + 4 - oscAddr.size() % 4 + 4;
	data.reserve(size);
	for (int i = 0; i < oscAddr.size(); ++i)
		data.push_back(oscAddr[i]);
	for (int i = 0; i < 4 - oscAddr.size() % 4; ++i)
		data.push_back(0);
	data.push_back(',');
	if(v)
		data.push_back('T');
	else
		data.push_back('F');
	data.push_back(0);
	data.push_back(0);

	return data;
}

auto OSC::GetOSCData(const std::string& oscAddr, int v) -> std::vector<unsigned char>
{
	std::vector<unsigned char> data;
	std::size_t size = oscAddr.size() + 4 - oscAddr.size() % 4 + 4;
	data.reserve(size);
	for (int i = 0; i < oscAddr.size(); ++i)
		data.push_back(oscAddr[i]);
	for (int i = 0; i < 4 - oscAddr.size() % 4; ++i)
		data.push_back(0);
	data.push_back(',');
	data.push_back('i');
	data.push_back(0);
	data.push_back(0);

	unsigned char* bytes = reinterpret_cast<unsigned char*>(&v);
	for (int i = sizeof(float) - 1; i >= 0; --i)
		data.push_back(bytes[i]);

	return data;
}

OSC::OSC() :
	port(0)
{
}

void OSC::Init(const std::string& address, unsigned int port)
{
	this->addr = address;
	this->port = port;

	if (socket.bind(sf::Socket::AnyPort) != sf::Socket::Done)
	{
		AddLog("Can't bind port in " + std::to_string(port));
	}
	AddLog("Init socket port:" + std::to_string(socket.getLocalPort()));
}

void OSC::Init(std::string&& address, unsigned int port)
{
	this->addr = std::move(address);
	this->port = port;
	if (socket.bind(sf::Socket::AnyPort) != sf::Socket::Done)
	{
		AddLog("Can't bind port in " + std::to_string(port));
	}
	AddLog("Init socket port:" + std::to_string(socket.getLocalPort()));
}

void OSC::SetOSCPort(int port)
{
	this->port = port;
}


bool OSC::SendOSC(std::string oscAddr, float v)
{
	auto data = GetOSCData(oscAddr, v);
	if (socket.send(&data[0], data.size(), addr, port) != sf::Socket::Done)
	{
		AddLog("Socket send failed.");
		return false;
	}
	AddLog("Socket send : " + oscAddr + ", value:" + std::to_string(v));
	return true;
}

bool OSC::SendOSC(std::string oscAddr, bool v)
{
	auto data = GetOSCData(oscAddr, v);
	if (socket.send(&data[0], data.size(), addr, port) != sf::Socket::Done)
	{
		AddLog("Socket send failed.");
		return false;
	}
	AddLog("Socket send : " + oscAddr + ", value:" + std::to_string(v));
	return true;
}

bool OSC::SendOSC(std::string oscAddr, int v)
{
	auto data = GetOSCData(oscAddr, v);
	if (socket.send(&data[0], data.size(), addr, port) != sf::Socket::Done)
	{
		AddLog("Socket send failed.");
		return false;
	}
	AddLog("Socket send : " + oscAddr + ", value:" + std::to_string(v));
	return true;
}

auto OSC::GetLogs() -> const std::deque<std::string>&
{
	return logs;
}

void OSC::AddLog(const std::string& str)
{
	if (log_lock == true)
		return;

	while (logs.size() > max_log)
		logs.pop_front();
	logs.push_back(str);
}

void OSC::ClearLogs()
{
	logs.clear();
}
