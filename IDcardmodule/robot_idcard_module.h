#pragma once
#include "FL_read_IDCard.h"
#include "common_helper.h"
#include "robot_json_rsp.h"


class robot_idcard_module:
	public Thread_helper
{
public:
	robot_idcard_module();
	virtual ~robot_idcard_module();
	bool message_handler(idcard_message& rcs);
private:
	virtual void run();
	std::string get_temp_path();
	int timeout;
	std::string  task_client_guid;
	Read_IDCD m_idreader;
	robot_json_rsp m_json;
};


