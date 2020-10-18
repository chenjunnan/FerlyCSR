#pragma once
#include "robot_voice_tts_interface.h"
#include "BaiduSpeechApi.h"
#include <unordered_map>
class robot_voice_tts_baidu_ability :
	public robot_voice_tts_source_interface
{
public:

	static robot_voice_tts_baidu_ability*getInstanse();

	virtual int		InitEngine();

	virtual void	UninitEngine();

	virtual int		RequestTTS(int mode, std::string text);

	virtual void	Cancel();

	virtual void	EndTTS();

	virtual int		GetVoiceData(unsigned char *& data, unsigned int &len, TTS_STATUS &state);//<0 error ==0 ok continu data check len ==1 end

	virtual int		SetParam(int type, std::string key, std::string value);

	virtual int		GetParam(int type, std::string key, std::string &value);

	virtual ~robot_voice_tts_baidu_ability();
private:

	int read_from_cache(std::string text);
	void check_path();
	void build_cache(std::string text);
	void load_cacheindex();
	void write_cacheindex();
	robot_voice_tts_baidu_ability();

	std::vector<unsigned char>  pcm_cache;
	std::vector<std::string>    io_string_spit;

	CHANDLE						m_thrGuard;
	BaiduSpeechApi *								_baiduaip;
	std::map<std::string, std::string>				_baidumap;
	std::unordered_map<std::string, std::string>	_history_cache;
	std::string										_ttscachepath;
	bool											_shouldupdate;
	bool											_cache;
	int												_maxlen;
};

