#pragma once
#include "common_helper.h"
class robot_voice_release :
	public Thread_helper
{
public:
	static robot_voice_release * getptr();
	virtual void run();
	virtual ~robot_voice_release();
private:
	robot_voice_release();
};

