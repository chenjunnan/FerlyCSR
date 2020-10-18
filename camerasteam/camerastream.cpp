// camerasteam.cpp : 定义 DLL 应用程序的导出函数。
//
#include "memorySwaper.h"
#include "RobotModule_interface.h"
#include "xCommon.h"
#include "VideoManager.h"
#include "DeviceNotify.h"
#include "robot_PostMessage.h"


const char *MODULE_NAME = "video_stream_module";

static const char *HANDLE_LIST[] = { 
"CAMERA_START_STREAMNAME_REQUEST",//0
"CAMERA_STOP_STREAMNAME_REQUEST",
"CAMERA_SETPARAMETER_STREAMNAME_REQUEST",
"CAMERA_GETPARAMETER_STREAMNAME_REQUEST",
"CAMERA_LIST_STREAMNAME_REQUEST",
"CAMERA_CHANGE_USER_REQUEST",
"CAMERA_RESUME_USER_REQUEST",
NULL };

static fPostDataCB   post_message = NULL;
#define VERSION_MODULE	"02.02"//大版本.模块号
#define MODULE_CLASS    1     //模块子版本类型
#define SUBVERSION      3     //模块小版本号.编译时间

char  version[1024] = { 0 };

void  get_version(char *version);

ROBOTMODULE_API bool  Init()
{
	return VideoManager::get()->Init();;
}

ROBOTMODULE_API  bool ROBOTMODULE_API_M Start()
{
	return  VideoManager::get()->start();
}

ROBOTMODULE_API bool ROBOTMODULE_API_M Stop()
{
	VideoManager::get()->stop_all();
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



ROBOTMODULE_API int ROBOTMODULE_API_M SetProcessData(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, 
	const unsigned char *strExtdata /*= NULL*/,unsigned long dExtlen /*= 0*/)
{
	CAMERA_MESSAGE cm;
	cm.appid = dAppid;
	cm.context = strMessage;
	cm.messagetype = dMessageType;
	cm.from = strFrom;
	VideoManager::get()->message_handler(cm);
	return 0;
}

ROBOTMODULE_API void ROBOTMODULE_API_M SetPostFuntion(fPostDataCB ptr)
{
	if (ptr != NULL)
	{
		post_message = ptr;
		robot_PostMessage::getPostManager()->set(ptr);
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
