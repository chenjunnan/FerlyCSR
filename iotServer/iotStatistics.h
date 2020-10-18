#pragma once
#include <string>
#include <map>
#include <vector>
#include "common_helper.h"
#include <mutex>

struct Netstatus
{
	std::string id;
	bool changenet = false;
	bool online = false;
	unsigned long long onlinetime=0;
	unsigned long long offlinetime=0;
	unsigned long long hasdatatime = 0;
	unsigned long long totelonline=0;
	unsigned long long toteloffline = 0;
	std::vector<int> offlinesum;
};

class iotStatistics:
	public Thread_helper
{
public:
	iotStatistics();
	~iotStatistics();
	void SetID(std::string ip, std::string id);
	void changeNet(std::string ip);
	void hasdata(std::string ip);
	void Boxoffline(std::string ip);
private:
	void Boxonline(std::string ip, std::string id);
	void _online(std::string ip, std::string id);
	virtual void run();
	std::map<std::string, Netstatus> m_state;
	unsigned long long writetime;
	std::mutex m_mapl;
};

