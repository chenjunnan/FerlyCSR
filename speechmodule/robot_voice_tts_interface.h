#pragma once
#include <map>
#include "common_helper.h"
#include "robot_voice_struct.h"


class robot_voice_tts_interface :public Thread_helper
{
public:
	robot_voice_tts_interface();

	virtual int  	Init(int index) = 0;

	virtual void	UnInit() = 0;

	virtual int     StartTTS(tts_message &tsm)=0;

	virtual int     TTSState() = 0;

	virtual int		GetError() = 0;

	virtual void    Cancel(int channel) = 0;

	virtual void    CancelAll() = 0;

	virtual void    Pause(int channel) = 0;

	virtual void    Resume(int channel) = 0;

	virtual int		setParam(int type, std::string key, std::string value) = 0;

	virtual int		getParam(int type, std::string key, std::string &value) = 0;

	virtual ~robot_voice_tts_interface();

protected:
};

enum TTS_STATUS
{
	WAITTTSDATA = 0,
	HASTTSDATA,
	ENDTTSDATA,

};

class robot_voice_tts_source_interface
{
public:
	virtual ~robot_voice_tts_source_interface(){}

	virtual int		InitEngine()= 0;

	virtual void	UninitEngine() = 0;

	virtual void	Cancel() = 0;

	virtual int		RequestTTS(int mode,std::string text) = 0;//mode 0 cloud or 1 local, if not support local return -50003 ,-50002 is not mutithread ,-50001 is local error

	virtual int		GetVoiceData(unsigned char *& data, unsigned int &len, TTS_STATUS &state) = 0;// return 0 is ok ,other is bad return ,if 0 check state

	virtual void	EndTTS() = 0;

	virtual int		SetParam(int type, std::string key, std::string value) = 0;

	virtual int		GetParam(int type, std::string key, std::string &value) = 0;
};




