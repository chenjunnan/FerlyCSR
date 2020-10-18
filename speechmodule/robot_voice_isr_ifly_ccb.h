#pragma once
#include "robot_voice_isr_interface.h"
#include <map>
#include "robot_media_publish.h"
#include "robot_voice_com.h"
#include "robot_voice_isr_ify.h"
#include "robot_json_rsp.h"
#include "robot_global_singleton.h"
#ifdef CCB_OFFLINEIFLY // CCB_OFFLINEIFLY


class robot_fuck_hemei;

class robot_voice_isr_ifly_ccb :
	public robot_voice_isr_interface,
	public robot_global_autointerface
{
public:
	robot_voice_isr_ifly_ccb();
	virtual ~robot_voice_isr_ifly_ccb();
	virtual void SetConfig(int mic_no);

	virtual int  Init();

	virtual void UnInit();

	virtual void Recoder(bool);

	virtual int StartISR(VoiceMessage &vm);

	virtual int StartISR_Rec(int error, VoiceMessage &vm);

	virtual int  GetError();

	virtual void  CancelISR();

	virtual bool EnhanceMic(int angle);

	virtual bool  SleepMic();

	virtual bool GetInfo(speech_process_info &info);

	virtual int setParam(int type, std::string key, std::string value);

	virtual int getParam(int type, std::string key, std::string &value);

	virtual int onInit();
	virtual int onUninit();

	friend class robot_fuck_hemei;
private:
	virtual void run();

	void store_pcm();

	int isr_restart(int &error_count);

	bool apply_iFly_session();

	void close_iFly_session();

	std::string handler_isr_result();

	int msg_handler();


	bool			m_init;
	bool			m_recoder;
	std::string		m_streamstring;
	std::string		m_name;
	int				m_mic_no;
	bool			m_loop;

	std::string		m_config_session;

	std::map<std::string, std::string> isr_params_map;

	std::vector<std::string >			result_list;
	int				m_error;
	bool			m_iflycom_online;
	bool			m_iflycom_sleep;

	robot_media_publish	m_media;
	robot_voice_com	m_iflycom;

	volatile int	m_ctrol;
	const char *	m_session_id;
	int				m_endpointer_state;
	int				m_rec_result_state;
	int				m_push_audio_status;
	ISRSTATE		m_isrstate;
	robot_json_rsp	m_jsonhelper;
	VoiceMessage    m_message;
	int m_samplerate;


	int							m_session_count;
	robot_fuck_hemei			*m_session_appect;

	std::string			     m_session_str;
	HANDLE				     m_wait_mutex;
	CHANDLE			         m_session_mutex;
	std::list<std::string> m_session_list;
};


class robot_fuck_hemei :public Thread_helper
{
public:
	robot_fuck_hemei(robot_voice_isr_ifly_ccb*  rviic) :m_cb(rviic){ m_count = 1; }
	virtual ~robot_fuck_hemei(){}

	void setappect(std::string params, int count)
	{
		m_count = count;
		m_params = params;
		start();
	}

	virtual void run();
	
	std::string				  m_params;
	int						  m_count;
	robot_voice_isr_ifly_ccb *m_cb;
};

#endif // CCB_OFFLINEIFLY
