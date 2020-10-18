#pragma once
#include "robot_json_rsp.h"
#include "robot_voice_process.h"
#include "robot_voice_struct.h"
class robot_voice_manager
{
public:

	static robot_voice_manager *getInstance();

	bool   init();

	bool   start();

	void   stop();

	void   add_message_type(const char * message);

	int    message_handler(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata = NULL,
		unsigned long dExtlen = 0);

	void start_isr_write_status(std::string streams, int speechid, int error, VoiceMessage &vmsg);

	virtual~robot_voice_manager();

private:
	void post_report(VoiceMessage &vmsg);

	void get_statusfilepath(std::string &exepath);
	void check_status();
	void write_status(int id, int pos, int state);
	robot_voice_manager();

	robot_json_rsp      m_json;
	std::vector<std::string >  m_message_list;
	int       m_mainmic;


	std::map<int,robot_voice_process*>  m_speech_handler;


};

