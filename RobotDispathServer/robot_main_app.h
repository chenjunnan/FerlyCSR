#pragma once
#include <stddef.h>

#include "ServerInterface.h"
#include "xCommon.h"
#include "common_helper.h"
#include "robot_message_dispatch.h"
#include "common_helper.h"
#include "robot_module_manager.h"
#include "robot_message_handler.h"


#include <Windows.h>
#include <DbgHelp.h>
#include <signal.h>
class robot_input_thread :public Thread_helper
{
public:
	robot_input_thread(){}
	virtual ~robot_input_thread(){}
	virtual void run();
	char order_c = 0;
};


class robot_main_app
{
public:
	static robot_main_app* getinstance();
	void run();
	bool init();
	void main_close();
	void close_service();
	~robot_main_app();
private:
	void wait();
	robot_main_app();
	void get_statusfilepath(std::string &exepath);
	static robot_main_app* m_instance;
	bool close_flag;
	bool m_ok;
	robot_input_thread m_input;
};

