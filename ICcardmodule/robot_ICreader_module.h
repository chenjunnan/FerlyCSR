#pragma once 
#include "FL_ReadCard.h"
#include "common_helper.h"
#include "robot_json_rsp.h"

class robot_ICreader_module :
	public Thread_helper
{
public:
	robot_ICreader_module();
	bool message_handler(icreader_message& rcs);
	int init();
	virtual void run();
	virtual ~robot_ICreader_module();
private:
	bool init(char *port, int type, int result, int timeout);
	Read_BankCD m_crt310;
	int card_type;
	int result_type;
	int timeout;
	int task_appid;
	std::string  task_client_guid;
	robot_json_rsp m_json;
	std::string m_port;
};

