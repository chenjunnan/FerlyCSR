#pragma once
#include "robot_voice_isr_interface.h"
#include "memorySwaper.h"
#include <map>
#include "robot_voice_com.h"
#include "robot_media_publish.h"
#include "robot_global_singleton.h"


#define MESG_START 0


class robot_voice_isr_ify :
	public robot_voice_isr_interface,
	public robot_global_autointerface
{
public:
	robot_voice_isr_ify();

	virtual void SetConfig(int mic_no);

	virtual int  Init();


	virtual void UnInit();

	virtual void Recoder(bool);

	virtual int StartISR(VoiceMessage &vm);

	virtual int StartISR_Rec(int error, VoiceMessage &vm);

	virtual int  GetError();

	virtual void  CancelISR();

	virtual bool  EnhanceMic(int angle);

	virtual bool SleepMic();

	virtual bool GetInfo(speech_process_info &info);


	virtual int setParam(int type, std::string key, std::string value);

	virtual int getParam(int type, std::string key, std::string &value);

	virtual int onInit();
	virtual int onUninit();

	virtual ~robot_voice_isr_ify();


private:
	virtual void run();

	void result_notify();
	void setSessionMap(std::string key, std::string defaultstring);

	void store_pcm();

	int isr_restart(int &error_count);

	bool apply_iFly_session();

	void close_iFly_session();

	bool handler_isr_result_one(std::string iflydata, std::string &result, std::vector<slot_id> &slot);
	bool handler_isr_result(std::string &result, std::vector<slot_id> &slot);

	int msg_handler();

	int get_tail_result();
	static int build_grm_cb(int ecode, const char *info, void *udata);
	static int update_lex_cb(int ecode, const char *info, void *udata);

	void trim_return(std::string &instr);
	int update_lexicon_imp(std::string slotname, std::vector<char> &data, std::string grammar);
	void replace_char(std::string &data, const char src, const char des);
	int readfile(std::string filename, std::vector<char> &out);
	int update_lexicon();

	int build_grammar();

	std::vector<std::string> split(std::string strtem, char a);
	bool			m_init;
	bool			m_recoder;
	std::string		m_streamstring;
	std::string		m_name;
	int				m_mic_no;
	bool			m_loop;
	int 			m_mode;
	std::string		m_resultTo;
	std::string		m_config_session;
	std::string		m_result;
	std::vector<slot_id> m_vecsolt;


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

	std::map<std::string,GrammarData> m_gramdatalist;

	CHANDLE					m_wait_sem;
	int					m_notify_status;
};

