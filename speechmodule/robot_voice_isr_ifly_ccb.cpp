#include "robot_voice_isr_ifly_ccb.h"
#ifdef CCB_OFFLINEIFLY // CCB_OFFLINEIFLY

#include "robot_voice_release.h"
#include "robot_PostMessage.h"
#include "ifly\ccb\scylla_errors.h"
#include "ifly\ccb\mt_scylla.h"




#define  DEFINE_HEAD_MUTE_TIMEOUT	10000
#define  DEFINE_HEAD_TAIL_TIMEOUT	700


robot_voice_isr_ifly_ccb::robot_voice_isr_ifly_ccb() :m_media(this)
{
	m_init = false;
	m_recoder = false;
	m_session_appect = new robot_fuck_hemei(this);
	m_error = 0;

	m_mic_no = 0;
	m_loop = false;

	m_ctrol = 0;

	m_samplerate = DEFINE_SAMPLE_RATE;

	isr_params_map["svc"] = "iat";
	isr_params_map["type"] = "1";
	isr_params_map["auf"] = "audio/L16;rate=16000";
	isr_params_map["aue"] = "raw";//"speex-wb;7";


	std::string temp;
	if (!common_config_get_field_string("iflyccb", "uid", &temp))
		temp = "660Y5r";

	isr_params_map["uid"] = temp;
	m_error = 0;
	

	m_iflycom_online = false;
	m_iflycom_sleep = false;

	m_session_id = NULL;
	m_isrstate = ISRINIT;

	m_session_count = 1;
	m_wait_mutex = CreateEvent(NULL, false, false, NULL);
	m_session_mutex = common_mutex_create_threadlock();
}


robot_voice_isr_ifly_ccb::~robot_voice_isr_ifly_ccb()
{
	if (m_iflycom_online)
	{
		m_iflycom.quit();
		m_iflycom.close_com();
	}
	UnInit();
	CloseHandle(m_wait_mutex);
	common_mutex_threadlock_release(m_session_mutex);
	SAFE_DELETE(m_session_appect);
}

void robot_voice_isr_ifly_ccb::SetConfig(int mic_no)
{
	if (mic_no < 0 || mic_no>10)return;

	m_mic_no = mic_no;
	char name_buffer[1024] = { 0 };
	sprintf_s(name_buffer, 1024, "SpeechConfig_%d", m_mic_no);
	m_config_session = name_buffer;


	int voice_need_debug = 0;
	if (!common_config_get_field_int(m_config_session.c_str(), "vdebug", &voice_need_debug))
		voice_need_debug = 0;




	m_recoder = voice_need_debug == 0 ? false : true;

	std::string temp;
	if (common_config_get_field_string(m_config_session.c_str(), "port", &temp))
	{
		m_iflycom_online = m_iflycom.open(temp.c_str());

		if (!m_iflycom_online)
		{
			LOGE("ifly com open error!!");
		}
	}

	m_name = name_buffer;
	m_name.append("reader");
}

int robot_voice_isr_ifly_ccb::Init()
{
	if (m_init) return 0;

	int			ret = SCY_SUCCESS;//sql                        
	std::string appid, server_addr;


	if (!common_config_get_field_string(m_config_session.c_str(), "streamname", &m_streamstring) || m_streamstring.empty())
	{
		LOGF("ID.%d stream name is null", m_mic_no);
		m_error = 1010;
		return m_error;
	}


	if (!common_config_get_field_int("iflyccb", "session_count", &m_session_count))
		m_session_count = 1;

	/* 用户登录 */
	ret = robot_global_singleton::Get()->Register_Init("iflyccb", this);
	if (SCY_SUCCESS != ret)
	{
		LOGF("init SCYMTInitialize failed %d.", ret);
		m_error = ret;
		return ret;
	}

	robot_voice_release::getptr()->start();
	m_init = true;
	m_error = 0;
	return m_error;
}

void robot_voice_isr_ifly_ccb::UnInit()
{
	if (!m_init)return;

	if (state())CancelISR();

	if (m_session_count != 1)
	{
		if (m_session_appect->state())
			m_session_appect->quit();
		for each (std::string var in m_session_list)
			robot_voice_release::getptr()->pushq(91, var);
		m_session_list.clear();
	}

	m_init = false;

	if (m_media.state())m_media.quit();


	close_audio(m_config_session,m_streamstring);
	
	robot_voice_release::getptr()->quit();

	int ret = robot_global_singleton::Get()->UnRegister_Uninit(this);
	if (SCY_SUCCESS != ret)
	{
		LOGF("SCYMTUninitialize failed, error code is: %d", ret);
		return;
	}

}

void robot_voice_isr_ifly_ccb::Recoder(bool b)
{
	m_recoder = b;
}

int robot_voice_isr_ifly_ccb::StartISR(VoiceMessage &vm)
{
	if (!m_init)
	{
		m_error = 9999;
		return m_error;
	}
	if (state()) return 0;

	m_message = vm;
	m_loop =  vm.in.loop;
	
	open_audio(m_config_session,m_streamstring);
	m_error = 0;
	return 2;
}

int robot_voice_isr_ifly_ccb::StartISR_Rec(int error, VoiceMessage &vm)
{
	vm = m_message;
	if (error)return error;

	if (!m_init)
	{
		m_error = 9999;
		return m_error;
	}

	if (state()) return 0;


	if (m_media.init(m_streamstring, m_name))
	{
		LOGE("Create media publish failed.\n");
		m_error = 1011;
		return m_error;
	}

	m_samplerate = m_media.get_samplerate();
	m_ctrol = 0;
	m_error = 0;
	clean_msgqueue();


	init_first_voicecache(m_config_session);



	return start() ? 0 : 1;
}


int robot_voice_isr_ifly_ccb::GetError()
{
	return m_error;
}

void robot_voice_isr_ifly_ccb::CancelISR()
{
	if (!m_init)return;
	if (!state())return;
	m_ctrol = 1;
	quit();
}

bool robot_voice_isr_ifly_ccb::GetInfo(speech_process_info &info)
{
	info.stream_name = m_streamstring;
	info.loop = m_loop;
	info.semname.clear();
	return true;
}



int robot_voice_isr_ifly_ccb::setParam(int type, std::string paramName, std::string paramValue)
{
	std::string key = paramName;
	std::string value = paramValue;
	if (key.compare("appid") == 0 || key.compare("url") == 0)
	{
		int status = state();
		int initstatus = m_init;
		UnInit();
		common_config_set_field("iflyccb", key.c_str(), value.c_str());
		common_config_update_persistence();
		if (initstatus)
		{
			int error = Init();
			if (error)return error;
		}
		if (status)
		{
			VoiceMessage empty;
			empty.in.loop = m_loop;
			StartISR(empty);
		}		
	}

	return 0;
}


int robot_voice_isr_ifly_ccb::getParam(int type, std::string key, std::string &value)
{
	if (isr_params_map.find(key) != isr_params_map.end())
		value = isr_params_map[key];
	return 0;
}


int robot_voice_isr_ifly_ccb::onInit()
{
	std::string appid, server_addr;

	if (!common_config_get_field_string("iflyccb", "appid", &appid))
	{
		LOGF("ifly appid null");
		m_error = 1012;
		return m_error;
	}

	if (!common_config_get_field_string("iflyccb", "url", &server_addr))
	{
		LOGF("ifly url null");
		m_error = 1013;
		return m_error;
	}

	return SCYMTInitialize(server_addr.c_str(), appid.c_str(), 30);
}

int robot_voice_isr_ifly_ccb::onUninit()
{
	return SCYMTUninitialize();
}

bool robot_voice_isr_ifly_ccb::apply_iFly_session()
{
	close_iFly_session();
	const char*		session_id = NULL;
	int				errcode = SCY_SUCCESS;

	std::string    session_begin_params;// = "auf=audio/L16;rate=16000,svc=iat,aue=raw,type=1,uid=660Y5r";

	auto it = isr_params_map.begin();
	for (; it != isr_params_map.end(); it++)
	{
		session_begin_params.append(it->first);
		session_begin_params.append("=");
		session_begin_params.append(it->second);
		if ((++it) != isr_params_map.end())
			session_begin_params.append(",");
		it--;

	}
	m_session_id = NULL;

	if (m_session_count == 1)
	{
		//LOGD("ISR session_begin_params %s", session_begin_params.c_str());
		session_id = SCYMTSessionBegin(session_begin_params.c_str(), &errcode);
		if (SCY_SUCCESS != errcode || session_id == NULL)
		{
			m_error = errcode;
			printf("SCYMTSessionBegin failed! error code:%d\n", errcode);
			//LOGE("SCYMTSessionBegin failed! error code:%d\n", errcode);
			return false;
		}
		//LOGD("ISR sessionID %s", session_id);
		
	}
	else
	{
		if (!m_session_appect->state())
			m_session_appect->setappect(session_begin_params, m_session_count);
		m_session_str.clear();
		common_mutex_threadlock_lock(m_session_mutex);
		if (m_session_list.size() > 0)
		{
			m_session_str = m_session_list.front();
			m_session_list.pop_front();
			session_id = m_session_str.c_str();
		}
		else
		{
			common_mutex_threadlock_unlock(m_session_mutex);
			if (WaitForSingleObject(m_wait_mutex, 100) != WAIT_OBJECT_0)
			{

				return false;
			}
			else
			{
				common_mutex_threadlock_lock(m_session_mutex);
				if (m_session_list.size() > 0)
				{
					m_session_str = m_session_list.front();
					m_session_list.pop_front();
					session_id = m_session_str.c_str();
				}
				else
				{
					common_mutex_threadlock_unlock(m_session_mutex);
					return false;

				}
			}
		}
		common_mutex_threadlock_unlock(m_session_mutex);
	}
	
	m_session_id = session_id;
	m_endpointer_state = 0;
	m_rec_result_state = SCY_REC_STATUS_FAILURE;
	m_push_audio_status = SCY_AUDIO_SAMPLE_FIRST;

	return true;
}



void robot_voice_isr_ifly_ccb::close_iFly_session()
{
	if (m_session_id) {
		robot_voice_release::getptr()->pushq(91, m_session_id);

	}
	m_session_id = NULL;
	return;
}


std::string	 robot_voice_isr_ifly_ccb::handler_isr_result()
{
	std::string	   irs_result,utfstring;

	for (unsigned int i = 0; i < result_list.size(); i++)
	{
		Json::Value   json_object;
		Json::Reader  json_reader;
		bool b_json_read = json_reader.parse(result_list[i].c_str(), json_object);
		if (b_json_read)
		{
			if (json_object.isMember("result") && json_object["result"].isString())
			{
				irs_result.append(json_object["result"].asString());
			}

		}
		else
		{
			LOGE("ifly json format error:%s", result_list[i].c_str());
		}
	}
	//LOGD("irs result =%s", utf8_2gbk(irs_result).c_str());
	common_gbk_2utf8(&irs_result, &utfstring);
	return utfstring;
}


int robot_voice_isr_ifly_ccb::msg_handler()
{
	bool			  mc_falg = false;

	Timer_helper now;

	int ret = SCY_SUCCESS;

	robot_voice_media t_mete;
	int recoder = m_recoder;
	while (1)
	{
		Message xMSG = getq(0);
		if (getCacheMedia(t_mete, 1000))
		{
			if ((common_get_longcur_time_stampms() - t_mete.GetMediaTime()) < (m_usermaxcahce))
			{
				m_usermaxcahce = 3000;
				if (recoder)
					loopwrite_to_file((char*)t_mete.GetMedia(), t_mete.GetMediaLen());

				char resultData[6400] = { 0 };
				unsigned int resultLen = 0;
				int             pgStatus = 0;
				ret = SCYMTAudioWrite(m_session_id, (char*)t_mete.GetMedia(), t_mete.GetMediaLen(), m_push_audio_status, &m_endpointer_state, &m_rec_result_state, &pgStatus, resultData, &resultLen);

				now.update();
				std::string strtime;
				common_get_cur_time_string(&strtime);
				printf("%s\tstate :%d\tendpointer_state: %d\tresult_state :%d\n", strtime.c_str(), ret, m_endpointer_state, m_rec_result_state);

				if (ret != SCY_SUCCESS)			{
					LOGE("QISRAudioWrite  failed: %d", ret);

					m_error = ret;
					return 1;
				}
				Mediadone();
				m_error = 0;
				m_push_audio_status = SCY_AUDIO_SAMPLE_CONTINUE;

				const char *rslt = NULL;

				if (pgStatus == 1)
				{
					result_list.push_back(resultData);
				}

				switch (m_rec_result_state)
				{
				case	SCY_REC_STATUS_SUCCESS:
				{
					int errorCode = 0;
					return result_list.size() == 0 ? 1 : 0;
					//return 0;
					break;
				}
				case	SCY_REC_STATUS_NO_MATCH:
				case	SCY_REC_STATUS_INCOMPLETE:
				case	SCY_REC_STATUS_COMPLETE:
					break;
				default:
					LOGE("SCYMTAudioWrite  unknow error rec_result_state state code: %d", m_rec_result_state);
				}

				switch (m_endpointer_state)
				{
				case		SCY_EP_LOOKING_FOR_SPEECH:
					break;
				case		SCY_EP_IN_SPEECH:
					break;
				case		SCY_EP_AFTER_SPEECH:
					return result_list.size() == 0 ? 1 : 0;
					break;
				case		SCY_EP_TIMEOUT:

				case		SCY_EP_ERROR:
					return 1;
				case		SCY_EP_MAX_SPEECH:
					return 1;
				case		SCY_EP_IDLE:
					LOGE("SCYMTAudioWrite error endpointer state ! error  endpointer code: %d", m_endpointer_state);
					return 1;
				default:
					LOGE("SCYMTAudioWrite  unknow error endpointer state code: %d", m_endpointer_state);
				}
			}
			else
			{
				Mediadone();
				now.update();
			}

		}


		if (now.isreach(3000))
		{
			m_error = 20011;
			return result_list.size() == 0 ? 1 : 0;
		}
		if (m_ctrol == 1)return -1;
	}
}
bool robot_voice_isr_ifly_ccb::EnhanceMic(int angle)
{
	if (m_iflycom_online)
	{
		m_iflycom_sleep = false;
		m_iflycom.Beam(angle);
		return true;
	}
	else
		return false;
}

bool robot_voice_isr_ifly_ccb::SleepMic()
{
	if (m_iflycom_online)
	{
		m_iflycom_sleep = true;
		m_iflycom.Reset();
		return true;
	}
	else
		return false;
}

int robot_voice_isr_ifly_ccb::isr_restart(int &error_count)
{
	int error_totle = 1000;

	if (m_isrstate != ISRRUNING)
	{
		if (!apply_iFly_session() && error_count < error_totle)
		{
			error_count++;
			return 2;
		}
		if (error_count >= error_totle)
			return 1;


		if (m_iflycom_online)
		{
			if (m_iflycom_online&&!m_iflycom_sleep)
			{
				if (!m_iflycom.Beam())
				{
					LOGE("ifly com config angel error!!");
				}
			}
				
		}
		result_list.clear();
	}
	m_isrstate = ISRRUNING;

	return 0;
}


void robot_voice_isr_ifly_ccb::run()
{
	bool loop = true;

	int ret = 0;
	int error_count = 0;
	LOGI("ISR %d Thread start", m_mic_no);
	int errcode = 0;
	bool recoder_state = m_recoder;
	if (recoder_state)store_pcm();
	std::string out;
	while (loop)
	{
		if (m_ctrol)
			break;
		ret = isr_restart(error_count);
		if (ret == 1)
		{
			m_isrstate = ISRIDE;
			out = m_jsonhelper.voice_error(GetError());
			robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length());
			LOGE("SCYMTSessionBegin failed! error code:%d\n", GetError());
			if (!m_loop)
				loop = false;///loop ccb永不退出
			else
				;// common_thread_sleep(50);
			error_count = 0;
			continue;
		}
		if (ret == 2)
			continue;

		if (!ret)
		{
			if (recoder_state != m_recoder)
			{
				store_pcm();
				recoder_state = m_recoder;
			}


			ret = msg_handler();

			m_isrstate = ISRDONE;

			if (ret == 0 && result_list.size() != 0)
			{



				std::string temp;

				temp = handler_isr_result();

				if (temp.size() != 0)
				{
					out = m_jsonhelper.irs_result_semantics_ntf(temp);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length());
					if (!m_loop)
						loop = false;
					std::string tempout;
					common_utf8_2gbk(&temp, &tempout);
					LOGI("[robot speech] listen :%s", tempout.c_str());
				}
				error_count = 0;
			}
			
		}
		close_iFly_session();

		if (ret == -1)
		{
			LOGT("ISR Thread over call quit");
			loop = false;
		}

		if (ret == 1)
			error_count++;
	}

	LOGW("ISR %d Thread exit", m_mic_no);
	return;

}

void robot_voice_isr_ifly_ccb::store_pcm()
{
	if (m_recoder)
	{
		open_stored_file(m_name.c_str());
	}
	else
	{
		safe_close_file();
		//message
	}
}

void robot_fuck_hemei::run()
{
	{
		const char*		session_id = NULL;
		int				errcode = SCY_SUCCESS;
		int waittime = 100;
		while (1)
		{
			Message xMSG = getq(waittime);

			switch (xMSG.msgType)
			{
			case _EXIT_THREAD_:
				return;
			default:
				;
			}
			int nowcount = 0;
			common_mutex_threadlock_lock(m_cb->m_session_mutex);
			nowcount = m_cb->m_session_list.size();
			common_mutex_threadlock_unlock(m_cb->m_session_mutex);

			if (nowcount <(m_count-1))
			{
				waittime = 0;
				session_id = SCYMTSessionBegin(m_params.c_str(), &errcode);
				if (SCY_SUCCESS != errcode || session_id == NULL)
				{
					m_cb->m_error = errcode;
					printf("SCYMTSessionBegin failed! error code:%d\n", errcode);
					//LOGE("SCYMTSessionBegin failed! error code:%d\n", errcode);
				}
				else
				{
					common_mutex_threadlock_lock(m_cb->m_session_mutex);
					m_cb->m_session_list.push_back(session_id);
					if (m_cb->m_session_list.size() == (m_count - 1))waittime = 100;
					SetEvent(m_cb->m_wait_mutex);
					common_mutex_threadlock_unlock(m_cb->m_session_mutex);
				}
			}
			else
				waittime = 100;


		}



	}
}

#endif // CCB_OFFLINEIFLY

