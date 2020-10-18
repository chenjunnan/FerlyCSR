#pragma once
#include "common_helper.h"
#include <list>
#include <map>
#include "json\json.h"
#include "iotStatistics.h"
#include "iot.h"

class socket_context
{
public:
	socket_context(){  }
	~socket_context(){
	}
	std::string  ip;
	int port = 0;
	bool   online = true;
	bool   ready = false;
	CHANDLESOCKET p_hsocket;
	std::string  m_cache;
	std::list<MessageRequest>  wait_list;
	std::list<MessageRequest>  report_list;
	std::list<MessageRequest>  readysend_list;
	std::list<MessageRequest>  accpet_list;
	std::string id;
	std::string lastdata;
	int type = 0;
	unsigned long long active_last_time=0;
};




class iotServerForWifi50315 :
	public SubmoduleInterface,
	public xSocketCallBackInterFace
{
public:
	iotServerForWifi50315();
	virtual ~iotServerForWifi50315();


	virtual void run();
	
	virtual void AppendPortList(Json::Value &json_object_base);

	virtual void onConnect(xSocketUserInterface* handle);
	virtual void onAccept(xSocketUserInterface* handle);
	virtual void onRead(xSocketUserInterface* handle, VEC_BYTE data);

	virtual void onWrite(xSocketUserInterface* handle, int error);
	virtual void onClose(xSocketUserInterface* handle);

	virtual void onEvent(int error);

private:
	void makeresponse(xSocketUserInterface* handle, std::string result);
	void shifttask(xSocketUserInterface* oldh, xSocketUserInterface*newh);
	void report(xSocketUserInterface* handle);
	int  timeout();

	void HeartBeat(xSocketUserInterface *it);

	int command(xSocketUserInterface* h, std::string cmd, int ts);


	void clean_overdue_task(std::string &id, std::list<MessageRequest>& _list,const char *merr="");
	void clean_overdue_donetask(std::string &id, std::list<MessageRequest>& _list);

	void msg_process(msg_context * msgc, int messagetype);

	xSocketUserInterface *  getHandler(int messagetype, msg_context * msgc);

	std::string getIotlist();

	void RemoveDevice(xSocketUserInterface* handle);

	std::string getCommandHeadstr(int type);

	void Post_Command(int type, xSocketUserInterface * handler, msg_context * msgc, unsigned long long times);

	std::string packageCommand(int type, msg_context * msgc);

	void process_STMresponse(std::string &order, xSocketUserInterface* handle);

	void PostRemainMessage(xSocketUserInterface* handle);

	void PostOneTimeoutMessage(MessageRequest&msr, std::string id);
	xSocketUserInterface * find_handle(std::string id, int type);
	CHANDLE							m_ml;
	std::map < xSocketUserInterface  *, socket_context > m_socket_map;

	iotStatistics   m_file_state;
};

