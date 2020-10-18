#pragma once
#include <map>
#include "typeenum.h"
#include "common_helper.h"


struct recv_cache_data
{
	unsigned int totle_bytes;
	std::vector<char> buffer;
	recv_cache_data()
	{
		totle_bytes = 0;
	}
};



class robot_message_dispatch:public Thread_helper
{
public:
	static robot_message_dispatch *getptr();
	virtual ~robot_message_dispatch();
	void remove(std::string clientguid);
private:
	int check_and_get_package_info(std::string guid, ROBOTSOCKETMESSAGE &rsm);
	int need_route_message(ROBOTSOCKETMESSAGE &rsm);
	int route_message(ROBOTSOCKETMESSAGE &rsm);
	void add_vectorchar_int(int value, std::vector<char > &out);
	void add_vectorchar_short(int value, std::vector<char > &out);
	void add_head_package_response(ROBOTSOCKETMESSAGE *rsmsg, std::vector<char > &out);
	void in_message_convert_result(Message &msg);
	int net_to_host_int(std::vector< char> &buffer, int b_pos);
	short net_to_host_short(std::vector< char> &buffer, int b_pos);
	virtual void run();
	void net_data_in_handler(Message &msg);
	void local_msg_handler(ROBOTSOCKETMESSAGE &rsm);
	void set_voice_service(int type);
	robot_message_dispatch();
	int										m_server_NodeID;
	std::map<std::string, recv_cache_data>  m_cache_netdata;
	int										m_endian_mode;
};

