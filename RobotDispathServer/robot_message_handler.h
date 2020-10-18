#pragma once
#include "common_helper.h"
#include "typeenum.h"
enum MESSAGE_ALIN_TYPE
{
	ROBOT_RESULT_MESSAHE,
	ROBOT_RESULT_MESSAHE_ALL
};

class robot_message_handler :
	public Thread_helper
{
public:
	static robot_message_handler *getptr();
	virtual ~robot_message_handler();

private:
	robot_message_handler();
	void add_vectorchar_int(int value, std::vector<char > &out);
	void add_vectorchar_short(int value, std::vector<char > &out);
	void add_head_package_response(ROBOTSOCKETMESSAGE *rsmsg, std::vector<char > &out);
	void message_convert_result(Message &msg);
	void message_convert_result_all(Message &msg);
	int							m_endian_mode;
	virtual void run();
};

