#include "robot_voice_isr_ifly_ccb95533.h"
#ifdef CCB_OFFLINEIFLY // CCB_OFFLINEIFLY
#include "robot_voice_release.h"
#include "robot_PostMessage.h"
#include "ifly/ccb/95533ifly/mt_scylla.h"
#include "ifly/ccb/95533ifly/scylla_errors.h"
#include "ifly/ccb/95533ifly/scylla_types.h"
#define  DEFINE_HEAD_MUTE_TIMEOUT	10000
#define  DEFINE_HEAD_TAIL_TIMEOUT	700

Proc_SCYMTInitializeEx SCYMTInitializeEx_=NULL;
Proc_SCYMTUninitializeEx SCYMTUninitializeEx_ = NULL;
Proc_SCYMTSessionBeginEx SCYMTSessionBeginEx_ = NULL;
Proc_SCYMTSessionEndEx SCYMTSessionEndEx_ = NULL;
Proc_SCYMTAudioWriteEx SCYMTAudioWriteEx_ = NULL;
Proc_SCYMTGetResultEx SCYMTGetResultEx_ = NULL;

static std::string mt_scylla = "mt_scylla_95533.dll";

robot_voice_isr_ifly_ccb95533::robot_voice_isr_ifly_ccb95533() :m_media(this)
{
	m_init = false;
	m_recoder = false;

	m_error = 0;

	m_mic_no = 0;
	m_loop = false;

	m_ctrol = 0;

	m_samplerate = DEFINE_SAMPLE_RATE;

	m_error = 0;


	m_iflycom_online = false;
	m_iflycom_sleep = false;

	m_session_id = NULL;
	m_isrstate = ISRINIT;
	m_hand = NULL;
}


robot_voice_isr_ifly_ccb95533::~robot_voice_isr_ifly_ccb95533()
{
	if (m_iflycom_online)
	{
		m_iflycom.quit();
		m_iflycom.close_com();
	}
	UnInit();
	FreeLibrary(m_hand);
	m_hand = NULL;
}

int robot_voice_isr_ifly_ccb95533::load_dll()
{
	if (m_hand)return 0;
	m_hand = LoadLibrary(mt_scylla.c_str());
	if (m_hand == NULL)
	{
		m_error =GetLastError();
		LOGE("load %s failed, error code is %d.\n", mt_scylla.c_str(), m_error);
		return -1;
	}
	SCYMTInitializeEx_ = (Proc_SCYMTInitializeEx)GetProcAddress(m_hand, "SCYMTInitializeEx");
	SCYMTUninitializeEx_ = (Proc_SCYMTUninitializeEx)GetProcAddress(m_hand, "SCYMTUninitializeEx");
	SCYMTSessionBeginEx_ = (Proc_SCYMTSessionBeginEx)GetProcAddress(m_hand, "SCYMTSessionBeginEx");
	SCYMTSessionEndEx_ = (Proc_SCYMTSessionEndEx)GetProcAddress(m_hand, "SCYMTSessionEndEx");
	SCYMTAudioWriteEx_ = (Proc_SCYMTAudioWriteEx)GetProcAddress(m_hand, "SCYMTAudioWriteEx");
	SCYMTGetResultEx_ = (Proc_SCYMTGetResultEx)GetProcAddress(m_hand, "SCYMTGetResultEx");
	return 0;
}



void robot_voice_isr_ifly_ccb95533::SetConfig(int mic_no)
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

void robot_voice_isr_ifly_ccb95533::setSessionMap(std::string key, std::string defaultstring)
{
	std::string temp;
	if (!common_config_get_field_string("iflyccb95533", key.c_str(), &temp))
		temp = defaultstring;
	isr_params_map[key.c_str()] = temp;
}


int robot_voice_isr_ifly_ccb95533::Init()
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
	if (!common_config_get_field_string("iflyccb95533", "appid", &appid))
	{
		LOGF("ifly appid null");
		m_error = 1012;
		return m_error;
	}
	isr_params_map["appid"] = appid;
	if (!common_config_get_field_string("iflyccb95533", "url", &server_addr))
	{
		LOGF("ifly url null");
		m_error = 1013;
		return m_error;
	}



	isr_params_map["url"] = server_addr;


	setSessionMap("svc","iat");
	setSessionMap("type", "1");
	setSessionMap("uid", "660Y5r");
	isr_params_map["auf"] = "audio/L16;rate=16000";
	isr_params_map["aue"] = "raw";//"speex-wb;7";
	setSessionMap("extend_params", "");
	setSessionMap("hotword", "");

	if (isr_params_map["extend_params"].empty())isr_params_map.erase("extend_params");
	if (isr_params_map["hotword"].empty())isr_params_map.erase("hotword");

	if (load_dll())
		return m_error;

	/* 用户登录 */
	ret = robot_global_singleton::Get()->Register_Init("iflyccb95533", this);
	if (SCY_SUCCESS != ret)
	{
		LOGF("init SCYMTInitializeEx failed %d.", ret);
		m_error = ret;
		return ret;
	}

	robot_voice_release::getptr()->start();
	m_init = true;
	m_error = 0;
	return m_error;
}

void robot_voice_isr_ifly_ccb95533::UnInit()
{
	if (!m_init)return;
	if (state())CancelISR();
	m_init = false;

	if (m_media.state())
		m_media.quit();
	close_audio(m_config_session,m_streamstring);
	robot_voice_release::getptr()->quit();


	int ret = robot_global_singleton::Get()->UnRegister_Uninit(this);
	if (SCY_SUCCESS != ret)
	{
		LOGF("SCYMTUninitializeEx failed, error code is: %d", ret);
		return;
	}

}

void robot_voice_isr_ifly_ccb95533::Recoder(bool b)
{
	m_recoder = b;
}

int robot_voice_isr_ifly_ccb95533::StartISR( VoiceMessage &vm)
{
	if (!m_init)
	{
		m_error = 9999;
		return m_error;
	}
	if (state()) return 0;

	m_message = vm;
	m_loop = vm.in.loop;

	open_audio(m_config_session, m_streamstring);
	m_error = 0;
	return 2;
}

int robot_voice_isr_ifly_ccb95533::StartISR_Rec(int error, VoiceMessage &vm)
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


int robot_voice_isr_ifly_ccb95533::GetError()
{
	return m_error;
}

void robot_voice_isr_ifly_ccb95533::CancelISR()
{
	if (!m_init)return;
	if (!state())return;
	m_ctrol = 1;
	quit();

}

bool robot_voice_isr_ifly_ccb95533::GetInfo(speech_process_info &info)
{
	info.stream_name = m_streamstring;
	info.loop = m_loop;
	info.semname.clear();
	return true;
}


int robot_voice_isr_ifly_ccb95533::setParam(int type, std::string paramName, std::string paramValue)
{
	std::string key = paramName;
	std::string value = paramValue;
	if (key.compare("appid") == 0 || key.compare("url") == 0)
	{
		int status = state();
		int initstatus = m_init;
		UnInit();
		common_config_set_field("iflyccb95533", key.c_str(), value.c_str());
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


int robot_voice_isr_ifly_ccb95533::getParam(int type, std::string key, std::string &value)
{
	if (isr_params_map.find(key) != isr_params_map.end())
		value = isr_params_map[key];
	return 0;
}


int robot_voice_isr_ifly_ccb95533::onInit()
{
	return SCYMTInitializeEx_(NULL);
}

int robot_voice_isr_ifly_ccb95533::onUninit()
{
	return SCYMTUninitializeEx_(NULL);
}

bool robot_voice_isr_ifly_ccb95533::apply_iFly_session()
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


	session_id = SCYMTSessionBeginEx_(session_begin_params.c_str(), &errcode, NULL);
	if (SCY_SUCCESS != errcode || session_id == NULL)
	{
		m_error = errcode;
		printf("SCYMTSessionBeginEx failed! error code:%d\n", errcode);
		//LOGE("SCYMTSessionBeginEx failed! error code:%d\n", errcode);
		return false;
	}
	//LOGD("ISR sessionID %s", session_id);

	m_session_id = session_id;
	m_endpointer_state = 0;
	m_rec_result_state = SCY_REC_STATUS_FAILURE;
	m_push_audio_status = SCY_AUDIO_SAMPLE_FIRST;

	return true;
}



void robot_voice_isr_ifly_ccb95533::close_iFly_session()
{
	if (m_session_id) {
		robot_voice_release::getptr()->pushq(92, m_session_id, SCYMTSessionEndEx_);
	}
	m_session_id = NULL;
	return;
}


std::string	 robot_voice_isr_ifly_ccb95533::handler_isr_result()
{
	std::string	   irs_result, gbkstring;

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
	return irs_result;
}


int robot_voice_isr_ifly_ccb95533::msg_handler()
{
	bool			  mc_falg = false;

	Timer_helper now;

	int ret = SCY_SUCCESS;
	robot_voice_media t_mete;
	int recoder = m_recoder;
	while (1)
	{
		Message xMSG = getq(0);
		if (getCacheMedia(t_mete, 1000) )
		{
			if ((common_get_longcur_time_stampms() - t_mete.GetMediaTime()) < (m_usermaxcahce))
			{
				m_usermaxcahce = 3000;
				if (recoder)
					loopwrite_to_file((char*)t_mete.GetMedia(), t_mete.GetMediaLen());


				m_rec_result_state = SCY_REC_STATUS_INCOMPLETE;
				const char *resultData = SCYMTAudioWriteEx_(m_session_id, (char*)t_mete.GetMedia(), t_mete.GetMediaLen(),
					m_push_audio_status, &m_endpointer_state, &m_rec_result_state, &ret, NULL);

				now.update();
				std::string strtime;
				common_get_cur_time_string(&strtime);
				printf("%s\tstate :%d\tendpointer_state: %d\tresult_state :%d\n", strtime.c_str(), ret, m_endpointer_state, m_rec_result_state);

				if (ret != SCY_SUCCESS)			{
					LOGE("QISRAudioWriteEx  failed: %d", ret);
					m_error = ret;
					return 1;
				}
				Mediadone();
				m_error = 0;
				m_push_audio_status = SCY_AUDIO_SAMPLE_CONTINUE;

				const char *rslt = NULL;

				if (resultData != NULL&&strlen(resultData) > 0)
				{
					result_list.push_back(resultData);
					//LOGD("result is %s.", resultData);
				}

				switch (m_rec_result_state)
				{
				case	SCY_REC_STATUS_SUCCESS:
				{
					int errorCode = 0;
					Timer_helper waittime;
					while (m_rec_result_state != SCY_REC_STATUS_COMPLETE && SCY_SUCCESS == ret)
					{
						const char *rslt = SCYMTGetResultEx_(m_session_id, &m_rec_result_state, 10, &ret, NULL);//获取结果
						if (NULL != rslt)
						{
							result_list.push_back(rslt);
							//LOGD("have end data is %s.", rslt);
						}
						if (waittime.isreach(500))
							break;
					}
					return result_list.size() == 0 ? 1 : 0;
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
				case	ISR_EP_LOOKING_FOR_SPEECH: 
					break;
				case		ISR_EP_IN_SPEECH:  
					break;
				case		ISR_EP_BEFORE_SPEECH:  
					break;
				case	ISR_EP_AFTER_SPEECH:  
					return result_list.size() == 0 ? 1 : 0;
					break;
				case		ISR_EP_TIMEOUT: //检测端点超时
					break;
				case		ISR_EP_ERROR:
					return 1;
				case		ISR_EP_MAX_SPEECH:
					return 1;
				case		ISR_EP_IDLE:  // internal state after stop and before start
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

bool robot_voice_isr_ifly_ccb95533::EnhanceMic(int angle)
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

bool robot_voice_isr_ifly_ccb95533::SleepMic()
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


int robot_voice_isr_ifly_ccb95533::isr_restart(int &error_count)
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

		if (m_iflycom_online&&!m_iflycom_sleep)
		{
			if (!m_iflycom.Beam())
				LOGE("ifly com config angel error!!");
		}
		result_list.clear();
	}
	m_isrstate = ISRRUNING;

	return 0;
}


void robot_voice_isr_ifly_ccb95533::run()
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
			LOGE("SCYMTSessionBeginEx failed! error code:%d\n", GetError());
			if (!m_loop)
				loop = false;///loop ccb永不退出
			else
				common_thread_sleep(5);
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

void robot_voice_isr_ifly_ccb95533::store_pcm()
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
#endif // CCB_OFFLINEIFLY