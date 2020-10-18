#pragma once
#include "RobotModule_interface.h"

class robot_PostMessage
{
public:
	static robot_PostMessage *getPostManager();
	void set(fPostDataCB fun);
	void post(const char *strModuleName, const char *strResult, unsigned long dResultLen,
		const char *desModuleName = NULL, const unsigned char *strExtdata = NULL, unsigned long  dExtlen=0);
	~robot_PostMessage();
private:
	robot_PostMessage();
	fPostDataCB   m_post_message ;
	
};

