#pragma once
#include "common_helper.h"
#include "robot_module_wrap.h"
#include "typeenum.h"

class robot_module_thread :public Thread_helper
{
public:
	robot_module_thread(robot_module_wrap * module);
	virtual ~robot_module_thread();
	virtual void run();
	
private:
	void selfhandler(ROBOTSOCKETMESSAGE * rbmsg, std::string module_name);
	robot_module_wrap *m_module;
};

