// expressionmodule.cpp : 定义 DLL 应用程序的导出函数。
//


#include "RobotModule_interface.h"
#include "xCommon.h"
#include "robot_PostMessage.h"
#include "robot_expression_module.h"

const char *MODULE_NAME = "expression_module";

static const char *HANDLE_LIST[] = {
"EXPRESSION_CHANGE_REQUEST",
"EXPRESSION_CHANGE_RESPONSE",
NULL };

static fPostDataCB   post_message = NULL;

robot_expression_module robot_expression;


ROBOTMODULE_API bool  Init()
{
	return true;
}


ROBOTMODULE_API  bool ROBOTMODULE_API_M Start()
{
	robot_expression.init();
	return true;
}

ROBOTMODULE_API bool ROBOTMODULE_API_M Stop()
{
	robot_expression.stop();
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
	Expression_task temp;
	temp.appid = dAppid;
	temp.messagetype = dMessageType;
	temp.fromguid = strFrom;
	temp.content = strMessage;
	temp.times = common_get_longcur_time_stampms();	
	return robot_expression.message_handler(temp);
}

ROBOTMODULE_API void ROBOTMODULE_API_M SetPostFuntion(fPostDataCB ptr)
{
	if (ptr != NULL)
	{
		post_message = ptr;
		robot_PostMessage::getPostManager()->set(ptr);
	}
		
}
