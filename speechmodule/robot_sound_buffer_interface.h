#pragma once
#include <vector>
#include "xCommon.h"
#include "common_helper.h"
#include "soundplayer.h"
#include "robot_voice_struct.h"
#include "robot_PostMessage.h"
#include "robot_json_rsp.h"

#define  TTS_TEXT   1001
#define  TTS_STOP	1002
#define  TTS_PROCESS	1003
#define  TTS_PAUSE  	1004
#define  TTS_RESUME 	1005
#define  TTS_STOPALL	1006
#define  TTS_FINISH	1007
struct cache_voice_info
{
	std::string context;
	std::vector<unsigned char > data;
};
class voice_recoder_finish
{
public:
	virtual ~voice_recoder_finish(){}
	virtual void onFinish(tts_message & id, std::vector<unsigned char>&) = 0;
};

class voice_play_finish
{
public:
	virtual ~voice_play_finish(){}
	virtual void onFinish(int id) = 0;
};


class cache_buffer:
	public robot_sound_buffer_interface
{
public:
	cache_buffer(voice_recoder_finish* fun){
		m_notify = fun,isstop = false;
	}
	virtual ~cache_buffer(){ }
	virtual int push_data(unsigned char *ptr, int len)
	{
		LOCK_HELPER_AUTO_HANDLER(buffer_ml)
		if (isstop)return -1;
		m_cache.insert(m_cache.end(), ptr, ptr + len);
		return len;
	}
	virtual void discard()
	{
		LOCK_HELPER_AUTO_HANDLER(buffer_ml)
		isstop = true;
		m_cache.clear();
	}
	virtual void clear()
	{
		LOCK_HELPER_AUTO_HANDLER(buffer_ml)
		m_cache.clear();
		isstop = false;
	}
	virtual void done()
	{
		if (m_notify)
			m_notify->onFinish(m_ms, m_cache);
	}
	virtual bool isStop()
	{
		//LOCK_HELPER_AUTO_HANDLER(buffer_ml)
		return isstop;
	}
	virtual int  status()
	{
		return isstop;
	}
	virtual int  work()
	{
		isstop = false;
		return 0;
	}

	void set_message(tts_message &ms)
	{
		m_ms = ms;
	}
private:
	std::vector<unsigned char> m_cache;
	bool isstop;
	tts_message m_ms;
	voice_recoder_finish* m_notify;
};



class player_warp :
	public robot_sound_buffer_interface,
	public SoundStateNotify
{
	virtual void onResume()
	{
		;
	}
	virtual void onPause()
	{
		;
	}
	virtual void onFinsih()
	{
		if (parent_nf)parent_nf->onFinish(m_id);
		std::string thistext;
		{
			LOCK_HELPER_AUTO_HANDLER(buffer_ml)
			if (isstop != PLAY_STATUS_WAI_FINIFSH)
				return;
			if (_text.empty())
				return;
			thistext = _text;
		}

		std::string out = m_jsonhelper.read_over_ntf(m_id, thistext);
		robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length());
	}
	virtual void onProcess(double rate)
	{

	}

public:
	player_warp(){

		player = Getplayer();
		player->brccache();
		player->set_notify(this);
		clear();
	}
	virtual ~player_warp(){ 
		Destoryplayer(player);
	}


	void setid(int id, voice_play_finish* tp)
	{
		m_id = id;
		parent_nf = tp;
	}

	virtual int push_data(unsigned char *ptr, int len)
	{
		LOCK_HELPER_AUTO_HANDLER(buffer_ml)
		if (isstop != PLAY_STATUS_WAIT_DATA)
			return -1;
		if (!isAlreadyReset)player->reset();
		isAlreadyReset = true;
		player->sound(ptr, len);
		return len;
	}
	virtual void discard()
	{
		LOCK_HELPER_AUTO_HANDLER(buffer_ml)
		if (isstop == PLAY_STATUS_STOP)return;
		isstop = PLAY_STATUS_STOP;
		isAlreadyReset = false;
		if (!player->isfinsh())
			player->stop();
	}
	virtual void clear()
	{
		LOCK_HELPER_AUTO_HANDLER(buffer_ml)
		isstop = PLAY_STATUS_RESET;	
		isAlreadyReset = false;
		_text.clear();
	}
	virtual void done()
	{
		LOCK_HELPER_AUTO_HANDLER(buffer_ml)
		if (isstop == PLAY_STATUS_STOP)return;
		isstop = PLAY_STATUS_WAI_FINIFSH;
		player->endsound();
	}
	virtual bool isStop()
	{
		LOCK_HELPER_AUTO_HANDLER(buffer_ml);
		return (isstop == PLAY_STATUS_STOP || isstop == PLAY_STATUS_RESET);
	}
	virtual int  status()
	{
		LOCK_HELPER_AUTO_HANDLER(buffer_ml)
		return isstop;
	}
	virtual int  work()
	{
		LOCK_HELPER_AUTO_HANDLER(buffer_ml)
		if (isstop == PLAY_STATUS_STOP)
			return -1;
		isstop = PLAY_STATUS_WAIT_DATA;
		return 0;
	}
	CSoundPlayer * get_player()
	{
		return player;
	}

private:
	CSoundPlayer *player;
	voice_play_finish *parent_nf;
	int  m_id;
	int isstop;
	bool isAlreadyReset;

	robot_json_rsp	m_jsonhelper;
};
