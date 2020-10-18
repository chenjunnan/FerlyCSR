#pragma once
#include "common_helper.h"
#include "SerialPort.h"
#include <map>
#include <mutex>
#include <list>

struct BOXINFO
{
	int id = 0;
	std::string wifiid;
	unsigned long long succcesscount=0;
	unsigned long long failedcount=0;
};


class iotServerForLoRaSerial :
	public Thread_helper
{
public:
	iotServerForLoRaSerial(Thread_helper * t=NULL);
	void getid(std::map<std::string, std::string> &list);
	virtual ~iotServerForLoRaSerial();
private:
	virtual void run();
	int Read(std::string &data, int timeout = 2000);
	std::string getIotlListResult();
	void CloseCom();
	int Get_IDList(int index, std::string &type,std::string & boxid, std::string &widifid);
	int write_wait_read(std::string &id, char * buffer, int len, std::string &rersult);
	void errorindex(int index);
	void successindex(int index);
	void successid(std::string id);
	void errorid(std::string id);
	int OpenDoor(std::string widifid, std::list<std::pair<int, int>> & openlist);
	int try_open_wifiid(std::string id, char * send_data);
	bool opencom();
	int Get_DoorState(std::string widifid, std::vector<std::string> &state);
	int Get_PaperState(std::string widifid,std::vector<std::string> &state);
	int Get_Bat(std::string widifid, int *battery);
	void insert_head(char * send_data, std::string &widifid);
	std::map<std::string, std::string> IDList;
	std::map<std::string, BOXINFO > IDListState;
	bool m_isopen;
	CSerialPort m_com;
	Thread_helper * _parent;
	std::mutex		map_lk;
};

