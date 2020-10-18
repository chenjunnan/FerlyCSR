#include "motion_manager.h"
#include "robot_PostMessage.h"
#include <windows.h>
#include "Define.h"

extern const char *MODULE_NAME;
#define VERSION_MODULE "02.15"
#define MODULE_CLASS 1
#define SUBVERSION 2

motion_manager::motion_manager()
{

}


motion_manager::~motion_manager()
{
	
}


bool motion_manager::Setup()
{
	return _base.start();
}

void motion_manager::Finish()
{
	_base.quit();
}

bool motion_manager::message_handler(headcontrol_message& rcs)
{
	Json::Value   json_object;
	Json::Reader  json_reader;
	RobotActionMessage ram;
	bool b_json_read = json_reader.parse(rcs.content, json_object);
	if (b_json_read)
	{
		std::string gbkstring;
		if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
		{
			std::string msgtype = json_object["msg_type"].asString();
			common_trim(&msgtype);
			if (msgtype.compare("AUTOMATIC_BACKLIGHT_OPEN_REQUEST") == 0)
			{
				if (json_object.isMember("timeout") && json_object["timeout"].isInt())
					rcs.timeout = json_object["timeout"].asInt();
/*				Light.start();*/
			}
			if (msgtype.compare("SCREEN_LUMINANCE_SET_REQUEST") == 0)
			{
				if (json_object.isMember("luminance") && json_object["luminance"].isInt())
					ram.luminance = json_object["luminance"].asInt();
				if (json_object.isMember("timeout") && json_object["timeout"].isInt())
					rcs.timeout = json_object["timeout"].asInt();
				rcs.tasktype = 0;
				rcs.opertype = 1;
			}
			if (msgtype.compare("HEAD_CONTROL_ROTATE_REQUEST") == 0)
			{
				if (json_object.isMember("rotate_type") && json_object["rotate_type"].isInt())
					ram.rotatetype = json_object["rotate_type"].asInt();
				if (json_object.isMember("rotate_angle") && json_object["rotate_angle"].isInt())
					ram.rotateangle = json_object["rotate_angle"].asInt();
				if (json_object.isMember("timeout") && json_object["timeout"].isInt())
					rcs.timeout = json_object["timeout"].asInt();
				rcs.tasktype = 0;
				rcs.opertype = 2;
			}
			if (msgtype.compare("HEAD_CONTROL_SET_ANGLE_REQUEST") == 0)
			{
				if (json_object.isMember("angle_level") && json_object["angle_level"].isInt())
					ram.anglelevel = json_object["angle_level"].asInt();
				if (json_object.isMember("angle_vertical") && json_object["angle_vertical"].isInt())
					ram.anglevertical = json_object["angle_vertical"].asInt();
				if (json_object.isMember("timeout") && json_object["timeout"].isInt())
					rcs.timeout = json_object["timeout"].asInt();
				rcs.tasktype = 0;
				rcs.opertype = 3;
			}
			if (msgtype.compare("HEAD_CONTROL_GET_ANGLE_REQUEST") == 0)
			{
				if (json_object.isMember("timeout") && json_object["timeout"].isInt())
					rcs.timeout = json_object["timeout"].asInt();
				rcs.tasktype = 0;
				rcs.opertype = 4;
			}
			if (msgtype.compare("HEAD_CONTROL_INIT_REQUEST") == 0)
			{
				if (json_object.isMember("timeout") && json_object["timeout"].isInt())
					rcs.timeout = json_object["timeout"].asInt();
				rcs.tasktype = 0;
				rcs.opertype = 5;
			}
			if (msgtype.compare("HEAD_CONTROL_SET_SPEED_REQUEST") == 0)
			{
				if (json_object.isMember("speed") && json_object["speed"].isInt())
					ram.speed = json_object["speed"].asInt();
				if (json_object.isMember("timeout") && json_object["timeout"].isInt())
					rcs.timeout = json_object["timeout"].asInt();
				rcs.tasktype = 0;
				rcs.opertype = 6;
			}
			if (msgtype.compare("HEAD_CONTROL_GET_SPEED_REQUEST") == 0)
			{
				if (json_object.isMember("timeout") && json_object["timeout"].isInt())
					rcs.timeout = json_object["timeout"].asInt();
				rcs.tasktype = 0;
				rcs.opertype = 7;
			}
			if (msgtype.compare("HEAD_LIGHT_SET_REQUEST") == 0)
			{
				if (json_object.isMember("timeout") && json_object["timeout"].isInt())
					rcs.timeout = json_object["timeout"].asInt();
				if (json_object.isMember("R") && json_object["R"].isInt())
					ram.R = json_object["R"].asInt();
				if (json_object.isMember("G") && json_object["G"].isInt())
					ram.G = json_object["G"].asInt();
				if (json_object.isMember("B") && json_object["B"].isInt())
					ram.B = json_object["B"].asInt();
				rcs.tasktype = 0;
				rcs.opertype = 8;
			}
			if (msgtype.compare("HEAD_LIGHT_CLOSE_REQUEST") == 0)
			{
				if (json_object.isMember("timeout") && json_object["timeout"].isInt())
					rcs.timeout = json_object["timeout"].asInt();
				rcs.tasktype = 0;
				rcs.opertype = 9;
			}
			if (msgtype.compare("AUTOMATIC_BACKLIGHT_CLOSE_REQUEST") == 0)
			{
				rcs.tasktype = 1;
			}
			if (msgtype.compare("HEAD_DEVSTATE_INFO_REQUEST") == 0)
			{
				rcs.tasktype = 2;
			}
			//以上是老协议与指令，暂不维护
			if (msgtype.find("MOTION") != -1)
			{
				MotorTask *mt = new MotorTask();
				if (msgtype.compare("MOTION_HEAD_ROTATE_REQUEST") == 0)
					mt->cmd = HEAD_ROTATE;
				if (msgtype.compare("MOTION_ARM_ROTATE_REQUEST") == 0)
					mt->cmd = ARM_ROTATE;
				if (msgtype.compare("MOTION_HEAD_GETANGLE_REQUEST") == 0)
					mt->cmd = HEAD_GETANGLE;
				if (msgtype.compare("MOTION_ARM_GETANGLE_REQUEST") == 0)
					mt->cmd = ARM_GETANGLE;
				if (msgtype.compare("MOTION_ROBOT_DO_ACTION_REQUEST") == 0)
					mt->cmd = ROBOT_DO_ACTION;
				if (msgtype.compare("MOTION_HEAD_STOP_REQUEST") == 0)
					mt->cmd = HEAD_STOP;
				if (msgtype.compare("MOTION_ARM_STOP_REQUEST") == 0)
					mt->cmd = ARM_STOP;
				if (msgtype.compare("MOTION_CHANGESPEED_REQUEST") == 0)
					mt->cmd = CHANGE_SPEED;
				if (msgtype.compare("MOTION_ROBOT_STOP_ACTION_REQUEST") == 0)
					mt->cmd = ROBOT_STOP_ACTION;
				if (msgtype.compare("MOTION_SET_SWINGANGLE_REQUEST") == 0)
					mt->cmd = SET_SWING_ANGLE;
				if (msgtype.compare("MOTION_REINIT_REQUEST") == 0)
					mt->cmd = ROBOT_REINIT;

				

				
				mt->context = rcs.content;
				mt->from = rcs.fromguid;
				_base.pushq(1, "", mt);
			}

		
		}
	}
	return true;
}

void motion_manager::get_version(char *version)
{
	if (version == NULL)return;
	SYSTEMTIME st;
	char szDate[1024];
	ZeroMemory(szDate, sizeof(szDate));
	ZeroMemory(&st, sizeof(SYSTEMTIME));
	sprintf_s(szDate, 1024, "%s", __DATE__);

	st.wYear = atoi(szDate + 7);

	st.wDay = szDate[5] - '0';
	if (szDate[4] != ' ')
	{
		st.wDay += (szDate[4] - '0') * 10;
	}

	switch (szDate[0])
	{
	case 'A':
		if (szDate[1] == 'p')
		{
			st.wMonth = 4;
		}
		else
		{
			st.wMonth = 8;
		}
		break;

	case 'D':
		st.wMonth = 12;
		break;

	case 'F':
		st.wMonth = 2;
		break;

	case 'J':
		if (szDate[1] == 'a')
		{
			st.wMonth = 1;
		}
		else
		{
			if (szDate[2] == 'n')
			{
				st.wMonth = 6;
			}
			else
			{
				st.wMonth = 7;
			}
		}
		break;

	case 'M':
		if (szDate[2] == 'r')
		{
			st.wMonth = 3;
		}
		else
		{
			st.wMonth = 5;
		}
		break;

	case 'N':
		st.wMonth = 11;
		break;

	case 'O':
		st.wMonth = 10;
		break;

	case 'S':
		st.wMonth = 9;
		break;
	}

	ZeroMemory(szDate, sizeof(szDate));
	sprintf_s(szDate, 1024, "%s", __TIME__);

	st.wHour = (szDate[0] - '0') * 10 + (szDate[1] - '0');
	st.wMinute = (szDate[3] - '0') * 10 + (szDate[4] - '0');
	st.wSecond = (szDate[6] - '0') * 10 + (szDate[7] - '0');
	sprintf_s(version, 1024, "%s.%02d.%04d.%d%02d%02d%02d%02d%02d", VERSION_MODULE, MODULE_CLASS, SUBVERSION, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}
