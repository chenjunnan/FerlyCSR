#pragma once
#include <map>
#include "common_helper.h"
#include "iot.h"


class iotServerManager
{
public:
	iotServerManager();
	bool  Init();
	bool  Start();
	bool  Stop();
	int   SetProcessData(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata /*= NULL*/,
		unsigned long dExtlen /*= 0*/);

	void GetAllList(const char * strFrom);


	virtual ~iotServerManager();
private:
	int getMessgaeType(std::string &msgtype);
	std::map<std::string, SubmoduleInterface*> m_module_list;
};

