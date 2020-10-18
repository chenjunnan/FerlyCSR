#pragma once
#include "common_helper.h"
#include "robot_voice_tts_interface.h"
#include "robot_voice_isr_interface.h"
#include "robot_voice_struct.h"
struct Speech_Status
{
	bool isr_run_state;
	int isr_error;
	bool tts_run_state;
	int tts_error;
};

class robot_voice_process 
{
public:
	robot_voice_process();

	void init(int);
	int start_voice_service();
	bool stop_voice_service();
	void get_status(Speech_Status&ss);

	int get_isr(std::string key, std::string &value);
	int set_isr(std::string key, std::string value);
	int start_isr(VoiceMessage &vm);
	int start_isr_result(int error, VoiceMessage &vm);
	bool stop_isr();
	bool enhance_mic(int angle);
	bool reset_mic();

	int get_tts(int type,std::string key, std::string &value, int channel);
	int set_tts(int type,std::string key, std::string value, int channel);

	int tts(tts_message &tsm);
	void stop_tts(int channel);
	void pause(int channel);
	void resume(int channel);

	virtual ~robot_voice_process();

	bool getInfo(speech_process_info &info);

private:

	void release_tts();
	void stop_all();
	robot_voice_isr_interface *m_rvin;
	robot_voice_tts_interface *m_rout;
	robot_voice_tts_source_interface *ttssource;
	void change_isr(std::string product_name);
	void change_tts(std::string product_name);

	std::string		m_config_session;
	int				m_mic_no;
};

