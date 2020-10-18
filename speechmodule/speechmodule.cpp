// speechmodule.cpp : 定义 DLL 应用程序的导出函数。
//
#include <windows.h>
#include "memorySwaper.h"
#include "RobotModule_interface.h"
#include "xCommon.h"
#include "robot_voice_manager.h"
#include "robot_PostMessage.h"
const char *MODULE_NAME = "speech_module";


static const char *HANDLE_LIST[] = {
	"START_SPEECH_SERVICE_REQUEST",//0
	"START_ONCE_ISR_REQUEST",
	"STOP_ONCE_ISR_REQUEST",
	"STOP_SPEECH_SERVICE_REQUEST",
	"START_TTS_REQUEST",
	"STOP_TTS_REQUEST",
	"SET_TTS_PARAMETER_REQUEST",
	"SET_ISR_PARAMETER_REQUEST",
	"GET_SPEECH_STATUS_REQUEST",
	"PAUSE_TTS_REQUEST",
	"RESUME_TTS_REQUEST",
	"ENHANCE_MIC_REQUEST",
	"RESET_MIC_REQUEST",
	"GET_TTS_PARAMETER_REQUEST",
	"GET_ISR_PARAMETER_REQUEST",
	NULL };

static fPostDataCB   post_message = NULL;
#define VERSION_MODULE	"02.08"//大版本.模块号
#define MODULE_CLASS    1     //模块子版本类型
#define SUBVERSION      9     //模块小版本号.编译时间

char  version[1024] = { 0 };

void  get_version(char *version);
ROBOTMODULE_API bool  Init()
{
	for (int i = 0; i < sizeof(HANDLE_LIST) / sizeof(char*); i++)
	{
		if (HANDLE_LIST[i] == NULL)
			break;
		robot_voice_manager::getInstance()->add_message_type(HANDLE_LIST[i]);
	}
	

	return robot_voice_manager::getInstance()->init();
}


ROBOTMODULE_API  bool ROBOTMODULE_API_M Start()
{
	return robot_voice_manager::getInstance()->start();
}

ROBOTMODULE_API bool ROBOTMODULE_API_M Stop()
{
	robot_voice_manager::getInstance()->stop();
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
	return robot_voice_manager::getInstance()->message_handler(dAppid,dMessageType,strFrom,strMessage,dMessagelen,strExtdata);
}

ROBOTMODULE_API void ROBOTMODULE_API_M SetPostFuntion(fPostDataCB ptr)
{
	if (ptr != NULL)
	{
		post_message = ptr;
		robot_PostMessage::getPostManager()->set(post_message);
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