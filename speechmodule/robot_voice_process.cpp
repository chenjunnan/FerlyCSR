#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>   
#include <process.h>
#include <errno.h>
#include <mmeapi.h>
#include "robot_voice_process.h"
#include "common_helper.h"
#include "json/json.h"
#include "robot_voice_release.h"
#include "robot_voice_isr_ify.h"

#include "robot_voice_tts_ifly_ability.h"
#include "robot_voice_tts.h"



#ifdef CCB_OFFLINEIFLY
#include "robot_voice_isr_ifly_ccb.h"
#include "robot_voice_isr_ifly_ccb95533.h"
#pragma comment( lib, "./ccb/mt_scylla.lib" )
#endif // CCB_OFFLINEIFLY
#include "robot_voice_tts_baidu_ability.h"


#pragma comment(lib, "winmm.lib") 


robot_voice_process::robot_voice_process()
{
	m_rvin = NULL;
	m_rout = NULL;
	ttssource=NULL;
}


void robot_voice_process::change_isr(std::string product_name)
{
	if (m_rvin)
	{
		m_rvin->UnInit();
		SAFE_DELETE(m_rvin);
	}

	if (product_name.compare("ifly") == 0)
	{
		m_rvin = new robot_voice_isr_ify();
		m_rvin->SetConfig(m_mic_no);
	}

#ifdef CCB_OFFLINEIFLY
	if (product_name.compare("iflyccb") == 0)
	{
		m_rvin = new robot_voice_isr_ifly_ccb();
		m_rvin->SetConfig(m_mic_no);
	}
	if (product_name.compare("iflyccb95533") == 0)
	{
		m_rvin = new robot_voice_isr_ifly_ccb95533();
		m_rvin->SetConfig(m_mic_no);
	}
#endif // CCB_OFFLINEIFLY
}

void robot_voice_process::change_tts(std::string product_name)
{
	if (m_rout)
	{
		if (m_rout)
			m_rout->UnInit();
		release_tts();
	}
	

	if (product_name.compare("ifly") == 0)
		m_rout = new robot_voice_tts(robot_voice_tts_ifly_ability::getInstanse());
	if (product_name.compare("baidu") == 0)
		m_rout = new robot_voice_tts(robot_voice_tts_baidu_ability::getInstanse());
}


void robot_voice_process::init(int index )
{
	m_mic_no = index;
	char name_buffer[1024] = { 0 };
	sprintf_s(name_buffer, 1024, "SpeechConfig_%d", m_mic_no);
	m_config_session = name_buffer;

	std::string temp;
	if (!common_config_get_field_string(m_config_session.c_str(), "isrproduct", &temp))
		temp = "ifly";
	change_isr(temp);

	if (!common_config_get_field_string(m_config_session.c_str(), "ttsproduct", &temp))
		temp = "ifly";
	change_tts(temp);
	return;
}

int robot_voice_process::start_voice_service()
{
	int ISRret = 0,TTSret=0;
	bool ok = false;

	int m_error = robot_voice_tts_ifly_ability::getInstanse()->InitEngine();//make sure local tts ability can work
	if (m_error)
		return m_error;

	if (m_rout)
		TTSret = m_rout->Init(m_mic_no);

	if (m_rvin)
		ISRret = m_rvin->Init();
	if (!TTSret&&!ISRret)
		return 0;

	stop_voice_service();

	return TTSret == 0 ? ISRret : TTSret;
}


bool robot_voice_process::stop_voice_service()
{
	bool ok = true;

	if (m_rvin)
		m_rvin->UnInit();

	if (m_rout)
		m_rout->UnInit();

	robot_voice_tts_ifly_ability::getInstanse()->UninitEngine();
	return ok;
}

void robot_voice_process::get_status(Speech_Status&ss)
{
	if (m_rvin)
	{
		ss.isr_run_state = m_rvin->state();
		ss.isr_error = m_rvin->GetError();
	}
	if (m_rout)
	{
		ss.tts_run_state = m_rout->TTSState();
		ss.tts_error = m_rout->GetError();
	}
}

int robot_voice_process::get_isr(std::string key, std::string &value)
{
	transform(key.begin(), key.end(), key.begin(), tolower);
	if (!m_rvin)return 1;

	return m_rvin->getParam(0,key.c_str(), value);
}

int robot_voice_process::set_isr(std::string key, std::string value)
{
	transform(key.begin(), key.end(), key.begin(), tolower);
	if (!m_rvin)return 1;
	return m_rvin->setParam(0,key.c_str(), value.c_str());
}

int robot_voice_process::start_isr(VoiceMessage &vm)
{
	if (m_rvin)
		return m_rvin->StartISR( vm);
	return 1;
}

int robot_voice_process::start_isr_result(int error, VoiceMessage &vm)
{
	if (m_rvin)
		return m_rvin->StartISR_Rec(error, vm);
	return 1;
}

bool robot_voice_process::stop_isr()
{
	if (m_rvin)
		m_rvin->CancelISR();
	else
		return false;
	return true;
}



bool robot_voice_process::enhance_mic(int angle)
{
	if (m_rvin)
		return m_rvin->EnhanceMic(angle);
	else
		return false;
}

bool robot_voice_process::reset_mic()
{
	if (m_rvin)
		return m_rvin->SleepMic();
	else
		return false;
}

int robot_voice_process::get_tts(int type, std::string key, std::string &value, int channel)
{
	if (m_rout)
		return m_rout->getParam(type, key.c_str(), value);
	return 1;
}

int robot_voice_process::tts(tts_message &tsm)
{
	if (m_rout)
		return m_rout->StartTTS(tsm);
	return 1;
}


int robot_voice_process::set_tts(int type, std::string key, std::string value, int channel)
{
	if (m_rout)
		return m_rout->setParam(type, key, value);
	return 1;
}

void robot_voice_process::stop_tts(int channel)
{
	if (m_rout)
		return m_rout->Cancel(channel);
	return ;
}

void robot_voice_process::stop_all()
{
	if (m_rout)
		return m_rout->CancelAll();
}

void robot_voice_process::pause(int channel)
{
	if (m_rout)
		return m_rout->Pause(channel);
	return;
}

void robot_voice_process::resume(int channel)
{
	if (m_rout)
		return m_rout->Resume(channel);
	return;
}

robot_voice_process::~robot_voice_process()
{
	stop_all();
	stop_isr();
	stop_voice_service();
	SAFE_DELETE(m_rvin);
	release_tts();

}

void robot_voice_process::release_tts()
{
	SAFE_DELETE(m_rout);
}

bool robot_voice_process::getInfo(speech_process_info &info)
{
	if (m_rvin)
		return m_rvin->GetInfo(info);
	return false;
}
