#pragma once
#include "common_helper.h"
class internet_notify
{
public:
	virtual void netnotify(int state)=0;
	virtual ~internet_notify(){}
};

class robot_internetcheck :
	public Thread_helper
{
public:
	robot_internetcheck(internet_notify *ine);
	void setTimeout(int timeout);
	virtual ~robot_internetcheck();
private:
	std::string imp_get(std::string url, int t);
	virtual void run();
	unsigned int m_timeout;
	CHANDLE m_http;
	internet_notify  *m_cb;
};

