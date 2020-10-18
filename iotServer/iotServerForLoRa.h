#pragma once
#include "common_helper.h"
#include "iotServerForLoRaSerial.h"
#include <list>
#include "iot.h"

enum RUNSTATE
{
	high,
	mid,
	low
};

class iotServerForLoRa :public SubmoduleInterface
{
public:
	iotServerForLoRa();
	bool init();
	virtual ~iotServerForLoRa();
	virtual void AppendPortList(Json::Value &json_object_base);
private:
	virtual void run();
	iotServerForLoRaSerial _exe;
	std::string getIotlist(std::map<std::string, std::string > &IDList);
	RUNSTATE _state;
	std::list<Message> highlist;
	std::list<Message> midlist;
	int		_current_task;
};

