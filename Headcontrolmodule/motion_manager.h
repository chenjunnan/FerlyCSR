#pragma once
#include "common_helper.h"
#include "robot_json_rsp.h"
#include "robot_baseaction_module.h"


class motion_manager 
{
public:
	motion_manager();
	~motion_manager();
	bool Setup();
	void Finish();
	bool message_handler(headcontrol_message& rcs);
	void get_version(char *version);
private:

	int get_devstate();
	robot_baseaction_module _base;
};


