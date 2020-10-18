#pragma once
#include <map>
#include "winrec.h"
#include "audio_stream_capture.h"
#include <set>
extern  const char *MODULE_NAME;

struct  AUDIO_MESSAGE
{
	int appid = 0;
	int messagetype = 0;
	std::string from;
	std::string context;
};

class AudioManager
{
public:
	AudioManager();
	bool add_audio(AUDIO_INFO &config);
	int  handle_message(AUDIO_MESSAGE &acm);
	void remove_audio(std::string);
	void stopall();
	~AudioManager();
private:
	std::map<std::string, audio_stream_capture*> m_list;
	std::map<std::string, std::set<std::string> > m_listenlist;
};

