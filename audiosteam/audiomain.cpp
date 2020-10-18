#include "memorySwaper.h"
#include "RobotModule_interface.h"
#include "xCommon.h"
#include "AudioManager.h"
#include "robot_PostMessage.h"
#include <time.h>

const char *MODULE_NAME = "audio_stream_module";

static const char *HANDLE_LIST[] = { 
"AUDIO_START_STREAMNAME_REQUEST",//0
"AUDIO_STOP_STREAMNAME_REQUEST", NULL };

static fPostDataCB   post_message = NULL;

static int g_audio_count = 0;

static AudioManager g_am;

#define VERSION_MODULE	"02.01"//大版本.模块号
#define MODULE_CLASS    1     //模块子版本类型
#define SUBVERSION      1     //模块小版本号.编译时间

char  version[1024] = { 0 };

void  get_version(char *version);

ROBOTMODULE_API bool  Init()
{
	if (!common_config_get_field_int("Audio", "audiocount", &g_audio_count))
	{
		LOGF("cant read audio stream config");
		return false;
	}
	LOGI("audio stream config count:%d ", g_audio_count);
	return true;
}


ROBOTMODULE_API  bool ROBOTMODULE_API_M Start()
{
	int audio_count = g_audio_count;
	while (audio_count)
	{
		audio_count--;
		char name_buffer[1024] = { 0 };
		sprintf_s(name_buffer, 1024, "AudioStreamConfig_%d", audio_count);
		AUDIO_INFO pau_in;
		pau_in.nDeviceID = audio_count;
		int enable = 0;
		if (!common_config_get_field_int(name_buffer, "enable", &enable))
		{
			LOGE("cant read audio %d enable", audio_count);
			continue;
		}
		if (!enable)continue;

		if (!common_config_get_field_string(name_buffer, "streamname", &pau_in.streamsname))
		{
			LOGE("cant read audio %d stream name", audio_count);
			continue;
		}
		if (!common_config_get_field_int(name_buffer, "samples", &pau_in.nSampleRate))
		{
			LOGE("cant read audio %d samples", audio_count);
			continue;
		}
		if (!g_am.add_audio(pau_in))
			continue;

	}
	return true;
}

ROBOTMODULE_API bool ROBOTMODULE_API_M Stop()
{
	g_am.stopall();
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
	AUDIO_MESSAGE acm;
	acm.appid = dAppid;
	acm.from = strFrom;
	acm.context = strMessage;
	acm.messagetype = dMessageType;
	return g_am.handle_message(acm);
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
 
