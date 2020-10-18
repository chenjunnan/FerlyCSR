#include "robot_face_manager.h"
#include "robot_face_detect_interface.h"
#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include <fstream>
#include <iostream>
#include "xCommon.h"
#include "robot_PostMessage.h"
#include "robot_face_msghandler.h"
#include "robot_face_template.h"


robot_face_manager::robot_face_manager()
{
	m_mainfaceid = 0;
}

robot_face_manager * robot_face_manager::getPtr()
{
	static robot_face_manager _s;
	return &_s;
}

bool robot_face_manager::init()
{
	int face_cnt=0;
	if (!common_config_get_field_int("Face", "facecount", &face_cnt))
		face_cnt=0;

	if (!common_config_get_field_int("Face", "mainface", &m_mainfaceid))
		m_mainfaceid = 0;

	while (face_cnt)
	{
		face_cnt--;
		robot_face_process * ptr = new robot_face_process;
		if (!ptr||!ptr->init(face_cnt))
		{
			LOGE("create face process failed !!");
			SAFE_DELETE(ptr);
		}
		else
		{
			face_process_content fcontent;
			ptr->get_processer_content(fcontent);
			m_face_processer_content.push_back(fcontent);
			m_face_handler.push_back(ptr);

		}

	}

	return m_face_handler.size()?true:false;

}

void robot_face_manager::get_statusfilepath(std::string &exepath)
{
	exepath.clear();
	common_get_current_dir(&exepath);

	exepath.append("status\\");
	if (!common_isDirectory(exepath.c_str()))
		common_create_dir(exepath.c_str());
	exepath.append("facestatus");
}

void robot_face_manager::check_status()
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
				Json::Value face_list = json_object["list"];
				for (int i = 0; i < face_list.size(); i++)
				{
					if (face_list[i].isObject())
					{
						if (face_list[i].isMember("id") && face_list[i]["id"].isInt())
						{
							int sid = face_list[i]["id"].asInt();
							bool startservice = false, infostatus = false, identifystatus = false;
							if (face_list[i].isMember("servicestatus") && face_list[i]["servicestatus"].isBool())
								startservice = face_list[i]["servicestatus"].asBool();
							if (face_list[i].isMember("infostatus") && face_list[i]["infostatus"].isBool())
								infostatus = face_list[i]["infostatus"].asBool();
							if (face_list[i].isMember("identifystatus") && face_list[i]["identifystatus"].isBool())
								identifystatus = face_list[i]["identifystatus"].asBool();

							if (startservice)
							{
								for (int j = 0; j < m_face_processer_content.size(); j++)
								{
									if (sid == m_face_processer_content[j].cameraid)
									{
										FaceMessage fm;
										m_face_handler[j]->start_detect_task(fm);

										if (infostatus)m_face_handler[j]->set_faceinfo(true);
										if (identifystatus)m_face_handler[j]->set_identify(true);
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



void robot_face_manager::write_status(int id, int pos, int state)
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


		Json::Value   json_object, one_speech;
		Json::Reader  json_reader;
		Json::FastWriter  json_writer;

		bool found = false;
		bool b_json_read = json_reader.parse(context, json_object);


		if (b_json_read)
		{
			if (json_object.isMember("list") && json_object["list"].isArray())
			{
				Json::Value face_list = json_object["list"];
				for (int i = 0; i < face_list.size(); i++)
				{
					if (face_list[i].isObject())
					{
						if (face_list[i].isMember("id") && face_list[i]["id"].isInt())
						{
							int sid = face_list[i]["id"].asInt();
							if (sid == id)
							{
								found = true;
								if (pos == 0)
								{
									face_list[i]["servicestatus"] = Json::Value(state == 0 ? false : true);
								}
								if (pos == 1)
								{
									face_list[i]["infostatus"] = Json::Value(state == 0 ? false : true);
								}
								if (pos == 2)
								{
									face_list[i]["identifystatus"] = Json::Value(state == 0 ? false : true);
								}

							}

						}
					}
				}
				json_object["list"] = face_list;

			}
		}
		if (!found)
		{
			one_speech["id"] = id;
			if (pos == 0)
				one_speech["servicestatus"] = Json::Value(state == 0 ? false : true);
			if (pos == 1)
				one_speech["infostatus"] = Json::Value(state == 0 ? false : true);
			if (pos == 2)
				one_speech["identifystatus"] = Json::Value(state == 0 ? false : true);
			json_object["list"].append(one_speech);
		}

		std::ofstream  statusoutfile(speech_status_file.c_str());
		std::string out = json_writer.write(json_object);
		statusoutfile.write(out.c_str(), out.size());
		statusoutfile.close();
	}
}


bool robot_face_manager::start()
{
	check_status();
	return true;
}

void robot_face_manager::stop()
{
	for each (auto pvt in m_face_handler)
	{
		if (pvt)
		{
			face_process_content fcontent;
			pvt->get_processer_content(fcontent);
			write_status(fcontent.cameraid, 0, 0);
			pvt->stop();
		}
	}
}

void robot_face_manager::add_message_type(const char * message)
{
	m_message_list.push_back(message);
}

void robot_face_manager::get_allprocesser(std::vector<face_process_content>&fc)
{
	fc.clear();
	fc = m_face_processer_content;
}

void robot_face_manager::post_report(FaceMessage &fmsg)
{
	robot_PostMessage::getPostManager()->post(MODULE_NAME, fmsg.content.c_str(), fmsg.content.size(), fmsg.fromguid.c_str());
}


int robot_face_manager::message_handler(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata /*= NULL*/, unsigned long dExtlen /*= 0*/)
{
	FaceMessage   fmsg;
	fmsg.appid = dAppid;
	fmsg.fromguid = strFrom;
	fmsg.content = strMessage;
	Json::Value   json_object;
	Json::Reader  json_reader;

	bool b_json_read = json_reader.parse(strMessage, json_object);
	if (b_json_read)
	{
		std::string gbkstring;
// 		common_utf8_2gbk(&fmsg.content, &gbkstring);
// 		LOGD("Receive face message request %s", gbkstring.c_str());
		if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
		{
			std::string msgtype = json_object["msg_type"].asString();
			common_trim(&msgtype);
			int result= handler_register_message(msgtype, fmsg, json_object);
			if (result)
			{
				if (msgtype.compare("CAMERA_START_STREAMNAME_RESPONSE") == 0)
				{
					int error = 1;
					std::string reader;
					if (json_object.isMember("error") && json_object["error"].isInt())
						error = json_object["error"].asInt();
					if (json_object.isMember("reader") && json_object["reader"].isString())
						reader = json_object["reader"].asString();

					if (json_object.isMember("streamname") && json_object["streamname"].isString())
					{
						std::string streams = json_object["streamname"].asString();
						for each (auto pvt in m_face_handler)
						{
							std::vector<std::string > name;
							pvt->get_streamname(name);
							auto it=find(name.begin(), name.end(), streams);
							if (it != name.end())
							{
								face_process_content fcontent;
								pvt->get_processer_content(fcontent);
								FaceMessage pushmesg;
								if (reader.compare(fcontent.camera_readername) == 0)
								{
									int ret = pvt->camera_message(error, streams, pushmesg);
									if (ret != 2)
									{
										if (!pushmesg.fromguid.empty())
										{
											pushmesg.content = m_json.start_face_service(ret, fcontent.cameraid);
											post_report(pushmesg);
											if (ret == 0 /*&& fcontent.cameraid == m_mainfaceid*/)
												robot_face_msghandler::getPtr()->pushq(FACE_DETECT_STRONG_NTF, pushmesg.fromguid);
										}

									}
									if (ret == 0)
										write_status(fcontent.cameraid, 0, 1);
								}

							}
						}
					}
					
				}
				if (msgtype.compare("CAMERA_STOP_STREAMNAME_RESPONSE") == 0)
				{

				}
				
			}

		}
	}
	return 1;

}

int robot_face_manager::handler_register_message(std::string &msgtype, FaceMessage &fmsg, Json::Value &json_object)
{
	int dmtype = -1;
	for (int i = 0; i < m_message_list.size(); i++)
	{
		if (msgtype.compare(m_message_list[i]) == 0)
		{
			dmtype = i;
			break;
		}
	}
	if (json_object.isMember("cameraid") && json_object["cameraid"].isArray())
	{
		Json::Value   json_camera = json_object["cameraid"];
		for (int i = 0; i < json_camera.size(); i++)
		{
			if (json_camera[i].isInt())
			{
				int cameraid = json_camera[i].asInt();
				for (int j = 0; j < m_face_processer_content.size();j++)
				{
					if (cameraid == m_face_processer_content[j].cameraid)
						fmsg.cameralist.push_back(cameraid);
				}
			}
				
		}
	}
	if (fmsg.cameralist.size() == 0)
		fmsg.cameralist.push_back(m_mainfaceid);

	switch (dmtype)
	{
	case 0:
	{
		for (int i = 0; i < fmsg.cameralist.size();i++)
		{
			for (int j = 0; j < m_face_processer_content.size(); j++)
			{
				if (fmsg.cameralist[i] == m_face_processer_content[j].cameraid)
				{
					int ret=m_face_handler[j]->start_detect_task(fmsg);
					if (ret != 2)
					{
						fmsg.content = m_json.start_face_service(ret, fmsg.cameralist[i]);
						post_report(fmsg);
						if (ret == 0 /*&& fcontent.cameraid == m_mainfaceid*/)
							robot_face_msghandler::getPtr()->pushq(FACE_DETECT_STRONG_NTF, fmsg.fromguid);
					}

				}
			}
		}
		return 0;
	}
	break;
	case 1:
	{
		for (int i = 0; i < fmsg.cameralist.size(); i++)
		{
			for (int j = 0; j < m_face_processer_content.size(); j++)
			{
				if (fmsg.cameralist[i] == m_face_processer_content[j].cameraid)
				{
					m_face_handler[j]->stop_detect();
					write_status(m_face_processer_content[j].cameraid, 0, 0);
					fmsg.content = m_json.stop_face_service(0, fmsg.cameralist[i]);
					post_report(fmsg);
				}
			}
		}

		return 0;
	}
	break;
	case 2:
	{
		bool showwindows = false;
		if (json_object.isMember("show") && json_object["show"].isBool())
			showwindows = json_object["show"].asBool();

		for (int i = 0; i < fmsg.cameralist.size(); i++)
		{
			for (int j = 0; j < m_face_processer_content.size(); j++)
			{
				if (fmsg.cameralist[i] == m_face_processer_content[j].cameraid)
				{
					m_face_handler[j]->set_debug(showwindows);
					fmsg.content = m_json.debug_face_show(0);
					post_report(fmsg);
				}
			}
		}
		return 0;
	}
	break;
	case 3:
	{
		for (int i = 0; i < fmsg.cameralist.size(); i++)
		{
			for (int j = 0; j < m_face_processer_content.size(); j++)
			{
				if (fmsg.cameralist[i] == m_face_processer_content[j].cameraid)
				{
					m_face_handler[j]->set_faceinfo(true);
					write_status(m_face_processer_content[j].cameraid, 1, 1);
					fmsg.content = m_json.face_info_begin(0, fmsg.cameralist[i]);
					post_report(fmsg);
				}
			}
		}
		return 0;
	}
	break;
	case 4:
	{
		for (int i = 0; i < fmsg.cameralist.size(); i++)
		{
			for (int j = 0; j < m_face_processer_content.size(); j++)
			{
				if (fmsg.cameralist[i] == m_face_processer_content[j].cameraid)
				{
					m_face_handler[j]->set_faceinfo(false);
					write_status(m_face_processer_content[j].cameraid, 1, 0);
					fmsg.content = m_json.face_info_stop(0, fmsg.cameralist[i]);
					post_report(fmsg);
				}
			}
		}
		return 0;
	}
	break;
	case 5:
	{
		if (json_object.isMember("timeout") && json_object["timeout"].isInt())
			fmsg.timeout = json_object["timeout"].asInt();
		if (json_object.isMember("savetype") && json_object["savetype"].isInt())
			fmsg.savetype = json_object["savetype"].asInt();
		for (int i = 0; i < fmsg.cameralist.size(); i++)
		{
			for (int j = 0; j < m_face_processer_content.size(); j++)
			{
				if (fmsg.cameralist[i] == m_face_processer_content[j].cameraid)
				{
					m_face_handler[j]->start_wait_face_image(fmsg);
				}
			}
		}
		return 0;
	}
	break;
	case 6:
	{
		for (int i = 0; i < fmsg.cameralist.size(); i++)
		{
			for (int j = 0; j < m_face_processer_content.size(); j++)
			{
				if (fmsg.cameralist[i] == m_face_processer_content[j].cameraid)
				{
					m_face_handler[j]->stop_wait_face_image();
					fmsg.content = m_json.cancel_face_snapshot(0, fmsg.cameralist[i]);
					post_report(fmsg);
				}
			}
		}

		return 0;

	}
	break;
	case 7:
	{
		if (json_object.isMember("type") && json_object["type"].isString())
		{
			std::string type = json_object["type"].asString();
			if (type.find("memory") != std::string::npos)
				fmsg.reg_type = MEMORY_FACE_REGISTER;
			else if (type.find("file") != std::string::npos)
				fmsg.reg_type = FILE_FACE_REGISTER;
			else if (type.find("base64") != std::string::npos)
				fmsg.reg_type = BASE64_FACE_REGISTER;
		}
		if (json_object.isMember("registerinfo") && json_object["registerinfo"].isArray())
		{
			Json::Value   json_registerinfo = json_object["registerinfo"];

			for (int i = 0; i < json_registerinfo.size(); i++)
			{
				Json::Value  json_registerinfo_one = json_registerinfo[i];
				RealtimeIdentifyFaceInfo f_info;
				if (json_registerinfo_one.isMember("name") && json_registerinfo_one["name"].isString())
					f_info.name = json_registerinfo_one["name"].asString();
				if (json_registerinfo_one.isMember("id") && json_registerinfo_one["id"].isInt())
					f_info.id = json_registerinfo_one["id"].asInt();
				if (json_registerinfo_one.isMember("data") && json_registerinfo_one["data"].isString())
					f_info.data = json_registerinfo_one["data"].asString();
				if (json_registerinfo_one.isMember("userdata") && json_registerinfo_one["userdata"].isString())
					f_info.userdata = json_registerinfo_one["userdata"].asString();
				if (json_registerinfo_one.isMember("sex") && json_registerinfo_one["sex"].isInt())
					f_info.face_reltimes.sex = json_registerinfo_one["sex"].asInt();
				if (json_registerinfo_one.isMember("age") && json_registerinfo_one["age"].isInt())
					f_info.face_reltimes.age = json_registerinfo_one["age"].asInt();
				if (!f_info.name.empty())
					fmsg.list.push_back(f_info);

			}
		}

		bool postok = false;
		for (int i = 0; i < fmsg.cameralist.size(); i++)
		{
			for (int j = 0; j < m_face_processer_content.size(); j++)
			{
				if (fmsg.cameralist[i] == m_face_processer_content[j].cameraid)
				{
					if (m_face_handler[j]->state())
					{
						FaceMessage *msg_syn = new FaceMessage;
						*msg_syn = fmsg;
						msg_syn->cameralist.clear();
						msg_syn->cameralist.push_back(fmsg.cameralist[i]);
						m_face_handler[j]->pushq(FACE_REGISTER_MESSAGE, "", msg_syn);
						postok = true;
					}
				}
			}

		}
		if (!postok)
		{
			fmsg.error = SERVICE_CLOSEING;
			fmsg.content = m_json.reg_face_begin(fmsg);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, fmsg.content.c_str(), fmsg.content.size(), fmsg.fromguid.c_str());
		}
		return 0;
	}
	break;
	case 8:
	{
		for (int i = 0; i < fmsg.cameralist.size(); i++)
		{
			for (int j = 0; j < m_face_processer_content.size(); j++)
			{
				if (fmsg.cameralist[i] == m_face_processer_content[j].cameraid)
				{
					m_face_handler[j]->set_identify(true);
					write_status(m_face_processer_content[j].cameraid, 2, 1);
					fmsg.content = m_json.identify_face_begin(0);
					post_report(fmsg);
				}
			}
		}
		return 0;
	}
	break;
	case 9:
	{
		for (int i = 0; i < fmsg.cameralist.size(); i++)
		{
			for (int j = 0; j < m_face_processer_content.size(); j++)
			{
				if (fmsg.cameralist[i] == m_face_processer_content[j].cameraid)
				{
					m_face_handler[j]->set_identify(false);
					write_status(m_face_processer_content[j].cameraid, 2, 0);
					fmsg.content = m_json.identify_face_stop(0);
					post_report(fmsg);
				}
			}
		}
		return 0;
	}
	break;
	case 10:
	{
		if (json_object.isMember("name") && json_object["name"].isString())
		{
			std::string name = json_object["name"].asString();
			robot_face_template::getInstance()->delTemplatebyName(name);
		}
		fmsg.content = m_json.remove_face(0);
		post_report(fmsg);
		return 0;
	}
	break;
	case 11:
	{
		std::map<std::string, FaceDatabaseFormat> facelist;
		robot_face_template::getInstance()->UpdateTemplate(facelist);
		fmsg.content = m_json.get_face_namelist(facelist);
		post_report(fmsg);
		return 0;
	}
	break;
	case 12:
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_STATUS_RESPONSE");

		for (int j = 0; j < m_face_processer_content.size(); j++)
		{
			Json::Value one;
			one["cameraid"] = Json::Value(m_face_processer_content[j].cameraid);
			face_status_info info;
			m_face_handler[j]->get_status(info);
			one["error"] = Json::Value(info.error);
			one["hasperson"] = Json::Value(info.has_person);
			one["status"] = Json::Value(info.run_state);
			json_object["list"].append(one);
		}
		fmsg.content = json_writer.write(json_object);
		post_report(fmsg);
		return 0;
	}
	break;
	case 13:
	{
		if (json_object.isMember("savetype") && json_object["savetype"].isInt())
			fmsg.savetype = json_object["savetype"].asInt();
		for (int i = 0; i < fmsg.cameralist.size(); i++)
		{
			for (int j = 0; j < m_face_processer_content.size(); j++)
			{
				if (fmsg.cameralist[i] == m_face_processer_content[j].cameraid)
				{
					m_face_handler[j]->start_wait_normal_image(fmsg);
				}
			}
		}
		return 0;
	}
	break;

// 	default:
// 		LOGE("face module unkonw message %s", msgtype.c_str());
	}
	return 1;
}

robot_face_manager::~robot_face_manager()
{

}
