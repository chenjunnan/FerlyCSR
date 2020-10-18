// PsycheClient.cpp : ���� DLL Ӧ�ó���ĵ���������
//
#include <time.h>
#include <windows.h>
#include "RobotModule_interface.h"
#include "xCommon.h"
#include "robot_PostMessage.h"
#include "PsycheManager.h"

const char *MODULE_NAME = "psycheclient_module";

static const char *HANDLE_LIST[] = {
	"PSYCHEL_APPLYFOR_INTERVENTION_REQUEST",//0
	"PSYCHEL_ABANDON_INTERVENTION_REQUEST",
	"PSYCHEL_POST_MESSAGEDATA_REQUEST",
	"PSYCHEL_GET_STATUS_REQUEST",
	"PSYCHEL_SHUTUP_REQUEST",
	"PSYCHEL_SPEAK_REQUEST",
	NULL };

static fPostDataCB   post_message = NULL;


#define VERSION_MODULE	"02.20"//��汾.ģ���
#define MODULE_CLASS    1     //ģ���Ӱ汾����
#define SUBVERSION      1     //ģ��С�汾��.����ʱ��

char  version[1024] = { 0 };

void  get_version(char *version);

ROBOTMODULE_API bool  Init()
{
	return PsycheManager::getInstance()->init();
}


ROBOTMODULE_API  bool ROBOTMODULE_API_M Start()
{
	return PsycheManager::getInstance()->StartManager();
}

ROBOTMODULE_API bool ROBOTMODULE_API_M Stop()
{
	return PsycheManager::getInstance()->StopManager();
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
	return PsycheManager::getInstance()->HandleMessage(dAppid,dMessageType,strFrom,strMessage,dMessagelen,strExtdata,dExtlen);
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
