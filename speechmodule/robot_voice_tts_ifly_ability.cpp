#include "robot_voice_tts_ifly_ability.h"
#include "ifly\qtts.h"
#include "ifly\msp_errors.h"
#include "ifly\msp_cmn.h"
#include "robot_global_singleton.h"
#include "robot_media_publish.h"


robot_voice_tts_ifly_ability::robot_voice_tts_ifly_ability()
{
	isInit = false;
	synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;
	m_session_id = NULL;
	m_thrGuard = common_mutex_create_threadlock();
}


robot_voice_tts_ifly_ability* robot_voice_tts_ifly_ability::getInstanse()
{
	static robot_voice_tts_ifly_ability m_ifly;
	return &m_ifly;
}

int robot_voice_tts_ifly_ability::InitEngine()
{
	LOCK_HELPER_AUTO_HANDLER(m_thrGuard);
	if (isInit)return 0;
	std::string temp;
	int nTemp = 0;
	default_type = "local";

	if (!common_config_get_field_string("ifly", "ttsengine_type", &temp))
		temp = "local";

	default_type = temp;

	config_local_params();

	config_cloud_params();

	m_session_id = NULL;


	int			ret = MSP_SUCCESS;//sql                        

	/* 用户登录 */
	ret = robot_global_singleton::Get()->Register_Init("ifly", this);

	if (MSP_SUCCESS != ret)	{
		LOGF("MSPLogin failed , Error code %d.\n", ret);
		return ret;
	}

	const char *versionPtr = MSPGetVersion("ver_msc", &ret);
	if (MSP_SUCCESS != ret)
	{
		LOGE("get iFly MSC version failed , Error code %d.\n", ret);
	}
	else
	{
		LOGI("iFly MSC version :%s", versionPtr);
	}


	versionPtr = MSPGetVersion("ver_tts", &ret);
	if (MSP_SUCCESS != ret)
	{
		LOGE("get iFly TTS version failed , Error code %d.\n", ret);
	}
	else
	{
		LOGI("iFly TTS version :%s", versionPtr);
	}
	isInit = true;
	return 0;
}

void robot_voice_tts_ifly_ability::UninitEngine()
{
	LOCK_HELPER_AUTO_HANDLER(m_thrGuard);
	if (!isInit)return;

	isInit = false;
	close_ifly_session();
	int ret = robot_global_singleton::Get()->UnRegister_Uninit(this);

	if (MSP_SUCCESS != ret)
	{
		LOGF("MSPLogout failed, error code is: %d", ret);
		return;
	}
}

void robot_voice_tts_ifly_ability::Cancel()
{

}

int robot_voice_tts_ifly_ability::RequestTTS(int mode, std::string text)
{
	LOCK_HELPER_AUTO_HANDLER(m_thrGuard);
	if (!isInit)return -50001;
	if (m_session_id)return -50002;

	int ret = 0;
	if (default_type.compare("cloud") == 0 && mode == 0)
		ret = apply_iFly_session(tts_cloudparams_map);
	else
		ret = apply_iFly_session(tts_localparams_map);

	if (MSP_SUCCESS != ret || m_session_id == NULL)
	{
		close_ifly_session();
		return ret;
	}


	ret = QTTSTextPut(m_session_id, text.c_str(), (unsigned int)text.size(), NULL);
	if (MSP_SUCCESS != ret)
	{
		LOGE("QTTSTextPut failed, error code: %d.\n", ret);
		close_ifly_session();
		return ret;
	}
	std::string out;
	common_utf8_2gbk(&text, &out);
	synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;
	LOGI("[robot speech] speak :%s", out.c_str());
	return ret;
}

void robot_voice_tts_ifly_ability::EndTTS()
{
	LOCK_HELPER_AUTO_HANDLER(m_thrGuard);
	close_ifly_session();
}

int robot_voice_tts_ifly_ability::GetVoiceData(unsigned char *& lpdata, unsigned int &len, TTS_STATUS &state)
{
	LOCK_HELPER_AUTO_HANDLER(m_thrGuard);
	if (!isInit)return -50001;
	int ret = 0;
	len = 0;
	state = TTS_STATUS::WAITTTSDATA;
	if (m_session_id == NULL) return -50001;

	/* 获取合成音频 */
	const void* data = QTTSAudioGet(m_session_id, &len, &synth_status, &ret);
	if (MSP_SUCCESS != ret)
	{
		LOGE("QTTSAudioGet failed, error code: %d.\n", ret);
		close_ifly_session();
		return ret;
	}
	
	if (NULL != data)
	{
		state = TTS_STATUS::HASTTSDATA;
		lpdata = (unsigned char*)data;
	}


	if (MSP_TTS_FLAG_DATA_END == synth_status)
		state = TTS_STATUS::ENDTTSDATA;
	return ret;
}

int robot_voice_tts_ifly_ability::SetParam(int type, std::string key, std::string value)
{
	LOCK_HELPER_AUTO_HANDLER(m_thrGuard);
	if (type == 0)
	{
		if (tts_localparams_map.find(key) != tts_localparams_map.end())
		{
			tts_localparams_map[key] = value;
			if (key.compare("person") == 0)
				key = "local_person";
			common_config_set_field("ifly", key.c_str(), value.c_str());
			common_config_update_persistence();
			return 0;
		}
	}
	else
	{
		if (tts_cloudparams_map.find(key) != tts_cloudparams_map.end())
		{
			tts_cloudparams_map[key] = value;
			common_config_set_field("ifly", key.c_str(), value.c_str());
			common_config_update_persistence();
			return 0;
		}
	}

	return 1;
}

int robot_voice_tts_ifly_ability::GetParam(int type, std::string key, std::string &value)
{
	LOCK_HELPER_AUTO_HANDLER(m_thrGuard);
	if (type == 0)
	{
		if (tts_localparams_map.find(key) != tts_localparams_map.end())
		{
			value = tts_localparams_map[key];
			return 0;
		}
	}
	else
	{
		if (tts_cloudparams_map.find(key) != tts_cloudparams_map.end())
		{
			value = tts_cloudparams_map[key];
			return 0;
		}
	}

	return 0;
}


int robot_voice_tts_ifly_ability::onInit()
{
	//const char* login_params = "appid = 5990186f, work_dir = ."; // 登录参数，appid与msc库绑定,请勿随意改动
	std::string login_param_str = "appid = ";
	std::string appid;

	if (!common_config_get_field_string("ifly", "appid", &appid))
	{
		LOGF("ifly appid null");
		return 1022;
	}
	login_param_str.append(appid);

	std::string proxyip, proxyport;
	common_config_get_field_string("ifly", "proxy_ip", &proxyip);
	common_config_get_field_string("ifly", "proxy_port", &proxyport);

	if (!proxyip.empty() && !proxyport.empty())
	{
		login_param_str.append(", net_type=custom ,proxy_ip=");
		login_param_str.append(proxyip);
		login_param_str.append(", proxy_port=");
		login_param_str.append(proxyport);
	}
	login_param_str.append(", work_dir = .");

	LOGI("ifly login param :%s", login_param_str.c_str());

	return  MSPLogin(NULL, NULL, login_param_str.c_str()); //第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数	;
}

int robot_voice_tts_ifly_ability::onUninit()
{
	return  MSPLogout();
}

robot_voice_tts_ifly_ability::~robot_voice_tts_ifly_ability()
{
	common_mutex_threadlock_release(m_thrGuard);
}



void robot_voice_tts_ifly_ability::config_local_params()
{
	std::string temp;
	tts_localparams_map["engine_type"] = "local";

	if (!common_config_get_field_string("ifly", "tts_res_path", &temp))
		temp = "fo|res\\tts\\xiaoyan.jet;fo|res\\tts\\common.jet";
	tts_localparams_map["tts_res_path"] = temp;


	if (!common_config_get_field_string("ifly", "rcn", &temp))
		temp = "1";
	tts_localparams_map["rcn"] = temp;


	if (!common_config_get_field_string("ifly", "effect", &temp))
		temp = "0";
	tts_localparams_map["effect"] = temp;


	if (!common_config_get_field_string("ifly", "speed_increase", &temp))
		temp = "0";
	tts_localparams_map["speed_increase"] = temp;

	if (!common_config_get_field_string("ifly", "local_person", &temp))
	{
		if (!common_config_get_field_string("ifly", "person", &temp))
			temp = "xiaoyan";
	}
	tts_localparams_map["voice_name"] = temp;


	common_params(tts_localparams_map);
}


int robot_voice_tts_ifly_ability::apply_iFly_session(std::map<std::string, std::string>& tts_params)
{
	const char*		session_id = NULL;
	int				errcode = 0;
	std::string    session_begin_params;

	auto it = tts_params.begin();
	for (; it != tts_params.end(); it++)
	{
		session_begin_params.append(it->first);
		session_begin_params.append("=");
		session_begin_params.append(it->second);
		if ((++it) != tts_params.end())
			session_begin_params.append(",");
		it--;

	}
	m_session_id = NULL;


	LOGD("TTS session_begin_params %s", session_begin_params.c_str());

	session_id = QTTSSessionBegin(session_begin_params.c_str(), &errcode);
	if (MSP_SUCCESS != errcode || session_id == NULL)
	{
		LOGE("QTTSSessionBegin failed, error code: %d.\n", errcode);
		return errcode;
	}
	LOGD("TTS sessionID %s", session_id);

	m_session_id = session_id;

	return 0;
}


void robot_voice_tts_ifly_ability::config_cloud_params()
{
	tts_cloudparams_map["engine_type"] = "cloud";

	std::string temp;
	if (!common_config_get_field_string("ifly", "background_sound", &temp))
		temp = "0";
	tts_cloudparams_map["background_sound"] = temp;

	if (!common_config_get_field_string("ifly", "person", &temp))
		temp = "babyxu";
	tts_cloudparams_map["voice_name"] = temp;


	common_params(tts_cloudparams_map);
}

void robot_voice_tts_ifly_ability::common_params(std::map<std::string, std::string> &map_in)
{
	std::string temp;

	if (!common_config_get_field_string("ifly", "speed", &temp))
		temp = "50";
	map_in["speed"] = temp;


	if (!common_config_get_field_string("ifly", "pitch", &temp))
		temp = "60";
	map_in["pitch"] = temp;

	if (!common_config_get_field_string("ifly", "rdn", &temp))
		temp = "3";
	map_in["rdn"] = temp;

	common_itoa_x(50, &map_in["volume"]);
	common_itoa_x(DEFINE_SAMPLE_RATE, &map_in["sample_rate"]);
	map_in["text_encoding"] = "utf8";
	map_in["aue"] = "raw";
	map_in["ttp"] = "text";
}

void robot_voice_tts_ifly_ability::close_ifly_session()
{
	if (m_session_id)
	{
		QTTSSessionEnd(m_session_id, "normal");
		m_session_id = NULL;
	}
}

