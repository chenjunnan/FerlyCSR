// footmodule.cpp : 定义 DLL 应用程序的导出函数。
//

#include <windows.h>
#include "memorySwaper.h"
#include "RobotModule_interface.h"
#include "xCommon.h"
#include "robot_controller_manager.h"
#include "robot_PostMessage.h"

extern const char *MODULE_NAME = "foot_module";

static const char *HANDLE_LIST[] = {
"NAVIGATE_TO_POSTION_REQUEST",
"NAVIGATE_TO_CANCEL_REQUEST",
"NAVIGATE_GET_CURRENT_POSTION_REQUEST",
"NAVIGATE_INITIALIZE_REQUEST",
"NAVIGATE_MOVE_REQUEST",//5
"NAVIGATE_ROTATE_REQUEST",
"NAVIGATE_DEVICE_STATUS_REQUEST",
"NAVIGATE_START_MOVEDATA_REQUEST",
"NAVIGATE_STOP_MOVEDATA_REQUEST",
"NAVIGATE_GET_MAP_REQUEST",//10
"NAVIGATE_MAP_LIST_REQUEST",
"NAVIGATE_APPEND_CHARGEPOINT_REQUEST",
"NAVIGATE_TO_CHARGE_REQUEST",
"NAVIGATE_GET_CHARGE_LIST_REQUEST",
"NAVIGATE_DEL_CHARGEPOINT_REQUEST",//15
"NAVIGATE_START_CHARGE_REQUEST",
"NAVIGATE_STOP_CHARGE_REQUEST",
"NAVIGATE_GET_STATE_CHARGE_REQUEST",
"NAVIGATE_SHUTDOWN_REQUEST",
"NAVIGATE_CHANGE_SPEED_REQUEST",//20
"NAVIGATE_GET_PATH_REQUEST",
"NAVIGATE_START_DANCE",
"NAVIGATE_STOP_DANCE",
"NAVIGATE_GREETER_REQUEST",
"NAVIGATE_HEALTH_REQUEST",//25
"NAVIGATE_RESUME_MOTOR_REQUEST",
"NAVIGATE_MOVETO_REQUEST",
"NAVIGATE_STOPMOVETO_REQUEST",
"NAVIGATE_STOPROTATE_REQUEST",
"NAVIGATE_MOTOR_REQUEST",
"NAVIGATE_GET_SPEED_REQUEST",//30
"NAVIGATE_START_SPEED_REQUEST",
"NAVIGATE_STOP_SPEED_REQUEST",
"NAVIGATE_SCAN_MAP_REQUEST",
"NAVIGATE_SCAN_SAVEMAP_REQUEST",
"NAVIGATE_SCAN_CANCEL_REQUEST",//35
"NAVIGATE_SCAN_GET_MAP_REQUEST",
"NAVIGATE_DEL_MAP_REQUEST",
"NAVIGATE_GET_VIRTUAL_WALL_REQUEST",
"NAVIGATE_SET_VIRTUAL_WALL_REQUEST",
"NAVIGATE_GET_PROTECT_REQUEST",//40
"NAVIGATE_APPEND_SPECIALPOINT_REQUEST",
"NAVIGATE_TO_SPECIALPOINT_REQUEST",
"NAVIGATE_GET_SPECIALPOINT_LIST_REQUEST",
"NAVIGATE_DEL_SPECIALPOINT_REQUEST",
"NAVIGATE_START_MASE_REQUEST",//45
"NAVIGATE_STOP_MASE_REQUEST",
"NAVIGATE_GET_GRAPHPATH_LIST_REQUEST",
"NAVIGATE_GET_TASK_STATUS_REQUEST",
"NAVIGATE_APPEND_TASKQUEUE_REQUEST",
"NAVIGATE_GET_TASKQUEUE_REQUEST",//50
"NAVIGATE_DEL_TASKQUEUE_REQUEST",
"NAVIGATE_START_TASKQUEUE_REQUEST",
"NAVIGATE_STOP_ALLTASKQUEUE_REQUEST",
"NAVIGATE_PAUSE_TASKQUEUE_REQUEST",
"NAVIGATE_RESUME_TASKQUEUE_REQUEST",//55
"NAVIGATE_STOP_CURRENTTASK_REQUEST",
"NAVIGATE_IS_TASKQUEUE_FINISH_REQUEST",
"NAVIGATE_MOVE_CHECK_RANGE_REQUEST",
"NAVIGATE_UPDATE_MOVE_WALL_REQUEST",
"NAVIGATE_GET_LASER_RAW_REQUEST",//60
"NAVIGATE_CANCEL_INITIALIZE_REQUEST",
"NAVIGATE_GET_TASK_LIST_REQUEST",
"NAVIGATE_RESET_ROBOT_REQUEST",
NULL };

static fPostDataCB   post_message = NULL;
#define VERSION_MODULE	"02.05"//大版本.模块号
#define MODULE_CLASS    1     //模块子版本类型
#define SUBVERSION      15   //模块小版本号.编译时间

char  version[1024] = { 0 };

void  get_version(char *version);
ROBOTMODULE_API bool  Init()

{
	for (int i = 0; i < sizeof(HANDLE_LIST) / sizeof(char*); i++)
	{
		if (HANDLE_LIST[i] == NULL)
			break;
		robot_controller_manager::getInstance()->append_message_type(HANDLE_LIST[i]);
	}
	common_log_create("mase");//TODO for 磁轨
	return robot_controller_manager::getInstance()->init();
}


ROBOTMODULE_API  bool ROBOTMODULE_API_M Start()
{
	return robot_controller_manager::getInstance()->start();
}

ROBOTMODULE_API bool ROBOTMODULE_API_M Stop()
{
	 robot_controller_manager::getInstance()->stop();
	 return true;
}

ROBOTMODULE_API const char* ROBOTMODULE_API_M GetModuleName()
{
	return MODULE_NAME;
}

ROBOTMODULE_API void ROBOTMODULE_API_M RegisterHandleMessageType(char *strHandleList)
{
	char *dst = strHandleList;

	for (int i = 0; i < sizeof(HANDLE_LIST) / sizeof(char*); i++)
	{
		if (HANDLE_LIST[i] == NULL)
		{
			*(strHandleList) = NULL;
			break;
		}

		strcpy(strHandleList, HANDLE_LIST[i]);
		strHandleList += strlen(HANDLE_LIST[i]);
		*(strHandleList) = NULL;
		strHandleList += 1;
	}
}



ROBOTMODULE_API int ROBOTMODULE_API_M SetProcessData(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata /*= NULL*/,
	unsigned long dExtlen /*= 0*/)
{
	return robot_controller_manager::getInstance()->message_handler(dAppid, dMessageType, strFrom, strMessage, dMessagelen, strExtdata, dExtlen);
}

ROBOTMODULE_API void ROBOTMODULE_API_M SetPostFuntion(fPostDataCB ptr)
{
	if (ptr != NULL)
	{
		robot_PostMessage::getPostManager()->set(ptr);
		post_message = ptr;
	}
	
}

ROBOTMODULE_API const char* ROBOTMODULE_API_M GetModuleVersion()
{
	if (strlen(version) == 0)
		get_version(version);
	return version;
}

void  get_version(char *version)
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