#pragma once
#include <windows.h>

#include "common_helper.h"
#include "robot_voice_struct.h"
#include "robot_voice_media.h"
#include "xCommon.h"
#include "robot_media_publish.h"
#include "json/json.h"
struct  speech_process_info
{
	std::string stream_name;
	bool  loop;
	std::string semname;
	speech_process_info()
	{
		loop = false;
	}
};



class robot_voice_isr_interface :
	public Thread_helper,
	public robot_voice_data_interface
{
public:
	robot_voice_isr_interface();


	virtual void SetConfig(int mic_no) = 0;

	virtual int  Init() = 0;// 开启语音服务

	virtual void UnInit()=0;// 关闭语音服务

	virtual void Recoder(bool)=0;//记录，或结束记录

	virtual int  StartISR(VoiceMessage &vm) = 0;//开启一次语音识别

	virtual int  StartISR_Rec(int error, VoiceMessage &vm) = 0;//处理录音回包

	virtual int  GetError() = 0;//获取一次错误

	virtual void  CancelISR()=0;//取消识别

	virtual bool  EnhanceMic(int angle) = 0;

	virtual bool  SleepMic() = 0;

	virtual bool GetInfo(speech_process_info &info) = 0;

	virtual int setParam(int type,std::string key, std::string value) = 0;

	virtual int getParam(int type, std::string key, std::string &value) = 0;

	virtual void inFlush();

	virtual void inData(robot_voice_media &data);

	virtual ~robot_voice_isr_interface();
protected:

	void setCachelenght(int lenght);

	bool getCacheMedia(robot_voice_media& data,int timeout);

	void Mediadone();

	bool getHeadMedia(robot_voice_media &data);

	int open_stored_file(const char * name);

	int loopwrite_to_file(char *data, size_t length);

	void safe_close_file();

	void open_audio(std::string readername, std::string streamname);

	void close_audio(std::string readername, std::string streamname);

	void init_first_voicecache(std::string filed);


	FILE *			fdwav;

	HANDLE         m_buffer_ev;
	CHANDLE		   m_buffer_lock;
	int			   m_maxcachevoice;
	int			   m_usermaxcahce;

	std::vector<robot_voice_media>    m_media_cache;
	unsigned char					  m_media_inindex;
	unsigned char					  m_media_outindex;

};

