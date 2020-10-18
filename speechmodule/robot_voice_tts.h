#pragma once
#include "robot_voice_tts_interface.h"
#include "soundplayer.h"
#include "robot_json_rsp.h"
#include "robot_internetcheck.h"
#include "robot_global_singleton.h"
#include "robot_voice_struct.h"
#include "robot_sound_buffer_interface.h"
#include "robot_voice_tts_ifly_ability.h"
#include <mutex>
#include <condition_variable>

class robot_voice_tts :
	public robot_voice_tts_interface,
	public internet_notify,
	public voice_recoder_finish,
	public voice_play_finish
{
public:
	robot_voice_tts(robot_voice_tts_source_interface *);

	virtual int  	Init(int index);

	virtual void	UnInit() ;

	virtual int     StartTTS(tts_message &tsm);

	virtual int     TTSState();

	virtual int		GetError();

	virtual void    Cancel(int channel);

	virtual void    CancelAll();

	virtual void    Pause(int channel);

	virtual void    Resume(int channel);

	virtual int		setParam(int type, std::string key, std::string value);

	virtual int		getParam(int type, std::string key, std::string &value);

	virtual ~robot_voice_tts();

private:
	virtual void onFinish(int id);

	virtual void run();

	virtual void onFinish(tts_message & id, std::vector<unsigned char>&);

	virtual void netnotify(int state);

	void		ntf_tts_error();

	void		error_handle();

	void		callEndTTS(bool use_other_local);
	void		end_request(bool &request_state, bool use_other_local);
	bool		check_stop(tts_message &task, bool &task_state);

	void		clean_tts();
	void		handle_remain_task();
	void		clean_old_ttsmessage(int channels);

	robot_voice_tts_source_interface *  m_speechengine;

	std::string							m_tts_context;

	CHANDLE								m_player_guard;
	std::map<int, player_warp>			m_player;

	std::mutex							m_currentchannels;
	std::condition_variable				m_condwakeup;
	std::list< tts_message>				m_channel_list;
	int									m_active_channel;
	std::vector<tts_message>			m_newmessage;

	cache_buffer						m_temp_cache;
	std::map<int, cache_voice_info>     m_cache_list;

	robot_json_rsp						m_jsonhelper;
	bool								m_net_status;
	robot_internetcheck					m_net_checker;
	bool								m_checker_onoff;
	int									m_error;
	bool								m_init;
	int									m_wait_fasttime ;

};

