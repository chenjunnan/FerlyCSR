#include "ifly/qisr.h"
#include "ifly/msp_errors.h"
#include "ifly/msp_types.h"
#include "ifly/msp_cmn.h"
#include "robot_voice_isr_ify.h"
#include "robot_voice_release.h"
#include "common_helper.h"
#include "audiohead.h"
#include "robot_voice_media.h"
#include "robot_PostMessage.h"
#include "robot_voice_struct.h"
#include <algorithm>
#include <iostream>


#define  DEFINE_HEAD_MUTE_TIMEOUT	10000
#define  DEFINE_HEAD_TAIL_TIMEOUT	700

robot_voice_isr_ify::robot_voice_isr_ify() :m_media(this)
{
	m_init = false;
	m_recoder = false;
	m_notify_status = 0;
	m_error=0;
	m_mode = 0;

	m_mic_no = 0;
	m_loop = false;

	m_ctrol = 0;

	int m_samplerate = DEFINE_SAMPLE_RATE;

	std::string temp;

	common_itoa_x(m_samplerate, &temp);
	isr_params_map["sample_rate"] = temp;



	if (!common_config_get_field_string("ifly", "isrengine_type", &temp))
		temp = "cloud";

	isr_params_map["engine_type"] = temp;

	if (temp.compare("local") != 0)
	{
 		isr_params_map["sub"] = "iat";
 
 		setSessionMap("language","zh_cn");

 		setSessionMap("accent", "mandarin");

	}

	if (temp.compare("cloud") != 0)
	{
		setSessionMap("asr_threshold", "0");

		setSessionMap("asr_denoise", "0");

		setSessionMap("asr_res_path", "");

		setSessionMap("grm_build_path", "");
	}

	if (temp.compare("mixed") == 0)
	{
		setSessionMap("mixed_type", "realtime");

		setSessionMap("mixed_timeout", "2000");

		setSessionMap("mixed_threshold", "60");
	}


	isr_params_map["result_type"] = "json";

	isr_params_map["result_encoding"] = "utf8";
	
	isr_params_map["ptt"] = "1";

	isr_params_map["sch"] = "0";

	isr_params_map["aue"] = "raw";//"speex-wb;7";


	common_itoa_x(DEFINE_HEAD_MUTE_TIMEOUT, &temp);
	isr_params_map["vad_bos"] = temp;

	
	if (!common_config_get_field_string("ifly", "endpoint", &temp))
		common_itoa_x(DEFINE_HEAD_TAIL_TIMEOUT, &temp);
	

	isr_params_map["vad_eos"] = temp;

	isr_params_map["vad_enable"] = "1";


	m_iflycom_online = false;
	m_iflycom_sleep = false;
	m_wait_sem = common_mutex_create_multisignal(0, 1);


	m_session_id = NULL;
	m_isrstate = ISRINIT;
}

void robot_voice_isr_ify::setSessionMap(std::string key, std::string defaultstring)
{
	std::string temp;
	if (!common_config_get_field_string("ifly", key.c_str(), &temp))
		temp = defaultstring;
	isr_params_map[key.c_str()] = temp;
}



robot_voice_isr_ify::~robot_voice_isr_ify()
{
	if (m_iflycom_online)
	{
		m_iflycom.quit();
		m_iflycom.close_com();
	}
	UnInit();
	common_mutex_multisignal_release(m_wait_sem);
}


void robot_voice_isr_ify::SetConfig(int mic_no)
{
	if (mic_no < 0 || mic_no>10)return ;

	m_mic_no = mic_no;
	char name_buffer[1024] = { 0 };
	sprintf_s(name_buffer, 1024, "SpeechConfig_%d", m_mic_no);
	m_config_session = name_buffer;


	int voice_need_debug = 0;
	if (!common_config_get_field_int(m_config_session.c_str(), "vdebug", &voice_need_debug))
		voice_need_debug = 0;

	m_recoder = voice_need_debug==0?false:true;

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

int robot_voice_isr_ify::build_grm_cb(int ecode, const char *info, void *udata)
{
	robot_voice_isr_ify *this_ptr = (robot_voice_isr_ify *)udata;

	GrammarData temp;
	if (NULL == this_ptr)return 1;
	this_ptr->m_error = ecode;
	if (MSP_SUCCESS == ecode && NULL != info) 
	{
		temp.build_fini = 1;
		temp.errcode = ecode;
		this_ptr->m_gramdatalist[info]=temp;
		LOGI("build grammar success！ grammar ID:%s\n", info);
	}
	else
	{
		LOGE("build grammar failed！%d\n", ecode);
	}


	common_mutex_multisignal_post(this_ptr->m_wait_sem);
	return 0;
}

int robot_voice_isr_ify::readfile(std::string filename, std::vector<char> &out)
{
	out.clear();
	if (filename.empty() || !common_check_file_exist(filename.c_str()))
	{
		LOGW("check file \"%s\" failed ", filename.c_str());
		return -1;
	}


	FILE *dir_context = NULL;

	dir_context = fopen(filename.c_str(), "rb");
	if (NULL == dir_context) {
		LOGW("open file \"%s\" failed！[%s] ", filename.c_str(), strerror(errno));
		return -1;
	}

	fseek(dir_context, 0, SEEK_END);
	int dir_lex_len = ftell(dir_context);
	fseek(dir_context, 0, SEEK_SET);

	out.resize(dir_lex_len+1);
	fread((void*)out.data(), 1, dir_lex_len, dir_context);
	out[dir_lex_len] = '\0';
	fclose(dir_context);
	dir_context = NULL;
	return 0;
}

int robot_voice_isr_ify::build_grammar()
{
	FILE *grm_file = NULL;
	char grm_build_params[MAX_PARAMS_LEN] = { NULL };
	int ret = 0;


	std::string temp;
	if (!common_config_get_field_string("ifly", "grm_file", &temp))
		return -1;

	if (temp.empty())return -1;

	std::vector<std::string > file_list = split(temp, '|');

	std::string exepath;
	common_get_current_dir(&exepath);

	for (int i = 0; i < file_list.size();i++)
	{
		std::string fullpath;
		fullpath = exepath;
		fullpath.append(file_list[i]);

		std::vector<char >  filedata;
		replace_char(fullpath, '/', '\\');
		if (readfile(fullpath, filedata))
			continue;

		_snprintf(grm_build_params, MAX_PARAMS_LEN - 1,
			"engine_type = %s, asr_res_path = %s, sample_rate = %s, grm_build_path = %s",
			isr_params_map["engine_type"].c_str(),
			isr_params_map["asr_res_path"].c_str(),isr_params_map["sample_rate"].c_str(),isr_params_map["grm_build_path"].c_str());

		ret = QISRBuildGrammar("bnf", filedata.data(), filedata.size(), grm_build_params, build_grm_cb, this);

		if (ret == MSP_SUCCESS)
			common_mutex_multisignal_wait(m_wait_sem);
		else
		{
			m_error = ret;
			LOGE("QISRBuildGrammar error %d", m_error);
			continue;
		}

	}

	if (m_gramdatalist.empty())
	{
		if (m_error == 0)return 1;
		return m_error;

	}

	return ret;
}



int robot_voice_isr_ify::update_lex_cb(int ecode, const char *info, void *udata)
{
	robot_voice_isr_ify *this_ptr = (robot_voice_isr_ify *)udata;

	if (NULL == this_ptr)return 1;
	this_ptr->m_error = ecode;
	if (NULL != this_ptr&&MSP_SUCCESS == ecode)
	{
		this_ptr->m_gramdatalist[info].update_fini = 1;
	}


	common_mutex_multisignal_post(this_ptr->m_wait_sem);
	return 0;
}


void robot_voice_isr_ify::trim_return(std::string &instr)
{
	std::string out;
	for (auto it = instr.begin(); it != instr.end();it++)
	{
		if (*it != '\n'&&*it != '\r')
			out.insert(out.end(),*it);
	}
	instr = out;
}

int robot_voice_isr_ify::update_lexicon_imp(std::string slotname,std::vector<char> &data,std::string grammar)
{
	char update_lex_params[MAX_PARAMS_LEN] = { NULL };

	_snprintf(update_lex_params, MAX_PARAMS_LEN - 1,
		"engine_type = %s, text_encoding = utf8, \
		asr_res_path = %s, sample_rate = %s, \
		grm_build_path = %s, grammar_list = %s, ",
		isr_params_map["engine_type"].c_str(),
		isr_params_map["asr_res_path"].c_str(),
		isr_params_map["sample_rate"].c_str(),
		isr_params_map["grm_build_path"].c_str(),
		grammar.c_str());
	data.erase(std::remove(data.begin(), data.end(), '\r'),data.end());

	int ret = QISRUpdateLexicon(slotname.c_str(), data.data(), data.size(), update_lex_params, update_lex_cb, this);

	if (ret == MSP_SUCCESS)
	{
		common_mutex_multisignal_wait(m_wait_sem);
		common_thread_sleep(50);
	}
	else
	{
		m_error = ret;
		LOGE("[%s] QISRUpdateLexicon error %d", slotname.c_str(), m_error);
		return m_error;
	}
	if (m_error == MSP_SUCCESS)
	{
		LOGI("updateLexicon [%s] success !!", slotname.c_str());
	}
	else
	{
		LOGE("updateLexicon [%s] error %d", slotname.c_str(),m_error);
	}
	return m_error;
}


void robot_voice_isr_ify::replace_char(std::string &data,const char src,const char des)
{
	for (int i = 0; i < data.size();i++)
	{
		if (data[i] == src)
			data[i] = des;
	}
}

int robot_voice_isr_ify::update_lexicon()
{
	FILE *lex_file = NULL;
	int ret = 0;


	std::string temp;
	if (!common_config_get_field_string("ifly", "grm_lexicon", &temp))
		return -1;
	if (temp.empty())
		return -1;

	std::vector<char >  filedata;


	std::string exepath;
	common_get_current_dir(&exepath);
	
	std::string fullpath;
	fullpath = exepath;
	fullpath.append(temp);
	replace_char(fullpath,'/','\\');

	if (readfile(fullpath, filedata))
		return -1;



	std::string text;
	text.assign(filedata.begin(), filedata.end());
	trim_return(text);

	std::vector<std::string > item_list = split(text, ';');
	

	for (int i = 0; i < item_list.size();i++)
	{
		std::vector<std::string > info = split(item_list[i].c_str(), '|');

		if (info.size()<3)continue;

		std::string slotname = info[0];
		std::string file = info[1];
		std::string grammera_id_list;

		for (int j = 0; j < info.size() - 2;j++)
		{
			if (m_gramdatalist.find(info[j + 2]) != m_gramdatalist.end())
			{
				if (!grammera_id_list.empty())
					grammera_id_list.append(";");
				grammera_id_list.append(info[j + 2]);
			}
		}
		if (!grammera_id_list.empty())
		{
			std::string temp;			
			temp = fullpath.substr(0, fullpath.find_last_of('\\')+1);
			temp.append(file);

			filedata.clear();
			if (readfile(temp, filedata))
				continue;
			update_lexicon_imp(slotname, filedata, grammera_id_list);
		}


	}

	return ret;
}



int robot_voice_isr_ify::Init()
{
	if (m_init) return 0;

	int			ret = MSP_SUCCESS;//sql   

	if (!common_config_get_field_string(m_config_session.c_str(), "streamname", &m_streamstring) || m_streamstring.empty())
	{
		LOGF("ID.%d stream name is null", m_mic_no);
		m_error = 1010;
		return m_error;
	}

	/* 用户登录 */
	ret = robot_global_singleton::Get()->Register_Init("ifly", this);
	if (MSP_SUCCESS != ret)	{
		LOGF("MSPLogin failed , Error code %d.\n", ret);
		m_error = ret;
		return m_error;
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

	robot_voice_release::getptr()->start();
	m_init = true;
	m_error = 0;
	return m_error;

rel:
	ret = robot_global_singleton::Get()->UnRegister_Uninit(this);
	if (MSP_SUCCESS != ret)
	{
		LOGF("MSPLogout failed, error code is: %d", ret);
		m_error = ret;
		return m_error;
	}
	return m_error;
}




void robot_voice_isr_ify::UnInit()
{
	if (!m_init)return;
	if (state())CancelISR();
	m_init = false;

	if (m_media.state())
		m_media.quit();

	close_audio(m_config_session,m_streamstring);

	if (robot_voice_release::getptr()->state())
		robot_voice_release::getptr()->quit();

	int ret = robot_global_singleton::Get()->UnRegister_Uninit(this);
	if (MSP_SUCCESS != ret)
	{
		LOGF("MSPLogout failed, error code is: %d", ret);
	}
}

void robot_voice_isr_ify::Recoder(bool b)
{
	m_recoder = b;
}


int robot_voice_isr_ify::StartISR(VoiceMessage &vm)
{
	if (!m_init)
	{
		m_error = 9999;
		return m_error;
	}
	if (state()) 
		return 0;

	if (isr_params_map["engine_type"].compare("cloud") != 0)
	{
		if (build_grammar() != MSP_SUCCESS)
			return 9991;
		update_lexicon();
	}

	m_message = vm;
	m_loop = vm.in.loop;
	m_mode = vm.in.mode;
	m_resultTo = vm.in.request_result_type;

	if (m_mode == 0)
	{
		isr_params_map["vad_enable"] = "1";
		if (!common_config_get_field_string("ifly", "endpoint", &isr_params_map["vad_eos"]))
			common_itoa_x(DEFINE_HEAD_TAIL_TIMEOUT, &isr_params_map["vad_eos"]);
		common_itoa_x(DEFINE_HEAD_MUTE_TIMEOUT, &isr_params_map["vad_bos"]);
	}
	if (m_mode == 1)
	{
		isr_params_map["vad_enable"] = "0";
		common_itoa_x(DEFINE_HEAD_MUTE_TIMEOUT*DEFINE_HEAD_MUTE_TIMEOUT, &isr_params_map["vad_bos"]);
		common_itoa_x(DEFINE_HEAD_MUTE_TIMEOUT*DEFINE_HEAD_MUTE_TIMEOUT, &isr_params_map["vad_eos"]);
		m_loop = false;
	}
	if (isr_params_map["engine_type"].compare("cloud") != 0)
	{
		if (vm.in.text.empty())
		{
			std::string finistring;
			for each (auto var in m_gramdatalist)
			{
				if (!finistring.empty())
					finistring.append(";");
				finistring.append(var.first);
			}
			isr_params_map["local_grammar"] = finistring;
		}
		else
		{
			isr_params_map["local_grammar"] = vm.in.text;
		}
	}

	open_audio(m_config_session,m_streamstring);
	m_error = 0;
	return 2;
}

int robot_voice_isr_ify::StartISR_Rec(int error, VoiceMessage &vm)
{
	vm = m_message;
	if (error)return error;

	if (!m_init)
	{
		m_error = 9999;
		return m_error;
	}

	if (state()) 
		return 0;
	
	if (m_media.init(m_streamstring, m_name))
	{
		LOGE("Create media publish failed.\n");
		m_error = 1011;
		return m_error;
	}

	int samp = m_media.get_samplerate();
	std::string temp;
	common_itoa_x(samp, &temp);
	isr_params_map["sample_rate"] = temp;
	m_ctrol = 0;
	m_error = 0;
	clean_msgqueue();
	init_first_voicecache(m_config_session);
	return start()?0:1;
}

int robot_voice_isr_ify::GetError()
{
	return m_error;
}

void robot_voice_isr_ify::CancelISR()
{
	if (!m_init)return ;
	if (!state())return;
	if (m_mode == 0)	m_ctrol = 1;
	if (m_mode == 1)	m_ctrol = 2;
	quit();
}

bool robot_voice_isr_ify::EnhanceMic(int angle)
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

bool robot_voice_isr_ify::SleepMic()
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


bool robot_voice_isr_ify::GetInfo(speech_process_info &info)
{
	info.stream_name = m_streamstring;
	info.loop = m_loop;
	info.semname = m_resultTo;
	return true;
}



int robot_voice_isr_ify::setParam(int type, std::string key, std::string value)
{
	if (isr_params_map.find(key) != isr_params_map.end())
	{
		common_config_set_field("ifly", key.c_str(), value.c_str());
		common_config_update_persistence();
		isr_params_map[key] = value;
		return 0;
	}
	return 1;
	
}


int robot_voice_isr_ify::getParam(int type, std::string key, std::string &value)
{
	if (isr_params_map.find(key) != isr_params_map.end())
		value = isr_params_map[key];
	return 0;
}




int robot_voice_isr_ify::onInit()
{
                     
	//const char* login_params = "appid = 5990186f, work_dir = ."; // 登录参数，appid与msc库绑定,请勿随意改动
	std::string login_param_str = "appid = ";
	std::string appid;

	if (!common_config_get_field_string("ifly", "appid", &appid))
	{
		LOGF("ifly appid null");
		m_error = 1012;
		return m_error;
	}
	std::string proxyip, proxyport;
	common_config_get_field_string("ifly", "proxy_ip", &proxyip);
	common_config_get_field_string("ifly", "proxy_port", &proxyport);

	Json::CharReaderBuilder crbuilder;
	login_param_str.append(appid);

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

int robot_voice_isr_ify::onUninit()
{
	return MSPLogout();
}

bool robot_voice_isr_ify::apply_iFly_session()
{
	close_iFly_session();
	const char*		session_id = NULL;
	int				errcode = MSP_SUCCESS;
	std::string    session_begin_params;
	////

	/////

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

	LOGD("ISR session_begin_params %s", session_begin_params.c_str());

	session_id = QISRSessionBegin(NULL, session_begin_params.c_str(), &errcode);
	if (MSP_SUCCESS != errcode || session_id == NULL)
	{
		m_error = errcode;
		LOGE("QISRSessionBegin failed! error code:%d\n", errcode);
		return false;
	}
	LOGD("ISR sessionID %s", session_id);
	
	m_session_id = session_id;

	m_endpointer_state = MSP_EP_LOOKING_FOR_SPEECH;
	m_rec_result_state = MSP_REC_STATUS_SUCCESS;
	m_push_audio_status = MSP_AUDIO_SAMPLE_FIRST;


	return true;
}



void robot_voice_isr_ify::close_iFly_session()
{
	if (m_session_id) {
		LOGD("end ifly session : %s", m_session_id);
		QISRSessionEnd(m_session_id, "normal");

	}
	m_session_id = NULL;
	return;
}

bool robot_voice_isr_ify::handler_isr_result_one(std::string iflydata, std::string &result, std::vector<slot_id> &slot)
{
	Json::Value   json_object;
	Json::Reader  json_reader(Json::Features::strictMode());
	bool has = false;
	if (iflydata.size() < 3)
		return false;
	if (iflydata.at(0) != '{')
		return false;
	try
	{
		bool b_json_read = json_reader.parse(iflydata.c_str(), json_object);
		if (b_json_read)
		{
			if (json_object.isMember("ws") && json_object["ws"].isArray())
			{
				Json::Value   voice_array = json_object["ws"];
				for (unsigned int i = 0; i < voice_array.size(); i++)
				{
					if (voice_array[i].isObject())
					{
						Json::Value   voice_value = voice_array[i];
						slot_id		if_solt;
						if (voice_value.isMember("slot") && voice_value["slot"].isString())
						{
							if_solt.indata = true;
							if_solt.slot_name = voice_value["slot"].asString();
						}
						if (voice_value.isMember("cw") && voice_value["cw"].isArray())
						{
							Json::Value voice_value_cw = voice_value["cw"];
							for (unsigned int j = 0; j < voice_value_cw.size(); j++)
							{
								if (voice_value_cw[j].isObject())
								{
									Json::Value voice_value_cw_one = voice_value_cw[j];
									std::string solt_con;
									if (voice_value_cw_one.isMember("w") && voice_value_cw_one["w"].isString())
									{
										solt_con = voice_value_cw_one["w"].asString();
										if (!solt_con.empty())
										{
											result.append(solt_con);
											has = true;
										}

									}
									if (if_solt.indata&&voice_value_cw_one.isMember("id") && voice_value_cw_one["id"].isInt())
									{
										slot_context c;
										c.id = voice_value_cw_one["id"].asInt();
										if (voice_value_cw_one.isMember("sc") && voice_value_cw_one["sc"].isInt())
											c.sc = voice_value_cw_one["sc"].asInt();
										c.context = solt_con;
										if_solt.slot_context.push_back(c);
									}
								}
							}
						}
						if (if_solt.indata)
							slot.push_back(if_solt);
					}
				}
			}
		}
		else
		{
			LOGE("ifly json format error:%s", iflydata.c_str());
		}
	}
	catch (const Json::Exception& e)
	{
		LOGE("ifly json data error %s", e.what());
	}
	catch (const std::exception& e) {
		LOGE("ifly json data std error %s", e.what());
	}
	catch (...)
	{
		LOGE("ifly json data error unkonw");
	}

	return has;
}

bool robot_voice_isr_ify::handler_isr_result(std::string &result, std::vector<slot_id> &slot)
{
	slot.clear();
	result.clear();

	for (unsigned int i = 0; i < result_list.size(); i++)
		handler_isr_result_one(result_list[i], result, slot);
	return !result.empty();
}

int robot_voice_isr_ify::msg_handler()
{
	bool			  mc_falg = false;

	Timer_helper now;
	now.update();
	int ret = MSP_SUCCESS;

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
				//printf("use %d\n", common_get_longcur_time_stampms() - t_mete.GetMediaTime());
				if (recoder)
					loopwrite_to_file((char*)t_mete.GetMedia(), t_mete.GetMediaLen());

				ret = QISRAudioWrite(m_session_id, (char*)t_mete.GetMedia(), t_mete.GetMediaLen(), m_push_audio_status, &m_endpointer_state, &m_rec_result_state);

				now.update();
				std::string strtime;
				common_get_cur_time_string(&strtime);
				printf("%s\tstate :%d\tendpointer_state: %d\tresult_state :%d\n", strtime.c_str(), ret, m_endpointer_state, m_rec_result_state);

				if (ret != MSP_SUCCESS)
				{
					LOGE("QISRAudioWrite  failed: %d", ret);
					m_error = ret;
					return 1;
				}
				Mediadone();
				m_error = ret;
				m_push_audio_status = MSP_AUDIO_SAMPLE_CONTINUE;

				const char *rslt = NULL;

				switch (m_rec_result_state)
				{
				case	MSP_REC_STATUS_SUCCESS:
				{
					rslt = QISRGetResult(m_session_id, &m_rec_result_state, 0, &ret);
					if (MSP_SUCCESS != ret || !rslt)	{
						m_error = ret;
						LOGW("QISRGetResult failed! error code: %d，0x%x", ret, rslt);
					}
					else
					{
						if (handler_isr_result_one(rslt, m_result, m_vecsolt))
						{
							std::string out = m_jsonhelper.irs_result_section_ntf(m_result);
							robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length());
						}

						result_list.push_back(rslt);
					}

					break;
				}
				case	MSP_REC_STATUS_NO_MATCH:
				case	MSP_REC_STATUS_INCOMPLETE:
				case	MSP_REC_STATUS_COMPLETE:
					break;
				default:
					LOGE("QISRAudioWrite  unknow error rec_result_state state code: %d", m_rec_result_state);
				}

				switch (m_endpointer_state)
				{
				case		MSP_EP_LOOKING_FOR_SPEECH:
					break;
				case		MSP_EP_IN_SPEECH:
					break;
				case		MSP_EP_AFTER_SPEECH:
					return get_tail_result();

				case		MSP_EP_TIMEOUT:
				case		MSP_EP_ERROR:
				case		MSP_EP_MAX_SPEECH:
				case		MSP_EP_IDLE:
					LOGE("QISRGetResult error endpointer state ! error  endpointer code: %d", m_endpointer_state);
					return 1;
				default:
					LOGE("QISRGetResult  unknow error endpointer state code: %d", m_endpointer_state);
				}

			}
			else
			{
				Mediadone();
				now.update();
			}
		}

	
		if (now.isreach(2000))
		{
			m_error = 200111;
			return result_list.size() == 0 ? 1 : 0;
		}

		if (m_ctrol == 1)return -1;
		if (m_ctrol == 2)
		{
			ret = QISRAudioWrite(m_session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &m_endpointer_state, &m_rec_result_state);
			if (MSP_SUCCESS != ret)
			{
				LOGE("QISRAudioWrite failed! error code:%d \n", ret);
				m_error = ret;
				return 1;
			}
			return get_tail_result();
		}
	}
}


int robot_voice_isr_ify::get_tail_result()
{
	const char *rslt = NULL;
	int errcode;
	int count_result = 0;
	while (m_rec_result_state != MSP_REC_STATUS_COMPLETE){
		rslt = QISRGetResult(m_session_id, &m_rec_result_state, 0, &errcode);
		if (MSP_SUCCESS == errcode && rslt)
		{
			handler_isr_result_one(rslt, m_result, m_vecsolt);
			result_list.push_back(rslt);
		}
		if (m_ctrol == 1)return -1;
		count_result++;
		m_error = errcode;
		printf("m_rec_result_state:%d\n", m_rec_result_state);
		if (count_result > 100)
		{
			if (m_error == 0)m_error = 9000;//network error
			return result_list.size() == 0 ? 1 : 0;
		}
		if (m_notify_status == 1 && count_result == 1 && m_rec_result_state != MSP_REC_STATUS_COMPLETE)
		{
			std::string out = m_jsonhelper.irs_result_readly_wait_ntf();
			robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length());
			m_notify_status = 2;
		}
		Sleep(30); /* for cpu occupy, should sleep here */
	}
	return 0;
}

int robot_voice_isr_ify::isr_restart(int &error_count)
{
	if (m_isrstate != ISRRUNING)
	{
		if (!apply_iFly_session() && error_count < 50)
		{
			error_count++;
			return 2;
		}
		if (error_count >= 50)
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



void robot_voice_isr_ify::run()
{
	bool loop = true;

	int ret = 0;
	int error_count = 0;
	LOGI("ISR %d Thread start", m_mic_no);
	int errcode=0;
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
			loop = false;
			continue;
		}
		if (ret == 2)
			continue;

 		if (!ret)
 		{
 			if (m_notify_status == 0 || m_notify_status==2)
 			{
 				out = m_jsonhelper.irs_result_readly_start_ntf(0);
 				robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length());
 				m_notify_status = 1;
 			}
 
 
 			if (recoder_state != m_recoder)
 			{
 				store_pcm();
 				recoder_state = m_recoder;
 			}
 
 			m_result.clear();
 			m_vecsolt.clear();
 
 			ret = msg_handler();
 
 			m_isrstate = ISRDONE;
 
 			if (ret == 0 && result_list.size() != 0)
 			{
 				if (m_result.size() != 0)
 				{	
 					result_notify();
 					if (!m_loop)
 						loop = false;
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

void robot_voice_isr_ify::result_notify()
{
	std::string out;
	char *modulename = NULL;
	if (!m_resultTo.empty())
		modulename = "semantics_module";

	if (isr_params_map["engine_type"].compare("cloud") != 0 && m_vecsolt.size() != 0)
	{
		if (!m_resultTo.empty())
			out = m_jsonhelper.irs_result_semantics_ntf(m_result, m_vecsolt, m_resultTo);
		else
			out = m_jsonhelper.irs_result_semantics_ntf(m_result, m_vecsolt);
	}
	else
	{
		if (!m_resultTo.empty())
			out = m_jsonhelper.irs_result_semantics_ntf(m_result, m_resultTo);
		else
			out = m_jsonhelper.irs_result_semantics_ntf(m_result);
	}
	robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length(), modulename);
	m_notify_status = 0;
	std::string data;
	common_utf8_2gbk(&m_result, &data);
	LOGI("[robot speech] listen :%s", data.c_str());
}

void robot_voice_isr_ify::store_pcm()
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


std::vector<std::string> robot_voice_isr_ify::split(std::string strtem, char a)
{

	std::vector<std::string> strvec;
	std::string::size_type pos1, pos2;

	pos2 = strtem.find(a);

	pos1 = 0;

	while (std::string::npos != pos2)
	{
		strvec.push_back(strtem.substr(pos1, pos2 - pos1));
		pos1 = pos2 + 1;
		pos2 = strtem.find(a, pos1);
	}

	strvec.push_back(strtem.substr(pos1));
	return strvec;
}

