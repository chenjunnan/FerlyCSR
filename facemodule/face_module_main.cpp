#include "memorySwaper.h"
#include "RobotModule_interface.h"
#include "xCommon.h"
#include "robot_PostMessage.h"
#include "robot_face_manager.h"
#include "robot_face_msghandler.h"


const char *MODULE_NAME = "face_module";

static const char *HANDLE_LIST[] = {
"START_FACE_DETECT_SERVICE_REQUEST",//0
"STOP_FACE_DETECT_SERVICE_REQUEST",
"FACE_DEBUG_REQUEST", 
"FACE_INFO_BEGIN_REQUEST",
"FACE_INFO_STOP_REQUEST",
"FACE_SNAPSHOT_REQUEST",
"FACE_SNAPSHOT_CANCEL_REQUEST",
"FACE_REGISTER_REQUEST",
"FACE_IDENTIFY_BEGIN_REQUEST",
"FACE_IDENTIFY_STOP_REQUEST",
"FACE_REMOVE_REQUEST",
"FACE_NAMELIST_REQUEST",
"FACE_STATUS_REQUEST",
"CAMERA_SNAPSHOT_REQUEST",//move to other moudle
NULL };

static fPostDataCB   post_message = NULL;

#define VERSION_MODULE	"02.04"//大版本.模块号
#define MODULE_CLASS    1     //模块子版本类型
#define SUBVERSION      6     //模块小版本号.编译时间

char  version[1024] = { 0 };

void  get_version(char *version);

ROBOTMODULE_API bool  Init()
{
	robot_face_msghandler::getPtr()->init();

	for (int i = 0; i < sizeof(HANDLE_LIST) / sizeof(char*); i++)
	{
		if (HANDLE_LIST[i] == NULL)
			break;
		robot_face_manager::getPtr()->add_message_type(HANDLE_LIST[i]);
	}
	return robot_face_manager::getPtr()->init();
}


ROBOTMODULE_API  bool ROBOTMODULE_API_M Start()
{
	robot_face_manager::getPtr()->start();
	return robot_face_msghandler::getPtr()->start();
}

ROBOTMODULE_API bool ROBOTMODULE_API_M Stop()
{
	robot_face_manager::getPtr()->stop();
	robot_face_msghandler::getPtr()->quit();

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
	return  robot_face_manager::getPtr()->message_handler(dAppid, dMessageType, strFrom, strMessage, dMessagelen, strExtdata, dExtlen);
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