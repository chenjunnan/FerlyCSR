#pragma once
#include "FL_read_IDCardDNYS.h"
#include "common_helper.h"
#include "robot_json_rsp.h"


class robot_idcardys_module :
	public Thread_helper
{
public:
	robot_idcardys_module();
	~robot_idcardys_module();
	bool message_handler(idcardys_message& rcs);
	int init();

	char version[1024];
	void get_version(char *version);

private:
	virtual void run();
	int get_devstate();
	std::string get_temp_path();
	int timeout;
	int DevState;
	std::string m_port;
	std::string  task_client_guid;
	Read_IDCardDNYS m_idysreader;
	robot_json_rsp m_json;
};


