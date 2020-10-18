#pragma once
#include "robot_json_rsp.h"
#include "robot_controller_struct.h"
#include "robot_controller_moduel.h"

class robot_controller_manager
{
public:
	static robot_controller_manager *getInstance();

	bool   init();

	bool   start();

	void   stop();

	void   append_message_type(const char * message);

	int    message_handler(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata = NULL,
		unsigned long dExtlen = 0);


	~robot_controller_manager();
private:

	robot_controller_manager();
	void build_report(FootCtrlMessage &fcmsg, std::string &msgtype);

	void parse_jsonmessage(Json::Value &json_object, FootCtrlMessage &fcmsg);

	void post_report(FootCtrlMessage &vmsg);

	robot_json_rsp      m_json;
	std::vector<std::string >  m_message_list;


	std::map<int, robot_controller_moduel*>  m_controller_handler;
};

