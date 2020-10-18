#pragma once

#include <windows.h>
#include "common_helper.h"
#include "robot_json_rsp.h"

class robot_voice_com :public Thread_helper

{
public:
	robot_voice_com();
	bool open(const char * port);
	void close_com();
	bool Beam(int i=360);
	bool Reset();
	virtual ~robot_voice_com();
private:
	bool write_imp(int i);
	bool Reset_imp();
	std::string substr_range(std::string str, int &begin, const char *key, char endspitc);
	virtual void run();
	bool readB(unsigned char* tranChar);
	HANDLE m_hCom;
	robot_json_rsp m_json;
	std::string port_str;
	int default_angle;
	bool autowakeup;
};

