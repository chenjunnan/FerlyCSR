// IDcardmodule.cpp : 定义 DLL 应用程序的导出函数。
//


#include "RobotModule_interface.h"
#include "xCommon.h"
#include "robot_PostMessage.h"
#include "robot_idcard_module.h"

const char *MODULE_NAME = "idcard_module";

static const char *HANDLE_LIST[] = {
	"IDCARD_WAIT_REQUEST",
	"IDCARD_CANCEL_REQUEST",
	NULL };

static fPostDataCB   post_message = NULL;

static robot_idcard_module  g_device;

ROBOTMODULE_API bool  Init()
{
	return true;
}


ROBOTMODULE_API  bool ROBOTMODULE_API_M Start()
{
	return g_device.start();
}

ROBOTMODULE_API bool ROBOTMODULE_API_M Stop()
{
	g_device.quit();
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
	idcard_message temp;
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
