#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>   
#include <process.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <mmeapi.h>
#include "robot_voice_manager.h"
#include "robot_voice_struct.h"
#include "json\json.h"
#include "xCommon.h"
#include "robot_PostMessage.h"



#pragma comment(lib, "winmm.lib") 





robot_voice_manager * robot_voice_manager::getInstance()
{
	static robot_voice_manager _s;
	return &_s;
}

bool robot_voice_manager::init()
{
	int speech_cnt = 0;
	if (!common_config_get_field_int("Speech", "speechcount", &speech_cnt))
		speech_cnt = 0;

	if (!common_config_get_field_int("Speech", "mainspeech", &m_mainmic))
		m_mainmic = 0;

	while (speech_cnt)
	{
		speech_cnt--;
	
		robot_voice_process * ptr = new robot_voice_process;
		if (!ptr)
		{
			LOGE("create face process failed !!");
			SAFE_DELETE(ptr);
		}
		else
		{
			ptr->init(speech_cnt);
			m_speech_handler[speech_cnt]=ptr;
		}

	}

	return m_speech_handler.size() ? true : false;
}


void robot_voice_manager::get_statusfilepath(std::string &exepath)
{
	exepath.clear();
	common_get_current_dir(&exepath);

	exepath.append("status\\");
	if (!common_isDirectory(exepath.c_str()))
		common_create_dir(exepath.c_str());
	exepath.append("speechstatus");
}

void robot_voice_manager::check_status()
{
	std::string speech_status_filename;
	get_statusfilepath(speech_status_filename);
	if (!speech_status_filename.empty())
	{
		std::ifstream statusfile(speech_status_filename.c_str());
		if (!statusfile.is_open())
			return;

		Json::Value   json_object;
		Json::Reader  json_reader;
		std::string context;
		std::string temp;
		while (std::getline(statusfile, temp))
		{
			if (!temp.empty())
				context.append(temp);
		}
		bool b_json_read = json_reader.parse(context, json_object);
		if (b_json_read)
		{
			if (json_object.isMember("list") && json_object["list"].isArray())
			{
				Json::Value speech_list = json_object["list"];
				for (int i = 0; i < speech_list.size(); i++)
				{
					if (speech_list[i].isObject())
					{
						if (speech_list[i].isMember("id") && speech_list[i]["id"].isInt())
						{
							int sid = speech_list[i]["id"].asInt();
							bool startservice = false, startisr = false;
							if (speech_list[i].isMember("servicestatus") && speech_list[i]["servicestatus"].isBool())
								startservice = speech_list[i]["servicestatus"].asBool();
							if (speech_list[i].isMember("isrstatus") && speech_list[i]["isrstatus"].isBool())
								startisr = speech_list[i]["isrstatus"].asBool();

							if (startservice)
							{
								if (m_speech_handler.find(sid) != m_speech_handler.end())
								{
									m_speech_handler[sid]->start_voice_service();
									if (startisr)
									{
										VoiceMessage vm;
										vm.in.loop = true;
										m_speech_handler[sid]->start_isr(vm);
									}

								}

							}
						}
					}
				}

			}
		}
		
		statusfile.close();
	}
}



void robot_voice_manager::write_status(int id, int pos, int state)
{
	std::string speech_status_file;
	get_statusfilepath(speech_status_file);
	if (!speech_status_file.empty())
	{
		std::string context;
		std::string temp;
		std::ifstream  statusfile(speech_status_file.c_str());
		if (statusfile.is_open())
		{
			while (std::getline(statusfile, temp))
			{
				if (!temp.empty())
					context.append(temp);
			}
			statusfile.close();
		}


		Json::Value   json_object,one_speech;
		Json::Reader  json_reader;
		Json::FastWriter  json_writer;

		bool found = false;
		bool b_json_read = json_reader.parse(context, json_object);

	
		if (b_json_read)
		{
			if (json_object.isMember("list") && json_object["list"].isArray())
			{
				Json::Value speech_list = json_object["list"];
				for (int i = 0; i < speech_list.size(); i++)
				{
					if (speech_list[i].isObject())
					{
						if (speech_list[i].isMember("id") && speech_list[i]["id"].isInt())
						{
							int sid = speech_list[i]["id"].asInt();
							if (sid == id)
							{
								found = true;
								if (pos == 0)
								{
									speech_list[i]["servicestatus"] = Json::Value(state == 0 ? false : true);
								}
								if (pos == 1)
								{
									speech_list[i]["isrstatus"] = Json::Value(state == 0 ? false : true);
								}

							}
							
						}
					}
				}
				json_object["list"] = speech_list;

			}
		}
		if (!found)
		{
			one_speech["id"] = id;
			if (pos == 0)
				one_speech["servicestatus"] = Json::Value(state == 0 ? false : true);
			if (pos == 1)
				one_speech["isrstatus"] = Json::Value(state == 0 ? false : true);

			json_object["list"].append(one_speech);
		}

		std::ofstream  statusoutfile(speech_status_file.c_str());
		std::string out = json_writer.write(json_object);
		statusoutfile.write(out.c_str(), out.size());
		statusoutfile.close();
	}
}

bool robot_voice_manager::start()
{
	int resume_laststate = 1;
	if (!common_config_get_field_int("Speech", "resume", &resume_laststate))
		resume_laststate = 1;
	if (resume_laststate)
		check_status();
	return true;
}

void robot_voice_manager::stop()
{
	auto it = m_speech_handler.begin();
	for (; it != m_speech_handler.end();it++)
	{
		if (it->second)
		{
			write_status(it->first, 0, 0);
			it->second->stop_voice_service();
			delete it->second;
		}
	}
	m_speech_handler.clear();
}

void robot_voice_manager::add_message_type(const char * message)
{
	m_message_list.push_back(message);
}




int robot_voice_manager::message_handler(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata /*= NULL*/, unsigned long dExtlen /*= 0*/)
{
	VoiceMessage   vmsg;
	vmsg.appid = dAppid;
	vmsg.fromguid = strFrom;
	vmsg.content = strMessage;
	Json::Value   json_object;
	Json::Reader  json_reader;

	bool b_json_read = json_reader.parse(strMessage, json_object);
	if (b_json_read)
	{
		std::string gbkstring;
		//common_utf8_2gbk(&vmsg.content, &gbkstring);
		//LOGD("Receive face message request %s", gbkstring.c_str());
		if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
		{
			std::string msgtype = json_object["msg_type"].asString();
			common_trim(&msgtype);
			int dmtype = -1;
			for (int i = 0; i < m_message_list.size(); i++)
			{
				if (msgtype.compare(m_message_list[i]) == 0)
				{
					dmtype = i;
					break;
				}
			}

			int speechid = 0;
			if (json_object.isMember("speechid") && json_object["speechid"].isInt())
				speechid = json_object["speechid"].asInt();
			else
				speechid = m_mainmic;

			int channel = 0;
			if (json_object.isMember("channel") && json_object["channel"].isInt())
				channel = json_object["channel"].asInt();


			switch (dmtype)
			{
			case 0:
			{
				int ret_value = 1;
				if (m_speech_handler.find(speechid) != m_speech_handler.end())
					ret_value = m_speech_handler[speechid]->start_voice_service();
				if (!ret_value)
					write_status(speechid, 0, 1);
				else
					write_status(speechid, 0, 0);
				vmsg.content = m_json.start_voice_service(ret_value);
				post_report(vmsg);
			}
				break;
			case 1:
			{
				if (json_object.isMember("isr_result_type") && json_object["isr_result_type"].isInt())
				{
					int isr_result = json_object["isr_result_type"].asInt();
					if (isr_result!=0)
						common_itoa_x(isr_result, &vmsg.in.request_result_type);
				}
				if (json_object.isMember("isr_result_type") && json_object["isr_result_type"].isString())
				{
					vmsg.in.request_result_type = json_object["isr_result_type"].asString();
				}

				if (json_object.isMember("loop") && json_object["loop"].isBool())
				{
					vmsg.in.loop = json_object["loop"].asBool();
				}
				if (json_object.isMember("mode") && json_object["mode"].isInt())
				{
					vmsg.in.mode = json_object["mode"].asInt();
				}

				int ret_value = 1;
				if (m_speech_handler.find(speechid) != m_speech_handler.end())
				{
					ret_value = m_speech_handler[speechid]->start_isr(vmsg);
				}
				if (ret_value != 2)
				{
					vmsg.content = m_json.start_once_isr(ret_value);
					post_report(vmsg);
				}

			}
			break;
			case 2:
			{
				int ret_value = 1;
				if (m_speech_handler.find(speechid) != m_speech_handler.end())
					ret_value = BOOLTOINT(m_speech_handler[speechid]->stop_isr());
				write_status(speechid, 1, 0);
				vmsg.content = m_json.stop_once_isr(ret_value);
				post_report(vmsg);
			}
			break;
			case 3:
			{
				int ret_value = 1;
				if (m_speech_handler.find(speechid) != m_speech_handler.end())
					ret_value = BOOLTOINT(m_speech_handler[speechid]->stop_voice_service());
				write_status(speechid, 0, 0);
				vmsg.content = m_json.stop_voice_service(ret_value);
				post_report(vmsg);
			}
			break;
			case 4:
			{
				if (json_object.isMember("text") && json_object["text"].isString())
					vmsg.in.text = json_object["text"].asString();//utf-8

				tts_message tsm;
				tsm.channels = channel;
				tsm.context = vmsg.in.text;
				int ret_value = 1;
				if (m_speech_handler.find(speechid) != m_speech_handler.end())
					ret_value = m_speech_handler[speechid]->tts(tsm);
	
				vmsg.content = m_json.tts_begin(channel,ret_value);
				post_report(vmsg);
			}
			break;
			case 5:
			{
				int ret_value = 1;
				if (m_speech_handler.find(speechid) != m_speech_handler.end())
					m_speech_handler[speechid]->stop_tts(channel);
				ret_value = 0;
				vmsg.content = m_json.tts_stop(channel,ret_value);
				post_report(vmsg);

			}
			break;
			case 6:
			{
				std::string name, value;
				int type = 1;
				if (json_object.isMember("paramName") && json_object["paramName"].isString())
					name = json_object["paramName"].asString();
				if (json_object.isMember("paramValue") && json_object["paramValue"].isString())
					value = json_object["paramValue"].asString();
				if (json_object.isMember("type") && json_object["type"].isInt())
					type = json_object["type"].asInt();
				int ret_value = 1;
				if (m_speech_handler.find(speechid) != m_speech_handler.end() && !value.empty() && !name.empty())
					ret_value = m_speech_handler[speechid]->set_tts(type,name, value, channel);
				vmsg.content = m_json.tts_set(ret_value);
				post_report(vmsg);
			}
			break;
			case 7:
			{
				std::string name, value;
				if (json_object.isMember("paramName") && json_object["paramName"].isString())
					name = json_object["paramName"].asString();
				if (json_object.isMember("paramValue") && json_object["paramValue"].isString())
					value = json_object["paramValue"].asString();
				int ret_value = 1;
				if (m_speech_handler.find(speechid) != m_speech_handler.end() && !value.empty() && !name.empty())
					ret_value = m_speech_handler[speechid]->set_isr(name, value);
				vmsg.content = m_json.isr_set(ret_value);
				post_report(vmsg);
			}
			break;
			case 8:
			{
				//todo for ccb ,easy,fix it!!!!
				int outnumber=waveOutGetNumDevs();
				int innumber=waveInGetNumDevs();
				LOGD("out :%d\t in :%d", outnumber, innumber);
				int ret_value = 1;
				Json::Value   json_object;
				Json::FastWriter json_writer;
				json_object["msg_type"] = Json::Value("GET_SPEECH_STATUS_RESPONSE");
				for (auto it = m_speech_handler.begin(); it != m_speech_handler.end();it++)
				{
					Json::Value one;
					one["speechid"] = Json::Value(it->first);
					Speech_Status info;
					it->second->get_status(info);
					one["isr_error"] = Json::Value(info.isr_error);
					one["isr_status"] = Json::Value(info.isr_run_state);
					one["tts_error"] = Json::Value(info.tts_error);
					one["tts_status"] = Json::Value(info.tts_run_state);
					json_object["list"].append(one);
				}
				json_object["speaker"] = Json::Value(outnumber>0?true:false);
				json_object["microphone"] = Json::Value(innumber>0 ? true : false);
				vmsg.content = json_writer.write(json_object);
				post_report(vmsg);
			}
			break;
			case 9:
			{

				int ret_value = 0;
				if (m_speech_handler.find(speechid) != m_speech_handler.end())
					m_speech_handler[speechid]->pause(channel);
				vmsg.content = m_json.tts_pause(channel,ret_value);
				post_report(vmsg);
			}
			break;
			case 10:
			{
				int ret_value = 0;
				if (m_speech_handler.find(speechid) != m_speech_handler.end())
					 m_speech_handler[speechid]->resume(channel);
				vmsg.content = m_json.tts_resume(channel,ret_value);
				post_report(vmsg);
			}
			break;
			case 11:
			{
				int ret_value = 1;
				if (json_object.isMember("angle") && json_object["angle"].isInt())
					vmsg.in.mode = json_object["angle"].asInt();
				vmsg.in.mode = abs(vmsg.in.mode);
				if (m_speech_handler.find(speechid) != m_speech_handler.end())
					ret_value=m_speech_handler[speechid]->enhance_mic(vmsg.in.mode);
				if (ret_value)ret_value = 0;
				else ret_value = 1;
				vmsg.content = m_json.irs_enhance_mic(ret_value);
				post_report(vmsg);
			}
			break;
			case 12:
			{
				int ret_value = 1;
				if (m_speech_handler.find(speechid) != m_speech_handler.end())
					ret_value = m_speech_handler[speechid]->reset_mic();
				if (ret_value)ret_value = 0;
				else ret_value = 1;
				vmsg.content = m_json.irs_reset_mic(ret_value);
				post_report(vmsg);
			}
			break;
			case 13:
			{
				std::string name, value;
				int type = 1;
				if (json_object.isMember("paramName") && json_object["paramName"].isString())
					name = json_object["paramName"].asString();
				if (json_object.isMember("type") && json_object["type"].isInt())
					type = json_object["type"].asInt();
				int ret_value = 1;
				if (m_speech_handler.find(speechid) != m_speech_handler.end() && !value.empty() && !name.empty())
					ret_value = m_speech_handler[speechid]->set_tts(type, name, value, channel);
				vmsg.content = m_json.tts_get(ret_value, name, value);
				post_report(vmsg);
			}
			break;
			case 14:
			{
				std::string name, value;
				if (json_object.isMember("paramName") && json_object["paramName"].isString())
					name = json_object["paramName"].asString();
				int ret_value = 1;
				if (m_speech_handler.find(speechid) != m_speech_handler.end() && !value.empty() && !name.empty())
					ret_value = m_speech_handler[speechid]->get_isr(name, value);
				vmsg.content = m_json.isr_get(ret_value, name, value);
				post_report(vmsg);
			}
			break;

			default:
			{
				if (msgtype.compare("AUDIO_START_STREAMNAME_RESPONSE") == 0)
				{
					int error = 1;
					if (json_object.isMember("error") && json_object["error"].isInt())
						error = json_object["error"].asInt();
					if (json_object.isMember("streamname") && json_object["streamname"].isString())
					{
						std::string streams = json_object["streamname"].asString();
						start_isr_write_status(streams, speechid, error, vmsg);			
					}
				}
				else if (msgtype.compare("AUDIO_STOP_STREAMNAME_RESPONSE") == 0)
				{

				}
				else
				{
					LOGE("voice module unkonw message %s", msgtype.c_str());
				}
				
			}

			}

		}
	}
	return 1;
	
}

void robot_voice_manager::start_isr_write_status(std::string streams, int speechid, int error, VoiceMessage &vmsg)
{
	int ret_value = 1;
	for (auto it = m_speech_handler.begin(); it != m_speech_handler.end(); it++)
	{
		speech_process_info temp;
		if (it->second->getInfo(temp) && temp.stream_name.compare(streams) == 0)
		{
			ret_value = m_speech_handler[speechid]->start_isr_result(error, vmsg);
			if (ret_value == 0 && temp.loop)
				write_status(speechid, 1, 1);
			else
				write_status(speechid, 1, 0);
			vmsg.content = m_json.start_once_isr(ret_value);
			post_report(vmsg);
			break;
		}
	}
}

void robot_voice_manager::post_report(VoiceMessage &vmsg)
{
	robot_PostMessage::getPostManager()->post(MODULE_NAME, vmsg.content.c_str(), vmsg.content.size(), vmsg.fromguid.c_str());
}

robot_voice_manager::robot_voice_manager()
{
}


robot_voice_manager::~robot_voice_manager()
{
}
