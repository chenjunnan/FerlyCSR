#include "robot_voice_tts.h"
#include <windows.h>


robot_voice_tts::robot_voice_tts(robot_voice_tts_source_interface *speeech) :m_net_checker(this), m_temp_cache(this), m_speechengine(speeech)
{
	m_init = false;
	m_player_guard = common_mutex_create_threadlock();
	m_net_status = true;
	m_checker_onoff = false;
	m_error = 0;
	m_active_channel = -1;
	m_player[0].setid(0, this);
	m_player[0].clear();
}


int robot_voice_tts::Init(int index)
{
	if (m_init) return 0;

	int nTemp = 0;
	m_checker_onoff = false;
	if (common_config_get_field_int("ifly", "tts_timeout", &nTemp))//move ini filed 
	{
		if (nTemp >= 0)
		{
			m_checker_onoff = true;
			m_net_checker.setTimeout(nTemp);
		}
	}
	if (m_speechengine)
		m_error = m_speechengine->InitEngine();
	else
		return -1;

	if (m_error)
		return m_error;

	m_net_status = true;
	if (m_checker_onoff)m_net_checker.start();

	m_init = true;
	m_error = 0;
	return m_error;
}

robot_voice_tts::~robot_voice_tts()
{
	UnInit();
	common_mutex_threadlock_release(m_player_guard);
}


void robot_voice_tts::UnInit()
{
	if (!m_init)return;

	if (state())
		quit();

	m_init = false;

	if (m_checker_onoff&&m_net_checker.state())
		m_net_checker.quit();

	if (m_speechengine)
		m_speechengine->UninitEngine();
}

int robot_voice_tts::StartTTS(tts_message &tsm)
{
	if (!m_init)return 9999;

	WAVEFORMATEX wfx;
	wfx.wFormatTag = WAVE_FORMAT_PCM;//设置波形声音的格式
	wfx.nChannels = 1;//设置音频文件的通道数量
	wfx.nSamplesPerSec = 16000;//设置每个声道播放和记录时的样本频率
	wfx.nAvgBytesPerSec = 32000;//设置请求的平均数据传输率,单位byte/s。这个值对于创建缓冲大小是很有用的
	wfx.nBlockAlign = 2;//以字节为单位设置块对齐
	wfx.wBitsPerSample = 16;
	wfx.cbSize = 0;//额外信息的大小

	{
		LOCK_HELPER_AUTO_HANDLER(m_player_guard);
		if (m_player.find(tsm.channels) == m_player.end())
		{
			m_player[tsm.channels].setid(tsm.channels, this);
			m_player[tsm.channels].clear();
		}
	}


	if (m_player[tsm.channels].get_player()->init(wfx.nChannels, wfx.nSamplesPerSec, wfx.nAvgBytesPerSec, wfx.nBlockAlign, wfx.wBitsPerSample)){
		LOGE("Audio channels[%d] engine start failed .", tsm.channels);
		m_error = 596;
		return m_error;
	}

	if (!state() && !start())
		return 2;

	{
		std::unique_lock<std::mutex> g(m_currentchannels);
		if (m_active_channel == tsm.channels || m_active_channel == -1)
		{
			m_player[tsm.channels].discard();
			m_player[tsm.channels].clear();
			m_active_channel = tsm.channels;	
			m_newmessage.clear();
			m_newmessage.push_back(tsm);
			m_speechengine->Cancel();
			m_condwakeup.notify_all();
	
		}
		else
		{
			LOGI("channel [%d] used ,enter wait list..", m_active_channel);
			clean_old_ttsmessage(tsm.channels);
			m_channel_list.insert(m_channel_list.end(), tsm);
		}

	}
	return 0;
}

void robot_voice_tts::clean_old_ttsmessage(int channels)
{
	for (auto it = m_channel_list.begin(); it != m_channel_list.end();)
	{
		if (it->channels == channels)
			it = m_channel_list.erase(it);
		else
			it++;
	}
}

int robot_voice_tts::TTSState()
{
	if (state())
	{
		bool bstate = false;
		for (auto it = m_player.begin(); it != m_player.end(); it++)
		{
			if (!it->second.get_player()->isfinsh())
				bstate = true;
		}
		return bstate;
	}
	else
		return false;
}

int robot_voice_tts::GetError()
{
	return m_error;
}


void robot_voice_tts::Cancel(int channel)
{
	if (m_player.find(channel) == m_player.end())
		return;
	m_player[channel].discard();
	clean_old_ttsmessage(channel);
}

void robot_voice_tts::CancelAll()
{
	for (auto it = m_player.begin(); it != m_player.end(); it++)
	{
		m_player[it->first].discard();
		clean_old_ttsmessage(it->first);
	}
}

void robot_voice_tts::Pause(int channel)
{
	if (m_player.find(channel) == m_player.end())
		return;
	m_player[channel].get_player()->pause();
}

void robot_voice_tts::Resume(int channel)
{
	if (m_player.find(channel) == m_player.end())
		return;
	m_player[channel].get_player()->resume();
}

int robot_voice_tts::setParam(int type, std::string key, std::string value)
{
	if (m_speechengine)return m_speechengine->SetParam(type, key, value);
	return -1;
}

int robot_voice_tts::getParam(int type, std::string key, std::string &value)
{
	if (m_speechengine)return m_speechengine->GetParam(type, key, value);
	return -1;
}


void robot_voice_tts::onFinish(tts_message & id, std::vector<unsigned char>&data)
{
	m_cache_list[id.id_cache].context = id.context;
	m_cache_list[id.id_cache].data = move(data);
	//TODO write file
}

void robot_voice_tts::onFinish(int id)
{
	LOGI("[robot speech] %d speak over", id);
}

void robot_voice_tts::clean_tts()
{
	m_tts_context.clear();
}


void robot_voice_tts::handle_remain_task()
{
	std::unique_lock<std::mutex> g(m_currentchannels);
	if (m_newmessage.size() != 0)
		return;
	bool hasTask = false;
	for (auto it = m_channel_list.begin(); it != m_channel_list.end(); it++)
	{
		m_active_channel = it->channels;
		m_newmessage.clear();
		m_newmessage.push_back(*it);
		m_channel_list.erase(it);
		hasTask = true;
		break;
	}
	if (!hasTask)
		m_active_channel = -1;
}


void robot_voice_tts::run()
{
	int wait_time = 0;
	bool loop = true;
	bool request_state = false;
	m_wait_fasttime = 20;
	tts_message task;
	Timer_helper	 requestbegin;
	bool			use_other_local = false;
	while (loop)
	{
		Message xMSG = getq(wait_time);

		switch (xMSG.msgType)
		{
		case _EXIT_THREAD_:
			loop = false;
			if (!m_tts_context.empty())
			{
				if (use_other_local)robot_voice_tts_ifly_ability::getInstanse()->EndTTS();
				else m_speechengine->EndTTS();
			}
			clean_tts();
			continue;
			break;
		default:
			;
		}
		{
			std::unique_lock<std::mutex> g(m_currentchannels);
			std::_Cv_status waistate = m_condwakeup.wait_for(g, std::chrono::milliseconds(m_wait_fasttime));
			if (m_newmessage.size() == 1)
			{
				task = m_newmessage[0];
				m_newmessage.clear();


				if (!m_tts_context.empty())
					callEndTTS(use_other_local);

				clean_tts();
				bool canwork = false;

				if (task.recoder)
				{
					m_temp_cache.set_message(task);
					task.handler.insert(task.handler.end(), &m_temp_cache);
					m_temp_cache.discard();
					m_temp_cache.clear();
					m_temp_cache.setText(task.context);

				}

				if (task.active)
				{
					LOCK_HELPER_AUTO_HANDLER(m_player_guard);
					task.handler.insert(task.handler.end(), &m_player[task.channels]);
					m_player[task.channels].setText(task.context);

					if (m_player[task.channels].work() == 0)
						canwork = true;
				}

				if (canwork)
				{
					request_state = false;
					m_tts_context = task.context;
					requestbegin.update();
					use_other_local = false;
				}

			}

		}


		if (m_tts_context.empty())continue;

		if (check_stop(task, request_state))
		{
			callEndTTS(use_other_local);
			continue;
		}


		if (!request_state)
		{
			int mode = 0;

			if (m_checker_onoff&&!m_net_status)
				mode = 1;

			m_error= m_speechengine->RequestTTS(mode, m_tts_context);

			if (m_error == -50003)
			{
				m_error = robot_voice_tts_ifly_ability::getInstanse()->RequestTTS(mode, m_tts_context);
				if (m_error==0)use_other_local = true;
			}
				
			if (m_error != 0 && requestbegin.isreach(1000))
			{
				error_handle();
				continue;
			}

			if (m_error == 0)
				request_state = true;
		}

		if (check_stop(task, request_state))
		{
			callEndTTS(use_other_local);
			continue;
		}

		if (request_state)
		{
			unsigned char* data = NULL;
			unsigned int len=0;
			TTS_STATUS state = TTS_STATUS::WAITTTSDATA;
			if (use_other_local)
				m_error=robot_voice_tts_ifly_ability::getInstanse()->GetVoiceData(data, len, state);
			else
				m_error = m_speechengine->GetVoiceData(data, len, state);

			if (len <= 2000)m_wait_fasttime = 1;
			if (len > 2000 && len<=5000)m_wait_fasttime = 3;

			if (m_error)
			{
				error_handle();
				callEndTTS(use_other_local);
				continue;
			}

			if (data != NULL&&state != TTS_STATUS::WAITTTSDATA)
			{
				int ret = 0;
				for (auto it = task.handler.begin(); it != task.handler.end(); it++)
				{
					ret = (*it)->push_data((unsigned char*)data, len);				
					if (ret == -1)
						break;
				}
				if (ret == -1)
				{
					end_request(request_state, use_other_local);
					continue;
				}
			}

			if (state == TTS_STATUS::ENDTTSDATA)
			{
				for (auto it = task.handler.begin(); it != task.handler.end(); it++)
					(*it)->done();
				end_request(request_state, use_other_local);
			}

		}
	}

	for (auto it = m_player.begin(); it != m_player.end(); it++)
	{
		it->second.discard();
		it->second.get_player()->quit();
	}

	return;
}

bool robot_voice_tts::check_stop(tts_message &task, bool &task_state)
{
	bool stop_state = false;
	for (auto it = task.handler.begin(); it != task.handler.end(); it++)
	{
		stop_state = (*it)->isStop();
		if (stop_state)
			break;
	}
	if (stop_state)
	{
		task_state = false;
		clean_tts();
		handle_remain_task();
	}		
	return stop_state;
}

void robot_voice_tts::netnotify(int state)
{
	LOGW("internet status %s", state ? "[connect]" : "disconnect");
	m_net_status = state;
	std::string out = m_jsonhelper.internet_status_notify(m_net_status);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length());
}

void robot_voice_tts::ntf_tts_error()
{
	std::string out = m_jsonhelper.tts_error(m_error);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length());
}

void robot_voice_tts::error_handle()
{
	ntf_tts_error();
	clean_tts();
	handle_remain_task();
}


void robot_voice_tts::callEndTTS(bool use_other_local)
{
	if (use_other_local)robot_voice_tts_ifly_ability::getInstanse()->EndTTS();
	else m_speechengine->EndTTS();
	m_wait_fasttime = 20;
}

void robot_voice_tts::end_request(bool &request_state, bool use_other_local)
{
	request_state = false;
	clean_tts();
	callEndTTS(use_other_local);
	handle_remain_task();
}
