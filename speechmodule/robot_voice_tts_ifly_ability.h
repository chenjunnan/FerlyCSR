#pragma once
#include "robot_voice_tts_interface.h"
#include "robot_global_singleton.h"
#include "ifly\msp_types.h"
class robot_voice_tts_ifly_ability:
	public robot_voice_tts_source_interface,
	public robot_global_autointerface
{
public:
	static robot_voice_tts_ifly_ability*getInstanse();

	virtual int		InitEngine();

	virtual void	UninitEngine();

	virtual void	Cancel();

	virtual int		RequestTTS(int mode, std::string text) ;

	virtual void	EndTTS();

	virtual int		GetVoiceData(unsigned char *& data, unsigned int &len, TTS_STATUS &state);//<0 error ==0 ok continu data check len ==1 end

	virtual int		SetParam(int type, std::string key, std::string value);

	virtual int		GetParam(int type, std::string key, std::string &value);

	virtual ~robot_voice_tts_ifly_ability();
private:
	robot_voice_tts_ifly_ability();
	
	virtual int onInit() ;
	virtual int onUninit();

	void config_local_params();
/*	int  try_apply_session(bool netstatus);*/
	int  apply_iFly_session(std::map<std::string, std::string>& tts_params);
	void config_cloud_params();
	void common_params(std::map<std::string, std::string> &map_in);
	void close_ifly_session();


	CHANDLE							   m_thrGuard;
	std::map<std::string, std::string> tts_localparams_map;
	std::string						   default_type;
	std::map<std::string, std::string> tts_cloudparams_map;

	const char *	m_session_id;
	bool			isInit;
	int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;
};

