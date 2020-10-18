// IDcardmodule.cpp : 定义 DLL 应用程序的导出函数。
//


#include "RobotModule_interface.h"
#include "xCommon.h"
#include "robot_PostMessage.h"
#include "motion_manager.h"

const char *MODULE_NAME = "headcontrol_module";

static const char *HANDLE_LIST[] = {
	"AUTOMATIC_BACKLIGHT_OPEN_REQUEST",
	"AUTOMATIC_BACKLIGHT_CLOSE_REQUEST",
	"SCREEN_LUMINANCE_SET_REQUEST",
	"HEAD_LIGHT_SET_REQUEST",
	"HEAD_LIGHT_CLOSE_REQUEST",
	"HEAD_CONTROL_ROTATE_REQUEST",
	"HEAD_CONTROL_SET_ANGLE_REQUEST",
	"HEAD_CONTROL_GET_ANGLE_REQUEST",
	"HEAD_CONTROL_INIT_REQUEST",
	"HEAD_CONTROL_SET_SPEED_REQUEST",
	"HEAD_CONTROL_GET_SPEED_REQUEST",
	"HEAD_DEVSTATE_INFO_REQUEST",

	"MOTION_HEAD_ROTATE_REQUEST",
	"MOTION_ARM_ROTATE_REQUEST",
	"MOTION_HEAD_GETANGLE_REQUEST",
	"MOTION_ARM_GETANGLE_REQUEST",
	"MOTION_ROBOT_DO_ACTION_REQUEST",
	"MOTION_HEAD_STOP_REQUEST",
	"MOTION_ARM_STOP_REQUEST",
	"MOTION_CHANGESPEED_REQUEST",
	"MOTION_ROBOT_STOP_ACTION_REQUEST",
	"MOTION_SET_SWINGANGLE_REQUEST",
	"MOTION_REINIT_REQUEST",
	NULL };

static fPostDataCB   post_message = NULL;

char version[1024] = {0};

static motion_manager  g_device;

ROBOTMODULE_API bool  Init()
{
	return true;
}


ROBOTMODULE_API  bool ROBOTMODULE_API_M Start()
{
	return g_device.Setup();
}

ROBOTMODULE_API bool ROBOTMODULE_API_M Stop()
{
	g_device.Finish();
	return true;
}

ROBOTMODULE_API const char* ROBOTMODULE_API_M GetModuleName()
{
	return MODULE_NAME;
}

ROBOTMODULE_API const char* ROBOTMODULE_API_M GetModuleVersion()
{
	if (strlen(version) == 0)
		g_device.get_version(version);
	return version;
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
	headcontrol_message temp;
	temp.appid = dAppid;
	temp.messagetype = dMessageType;
	temp.fromguid = strFrom;
	temp.content = strMessage;
	return g_device.message_handler(temp);
}

ROBOTMODULE_API void ROBOTMODULE_API_M SetPostFuntion(fPostDataCB ptr)
{
	if (ptr != NULL)
	{
		post_message = ptr;
		robot_PostMessage::getPostManager()->set(ptr);
	}

}
