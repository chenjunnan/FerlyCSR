#include "PsycheManager.h"
#include "PsycheRTCAgora.h"
#include "json\json.h"
#include "robot_PostMessage.h"
extern const char* MODULE_NAME;
typedef  void(_cdecl *FDisableAgora)(void);

PsycheManager::PsycheManager() :player(NULL), _pws(this)
{
	m_http = common_http_create();
	m_state_ml = common_mutex_create_threadlock();
	//m_map_ml = common_mutex_create_threadlock();
	_uid = 0;
}


PsycheManager * PsycheManager::getInstance()
{
	static  PsycheManager  m_ins;
	return &m_ins;
}

bool PsycheManager::init()
{
	common_config_get_field_string("psyche", "streamname", &_streamname);

	common_config_get_field_string("psyche", "robotserial", &m_robot_id);


	

	if (m_robot_id.empty())
	{
		int ret = common_get_robot_serialID(&m_robot_id);
		if (ret)
		{
			LOGE("error robot serial %d", ret);
		}
		if (ret)
			common_get_guid(&m_robot_id);
	}

	LOGI("Robot ID:%s", m_robot_id.c_str());
	player = Getplayer();
	player->brccache();
	player->init(1,16000,32000,2,16);

	if (!common_config_get_field_string("psyche", "websocket", &_wsurl))
	{
		LOGE("ws url empty");
		return false;
	}


	if (!common_config_get_field_string("psyche", "token", &_tokenurl))
	{
		LOGE("token url empty");
		return false;
	}
	_rtc = new PsycheRTCAgora();
	_rtc->SetNotify(this);
	psyche_state = PSYCHEM_IDE;
	return true;
	//return _rtc->Init();
}

bool PsycheManager::StartManager()
{
	std::string request = "{\"msg_type\":\"DISPATCH_REGISTER_MESSAGE_MONITOR_REQUEST\",\"name\":\"ISR_RESULT_NOTIFY\"}";
	robot_PostMessage::getPostManager()->post(MODULE_NAME, request.c_str(), request.size(), "dispath_service");
	request = "{\"msg_type\":\"DISPATCH_REGISTER_MESSAGE_MONITOR_REQUEST\",\"name\":\"FACE_DETECT_NOTIFY\"}";
	robot_PostMessage::getPostManager()->post(MODULE_NAME, request.c_str(), request.size(), "dispath_service");
// 	std::string request = "{\"msg_type\":\"DISPATCH_REGISTER_MESSAGE_MONITOR_REQUEST\",\"name\":\"ISR_RESULT_SECTION_NOTIFY\"}";
// 	robot_PostMessage::getPostManager()->post(MODULE_NAME, request.c_str(), request.size(), "dispath_service");
	return start();
}

bool PsycheManager::StopManager()
{
	quit();
	if (_rtc->Status())
		_rtc->Leave();
	_rtc->Close();
	if (!player->isfinsh())player->stop();
	player->quit();
	Destoryplayer(player);
	_pws.quit();
	return true;
}	

std::string PsycheManager::package_websocket_context(std::string source, std::string fun,std::string packtype)
{
	Json::FastWriter  json_writer;
	Json::Value   json_object_request;
	

	json_object_request["source"] = source;
	json_object_request["robotId"] = m_robot_id;
	json_object_request["function"] = fun;
	/*json_object_request["data"] = Json::Value();*/
	unsigned long long times = common_get_cur_time_stamps();
// 	char buffer[32] = { 0 };
// 	sprintf(buffer, "%lld", times);
	json_object_request["packType"] = packtype;
	json_object_request["date"] = times;
	return json_writer.write(json_object_request);
}

std::string PsycheManager::package_websocket_context(std::string source, std::string fun, std::string packtype ,Json::Value  &data)
{
	Json::FastWriter  json_writer;
	Json::Value   json_object_request;

	json_object_request["source"] = source;
	json_object_request["robotId"] = m_robot_id;
	json_object_request["function"] = fun;
	/*json_object_request["data"] = Json::Value();*/
	unsigned long long times = common_get_cur_time_stamps();
// 	char buffer[32] = { 0 };
// 	sprintf(buffer, "%lld", times);
	json_object_request["date"] = times;
	json_object_request["packType"] = packtype;
	json_object_request["data"] = Json::Value(data);
	return json_writer.write(json_object_request);
}

int PsycheManager::HandleMessage(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata /*= NULL*/, unsigned long dExtlen /*= 0*/)
{

	Json::Value   json_object;
	Json::Reader  json_reader(Json::Features::strictMode());
	Json::FastWriter  json_writer;
	bool ret_value = false;
	int error = 1;
	std::string stream_name, reader_name;
	bool b_json_read = json_reader.parse(strMessage, json_object);
	if (b_json_read)
	{
		if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
		{
			//LOCK_HELPER_AUTO_HANDLER(m_map_ml);
			LOCK_HELPER_AUTO_HANDLER(m_state_ml);
			std::string msgtype = json_object["msg_type"].asString();
			common_trim(&msgtype);
			if (msgtype.compare("PSYCHEL_APPLYFOR_INTERVENTION_REQUEST") == 0)
			{
				do 
				{
					if (psyche_state != PSYCHEM_IDE)
					{
						error = 90001;
						break;
					}

					if (m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].size() != 0)
					{
						error = 90002;
						break;
					}

					if (_rtc->Status())
					{
						error = 90003;
						break;
					}
					if (!_pws.isConnect())
					{
						error = 90004;
						break;
					}
					std::string request = package_websocket_context("robot", "call","request");	
					_channel.clear();
					error = _pws.PostCommand(OPCODE::OPCODETEXT, request.c_str(), request.size());
					if (error == 0)
					{
						REQUEST_CACHE_INFO temp;
						temp.fromer = strFrom;
						temp.requestdata = strMessage;
						temp.time = common_get_longcur_time_stampms();
						m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].push_back(temp);
						psyche_state = PSYCHEM_WAIT_CALL_WS_RESPONSE;
						LOGD("wait service response");
					}
					else
					{
						error = 90005;
						break;
					}
				} while (0);

				if (error)
				{
					std::string rep = _json.APPLYFOR_INTERVENTION(error);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size(), strFrom);
					return 1;
				}

			}
			else if (msgtype.compare("PSYCHEL_ABANDON_INTERVENTION_REQUEST") == 0)
			{
				psyche_state = PSYCHEM_IDE;

				if (_rtc->Status())
				{
					if (_rtc->Leave())error = 0;

					if (_pws.isConnect())
					{
						std::string request = package_websocket_context("robot", "leave", "request");
						_pws.PostCommand(OPCODE::OPCODETEXT, request.c_str(), request.size());
					}

					if (!_streamname.empty())
						close_camera();
				}
				else
					error = 0;


				std::string rep = _json.ABANDON_INTERVENTION(error);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size(), strFrom);


			}
			else if (msgtype.compare("PSYCHEL_POST_MESSAGEDATA_REQUEST") == 0)
			{
				if (_pws.isConnect())
				{
					Json::Value userdata;
					std::string packagetype = "noAck";
					std::string fun = "message";
					if (json_object.isMember("packType") && json_object["packType"].isString())
						packagetype = json_object["packType"].asString();
					if (json_object.isMember("function") && json_object["function"].isString())
						fun = json_object["function"].asString();
					if (json_object.isMember("data") && json_object["data"].isObject())
					{
						userdata = json_object["data"];
						std::string request = package_websocket_context("client", fun, packagetype, userdata);
						error = _pws.PostCommand(OPCODE::OPCODETEXT, request.c_str(), request.size());
					}
				}
				else
					error = 90004;

				std::string rep = _json.POST_MESSAGEDATA(error);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size(), strFrom);
				
			}
			else if (msgtype.compare("PSYCHEL_PUBLIC_MESSAGEDATA_REQUEST") == 0)
			{
				if (_pws.isConnect())
				{
					Json::Value userdata;
					if (json_object.isMember("data") && json_object["data"].isObject())
					{
						userdata = json_object["data"];
						std::string froms = strlen(strFrom) == 36 ? "client" : "robot";
						std::string request = package_websocket_context(froms, "push","noAck", userdata);
						error = _pws.PostCommand(OPCODE::OPCODETEXT, request.c_str(), request.size());
					}
				}

			}
			else if (msgtype.compare("PSYCHEL_GET_STATUS_REQUEST") == 0)
			{
				std::string rep = _json.PSYCHEL_GET_STATUS(_rtc->Status());
				robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size(), strFrom);
			}
			else if (msgtype.compare("PSYCHEL_SHUTUP_REQUEST") == 0)
			{
				if (_rtc->Status())
				{
					_rtc->Shutup();
					error = 0;

				}
				else
					error = 1;


				std::string rep = _json.PSYCHEL_SHUTUP(error);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size(), strFrom);
			}
			else if (msgtype.compare("PSYCHEL_SPEAK_REQUEST") == 0)
			{
				if (_rtc->Status())
				{
					_rtc->Speak();
					error = 0;

				}
				else
					error = 1;


				std::string rep = _json.PSYCHEL_SPEAK(error);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size(), strFrom);
			}
			else if(msgtype.compare("CAMERA_START_STREAMNAME_RESPONSE") == 0)
			{
				std::string stream, reader_name, streams;
				if (json_object.isMember("error") && json_object["error"].isInt())
					error = json_object["error"].asInt();
				if (json_object.isMember("reader") && json_object["reader"].isString())
					reader_name = json_object["reader"].asString();

				if (json_object.isMember("streamname") && json_object["streamname"].isString())
					streams = json_object["streamname"].asString();

				return handle_camera_response(error);
			}
			else if (msgtype.compare("CAMERA_RESUME_USER_RESPONSE") == 0)
			{
				;
			}
			else if (msgtype.compare("CAMERA_CHANGE_USER_RESPONSE") == 0)
			{			
				if (json_object.isMember("error") && json_object["error"].isInt())
					error = json_object["error"].asInt();
				return handle_camera_response(error);
			}
			else if (msgtype.compare("DISPATCH_MONITOR_MESSAGE_NOTIFY") == 0)
			{
				if (json_object.isMember("content") && json_object["content"].isString())
				{
					std::string  temp = json_object["content"].asString();
					Json::Value pushdata;
					if (json_reader.parse(temp.c_str(), pushdata))
					{
						if (pushdata.isMember("msg_type") && pushdata["msg_type"].isString())
						{
							std::string msg_type_dispath = pushdata["msg_type"].asString();
							if (msg_type_dispath.compare("ISR_RESULT_NOTIFY") == 0||
								msg_type_dispath.compare("FACE_DETECT_NOTIFY") == 0)
							{
								std::string froms = "robot";
								std::string request = package_websocket_context(froms, "push","noAck", pushdata);
								error = _pws.PostCommand(OPCODE::OPCODETEXT, request.c_str(), request.size());
							}
							if (msg_type_dispath.compare("ISR_RESULT_SECTION_NOTIFY") == 0)
							{

							}
						}
					}
					
				}				
			}
			else
			{
				if (m_map_remote.find(msgtype) != m_map_remote.end())
				{
					if (m_map_remote[msgtype].size() != 0)
					{
						REQUEST_CACHE_INFO msg = *m_map_remote[msgtype].begin();
						m_map_remote[msgtype].erase(m_map_remote[msgtype].begin());

						if (_pws.isConnect())
						{
							std::string strJson = package_websocket_context("robot", msg.function,"response", json_object);
							msg.resultdata = strJson;

							if (_pws.PostCommand(OPCODE::OPCODETEXT, strJson.c_str(), strJson.size()))
							{
								msg.time = common_get_cur_time_stamps();
								//m_error_retry_list.push_back(msg);
							}
						}
						
					}
				}
				else
				{
					// LOGD("none history %s", msgtype.c_str());
// 					Json::Value postdata;
// 					postdata["source"] = "robot";
// 					postdata["times"] = common_get_cur_time_stamps();
// 					postdata["robotId"] = Json::Value(m_robot_id);
// 					postdata["request"] = Json::Value(json_object);
// 					std::string strJson = json_writer.write(postdata);
// 					if (_pws.PostCommand(OPCODE::OPCODETEXT, strJson.c_str(), strJson.size()))
// 					{
// 						;
// 					}
				}
			}
		}
	}
	return 0;
	
}

int PsycheManager::handle_camera_response(int error)
{
	REQUEST_CACHE_INFO temp;

	if (m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].size() != 0)
		temp = *m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].begin();

	if (psyche_state == PSYCHEM_WAIT_CALL_CAMERA_BY_REMOTE_RESPONSE || psyche_state == PSYCHEM_WAIT_CALL_CAMERA_BY_REMOTE_RESPONSE_NOACK)
	{
		if (!error)error = _rtc->Join(_channel,_uid) ? 0 : 90010;
		Json::Value jsontemp;
		jsontemp["error"] = error;
		if (error)
		{
			jsontemp["errmsg"] = Json::Value("failed");
			close_camera();
		}
		else
		{
			std::string rep = _json.INTERVENTION_NOTIFY(91000, "join psyche system");
			robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size());
			jsontemp["errmsg"] = Json::Value("success");
		}
		if (psyche_state != PSYCHEM_WAIT_CALL_CAMERA_BY_REMOTE_RESPONSE_NOACK)
		{
			std::string response = package_websocket_context("robot", "join", "response", jsontemp);
		_pws.PostCommand(OPCODE::OPCODETEXT, response.c_str(), response.size());
		}
		psyche_state = PSYCHEM_IDE;
		return 1;
	}
	if (psyche_state == PSYCHEM_WAIT_CALL_CAMERA_RESPONSE)
	{
		LOGD("catch camera response");
		if (!error)error = _rtc->Call(_channel) ? 0 : 1;
		if (error)close_camera();
	}
	if (psyche_state == PSYCHEM_IDE)
	{
		error = 10;
		close_camera();
	}
	

	std::string rep = _json.APPLYFOR_INTERVENTION(error, 0, _channel);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size(), temp.fromer.c_str());
	if (m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].size()>0)
	m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].pop_front();
	if (psyche_state == PSYCHEM_WAIT_CALL_CAMERA_RESPONSE)
		psyche_state = PSYCHEM_IDE;
	return 1;
}

void PsycheManager::onRtcError(int error, const char *msg)
{
	if (!error)return;
	//LOCK_HELPER_AUTO_HANDLER(m_map_ml);
	LOCK_HELPER_AUTO_HANDLER(m_state_ml);
	LOGE("RTC error [%d] %s",error,msg);
	if (_rtc->Status())
	{
		std::string rep = _json.INTERVENTION_NOTIFY(91001, "leave psyche system");
		robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size());
		_rtc->Leave();
		Json::Value data;
		data["error"] = error;
		data["errmsg"] = msg;
		std::string request = package_websocket_context("robot", "psycheerror", "noAck", data);
		error = _pws.PostCommand(OPCODE::OPCODETEXT, request.c_str(), request.size());
		if (!_streamname.empty())
			close_camera();
	}
	LOGE("RTC close %s", msg);
}
void PsycheManager::onUserOffline(unsigned int uid, std::string stream)
{
	std::string rep = _json.PSYCHEL_USEROFFLINE(uid, stream);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size());
}
void PsycheManager::onUserOnline(unsigned int uid, std::string stream)
{
	std::string rep = _json.PSYCHEL_USERONLINE(uid, stream);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size());
}

void PsycheManager::open_camera()
{
	Json::Value   json_object;
	Json::Reader  json_reader;
	Json::FastWriter  json_writer;
	json_object["msg_type"] = Json::Value("CAMERA_START_STREAMNAME_REQUEST");
	json_object["reader"] = Json::Value("psychesystem");
	json_object["streamname"] = Json::Value(_streamname);
	std::string report = json_writer.write(json_object);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, report.c_str(), report.size(), "video_stream_module");

// 	std::string rep = _json.CAMERA_RESUME_USER(_streamname);
// 	robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size(), "video_stream_module");
}

void PsycheManager::close_camera()
{
	Json::Value   json_object;
	Json::Reader  json_reader;
	Json::FastWriter  json_writer;
	json_object["msg_type"] = Json::Value("CAMERA_STOP_STREAMNAME_REQUEST");
	json_object["reader"] = Json::Value("psychesystem");
	json_object["streamname"] = Json::Value(_streamname);
	std::string report = json_writer.write(json_object);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, report.c_str(), report.size(), "video_stream_module");

}

void PsycheManager::onTextJsonSingal(std::string &json)
{
	Json::Value json_object;
	Json::FastWriter  json_writer;
	Json::Reader  json_reader(Json::Features::strictMode());
	bool b_json_read = json_reader.parse(json.c_str(), json_object);
	if (b_json_read)
	{
		//LOCK_HELPER_AUTO_HANDLER(m_map_ml);
		LOCK_HELPER_AUTO_HANDLER(m_state_ml);
		LOGD("psyche json message request %s", json_object.toStyledString().c_str());
		std::string gbk;

		common_utf8_2gbk(&json, &gbk);
		printf("%s\n", gbk.c_str());

		std::string ercode;
		std::string erstr;
		std::string fun;
		std::string packtype;
		unsigned long long atimes = 0;
		if (json_object.isMember("source") && json_object["source"].isString())
		{
			std::string source = json_object["source"].asString();
			if (json_object.isMember("robotId") && json_object["robotId"].isString())
			{
				std::string robotId = json_object["robotId"].asString();
				if (robotId.compare(m_robot_id) != 0)
					return;
			}
			if (json_object.isMember("resCode") && json_object["resCode"].isString())
				ercode = json_object["resCode"].asString();

			if (json_object.isMember("resMsg") && json_object["resMsg"].isString())
				erstr = json_object["resMsg"].asString();

			if (json_object.isMember("packType") && json_object["packType"].isString())
				packtype = json_object["packType"].asString();

			if (json_object.isMember("function") && json_object["function"].isString())
				fun=json_object["function"].asString();

			if (json_object.isMember("date") && json_object["date"].isUInt64())
				atimes = json_object["date"].asUInt64();

// 			if ((common_get_cur_time_stamps() - atimes) > 60)
// 			{
// 				LOGW("Lost package time dif %d \n", (common_get_cur_time_stamps() - atimes));
// 				return;
// 			}
				
			if (source.compare("robot") == 0)
			{
				if (fun.compare("call") == 0 && packtype.compare("response") == 0)
					build_call(ercode, json_object);
				if (fun.compare("webCall") == 0 && packtype.compare("request") == 0)
					join_monitor(json_object);
				if (fun.compare("webLeave") == 0 && packtype.compare("response") != 0)
					web_leave(packtype);
				if (fun.compare("command") == 0 && packtype.compare("response") != 0)
					command_msg(json_object, packtype, fun);
				if (fun.compare("heartbeat") == 0 && packtype.compare("noAck") != 0)
				{

				}
				if (fun.compare("inviteRobotJoinRoom") == 0 && packtype.compare("noAck") == 0)
					live_join(ercode, json_object);
				if (fun.compare("robotVideoPushSwitch") == 0 && packtype.compare("noAck") == 0)
				{
					if (json_object.isMember("data") && json_object["data"].isObject())
					{
						Json::Value tempdata = json_object["data"];
						std::string  uid;
						int mute = 0;
						if (tempdata.isMember("mute") && tempdata["mute"].isInt())
							mute = tempdata["mute"].asInt();
						if (tempdata.isMember("uid") && tempdata["uid"].isString())
							uid = tempdata["uid"].asString();
						if (_rtc->Status())
							_rtc->SetLocalVideoMute(mute);
					}
				}
				if (fun.compare("robotAudioPushSwitch") == 0 && packtype.compare("noAck") == 0)
				{
					if (json_object.isMember("data") && json_object["data"].isObject())
					{
						Json::Value tempdata = json_object["data"];
						std::string  uid;
						int mute = 0;
						if (tempdata.isMember("mute") && tempdata["mute"].isInt())
							mute = tempdata["mute"].asInt();
						if (tempdata.isMember("uid") && tempdata["uid"].isString())
							uid = tempdata["uid"].asString();
						if (_rtc->Status())
							_rtc->SetLocalAudioMute(mute);
					}
				}
				if (fun.compare("robotVideoPullSwitch") == 0 && packtype.compare("noAck") == 0)
				{
					if (json_object.isMember("data") && json_object["data"].isObject())
					{
						Json::Value tempdata = json_object["data"];
						std::string  uid;
						int mute = 0;
						if (tempdata.isMember("mute") && tempdata["mute"].isInt())
							mute = tempdata["mute"].asInt();
						if (tempdata.isMember("uid") && tempdata["uid"].isString())
							uid = tempdata["uid"].asString();
						if (_rtc->Status())
							_rtc->SetRemoteVideoMuteStatus(atoi(uid.c_str()), mute);
					}
				}
				if (fun.compare("robotAudioPullSwitch") == 0 && packtype.compare("noAck") == 0)
				{
					if (json_object.isMember("data") && json_object["data"].isObject())
					{
						Json::Value tempdata = json_object["data"];
						std::string  uid;
						int mute = 0;
						if (tempdata.isMember("mute") && tempdata["mute"].isInt())
							mute = tempdata["mute"].asInt();
						if (tempdata.isMember("uid") && tempdata["uid"].isString())
							uid = tempdata["uid"].asString();
						if (_rtc->Status())
							_rtc->SetRemoteAudioMuteStatus(atoi(uid.c_str()), mute);
					}
				}
				if (fun.compare("robotLeave") == 0 && packtype.compare("noAck") == 0)
					web_leave(packtype);
			}
			if (source.compare("client") == 0)
			{
				if (json_object.isMember("data") && json_object["data"].isObject())
				{
					Json::Value robotrequest = json_object["data"];
					std::string request = _json.CLIENT_MESSAGE_NOTIFY(robotrequest,fun,packtype);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, request.c_str(), request.length());
				}
			}
		}
	}
	else
	{
		LOGD("psyche message request %s", json.c_str());
	}
}

void PsycheManager::command_msg(Json::Value &json_object, std::string packtype, std::string fun)
{
	Json::FastWriter json_writer;
	if (json_object.isMember("data") && json_object["data"].isObject())
	{
		Json::Value robotrequest = json_object["data"];
		if (robotrequest.isMember("msg_type") && robotrequest["msg_type"].isString())
		{
			std::string msgcontect = json_writer.write(robotrequest);
			std::string msgtype = robotrequest["msg_type"].asString();
			std::string request = json_writer.write(robotrequest);

			int pos = msgtype.find("REQUEST");
			if (pos != -1)
			{
				msgtype.erase(pos, pos + strlen("REQUEST"));
				msgtype.append("RESPONSE");
			}
			robot_PostMessage::getPostManager()->post(MODULE_NAME, msgcontect.c_str(), msgcontect.length(), "unname_dispath");

			if (packtype.compare("request") == 0)
			{
				REQUEST_CACHE_INFO remote_request_t1;
				remote_request_t1.time = common_get_cur_time_stamps();
				remote_request_t1.requestdata = request;
				remote_request_t1.function = fun;
				m_map_remote[msgtype].push_back(remote_request_t1);
			}


		}
	}
}

void PsycheManager::web_leave(std::string &packtype)
{
	psyche_state = PSYCHEM_IDE;

	int error = 0;
	if (_rtc->Status())
	{
		LOGI("psyche leave ");
		if (_rtc->Leave())error = 0;

		if (!_streamname.empty())
			close_camera();

		std::string rep = _json.INTERVENTION_NOTIFY(91001, "leave psyche system");
		robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size());


	}
	else
		error = 0;

	if (packtype.compare("request") == 0)
	{
		Json::Value temp;
		temp["error"] = error;
		temp["errmsg"] = Json::Value("success");
		std::string response = package_websocket_context("robot", "webLeave", "response", temp);
		_pws.PostCommand(OPCODE::OPCODETEXT, response.c_str(), response.size());
	}
}

void PsycheManager::join_monitor(Json::Value &json_object)
{
	Json::Value temp;
	if (psyche_state != PSYCHEM_IDE)
	{
		temp["error"] = 1001;
		temp["errmsg"] = Json::Value("wait a moment ");
		std::string response = package_websocket_context("robot", "webCall", "response", temp);
		_pws.PostCommand(OPCODE::OPCODETEXT, response.c_str(), response.size());
		return;
	}
	_channel.clear();
	if (json_object.isMember("data") && json_object["data"].isObject())
	{
		Json::Value temp = json_object["data"];	
		if (temp.isMember("channel") && temp["channel"].isString())
			_channel = temp["channel"].asString();

	}
	if (_channel.empty())
	{
		temp["error"] = 1002;
		temp["errmsg"] = Json::Value("channel is empty ");
		std::string response = package_websocket_context("robot", "webCall", "response", temp);
		_pws.PostCommand(OPCODE::OPCODETEXT, response.c_str(), response.size());
		return;
	}


	if (_streamname.empty())
	{
		int error = _rtc->Join(_channel) ? 0 : 1;
		temp["error"] = error;
		if (error)temp["errmsg"] = Json::Value("join failed");
		else temp["errmsg"] = Json::Value("success");

		std::string response = package_websocket_context("robot", "webCall", "response", temp);

		_pws.PostCommand(OPCODE::OPCODETEXT, response.c_str(), response.size());
		if (error == 0)
		{
			std::string rep = _json.INTERVENTION_NOTIFY(91000, "join psyche system");
			robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size());
		}
	}
	else
	{
		open_camera();
		psyche_state = PSYCHEM_WAIT_CALL_CAMERA_BY_REMOTE_RESPONSE;
		LOGD("wait camera response");
	}
}

void PsycheManager::build_call(std::string &ercode, Json::Value &json_object)
{
	if (m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].size() == 0)
		return;
	REQUEST_CACHE_INFO REQtemp = *m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].begin();
	std::string rep;
	int error = 10;
	do 
	{
		if (PSYCHEM_WAIT_CALL_WS_RESPONSE != psyche_state)
			break;
		
		if (ercode.compare("0000") != 0)
		{
			error = atoi(ercode.c_str());
			break;
		}

		if (json_object.isMember("data") && json_object["data"].isObject())
		{
			Json::Value temp = json_object["data"];
			_channel.clear();
			if (temp.isMember("channel") && temp["channel"].isString())
			{
				_channel = temp["channel"].asString();

				if (_streamname.empty())
				{
					error = _rtc->Call(_channel) ? 0 : 1;
					std::string rep = _json.APPLYFOR_INTERVENTION(error, 0, _channel);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size(), REQtemp.fromer.c_str());
					if (m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].size()>0)
					m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].pop_front();
					psyche_state = PSYCHEM_IDE;

				}
				else
				{
					open_camera();
					psyche_state = PSYCHEM_WAIT_CALL_CAMERA_RESPONSE;
					error = 0;
					LOGD("wait camera response");
				}
				return;

			}
		}


	} while (0);


	rep = _json.APPLYFOR_INTERVENTION(error);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size(), REQtemp.fromer.c_str());
	if (m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].size()>0)
	m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].pop_front();
	if (psyche_state == PSYCHEM_WAIT_CALL_WS_RESPONSE)
		psyche_state = PSYCHEM_IDE;
}

void PsycheManager::live_join(std::string &ercode, Json::Value &json_object)
{
	Json::Value temp;
	if (psyche_state != PSYCHEM_IDE)
	{
		return;
	}
	_channel.clear();
	if (json_object.isMember("data") && json_object["data"].isObject())
	{
		Json::Value temp = json_object["data"];
		if (temp.isMember("channel") && temp["channel"].isString())
			_channel = temp["channel"].asString();
		if (temp.isMember("uid") && temp["uid"].isInt())
			_uid = temp["uid"].asInt();
	}
	if (_channel.empty())
	{
		return;
	}
	if (_streamname.empty())
	{
		int error = _rtc->Join(_channel, _uid) ? 0 : 1;
		temp["error"] = error;
		if (error)temp["errmsg"] = Json::Value("join failed");
		else temp["errmsg"] = Json::Value("success");
		if (error == 0)
		{
			std::string rep = _json.INTERVENTION_NOTIFY(91000, "join psyche system");
			robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size());
		}
	}
	else
	{
		open_camera();
		psyche_state = PSYCHEM_WAIT_CALL_CAMERA_BY_REMOTE_RESPONSE_NOACK;
		LOGD("wait camera response");
	}
}
void PsycheManager::onDisconnect()
{
	//LOCK_HELPER_AUTO_HANDLER(m_map_ml);
	LOCK_HELPER_AUTO_HANDLER(m_state_ml);
	LOGE("Server Disconnect.");
	if (_rtc->Status())
	{
		std::string rep = _json.INTERVENTION_NOTIFY(91001,"leave psyche system");
		robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size());
		_rtc->Leave();
		if (!_streamname.empty())
			close_camera();
	}

	if (m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].size() != 0)
	{
		if (PSYCHEM_WAIT_CALL_WS_RESPONSE == psyche_state)
		{
			REQUEST_CACHE_INFO temp = *m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].begin();
			std::string rep = _json.APPLYFOR_INTERVENTION(90002);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, rep.c_str(), rep.size(), temp.fromer.c_str());
			if (m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].size()>0)
			m_map_remote["PSYCHEL_APPLYFOR_INTERVENTION_REQUEST"].pop_front();
			psyche_state = PSYCHEM_IDE;
		}
	}
	
}

PsycheManager::~PsycheManager()
{
	common_http_release(m_http);
	common_mutex_threadlock_release(m_map_ml);
	common_mutex_threadlock_release(m_state_ml);
}

void PsycheManager::play_startupsound()
{
	size_t result;
	char  *buf;
	std::string path;
	common_get_current_dir(&path);
	path.append("media\\startup.wav");
	FILE *fp1 = fopen(path.c_str(), "rb");//wav文件打开，打开读权限
	if (fp1 == NULL)
	{
		LOGE("file sound open filed!!");
		return;
	}
	fseek(fp1, 0, SEEK_END);//文件指针从0挪到尾部
	long filesize;
	filesize = ftell(fp1);//ftell求文件指针相对于0的便宜字节数，就求出了文件字节数

	rewind(fp1);//还原指针位置
	fseek(fp1, 44, SEEK_SET);//wav文件的指针从头向后移动44字节
	buf = (char *)malloc(sizeof(char)*filesize);//开辟空间给缓存数组

	if (buf == NULL)
	{
		printf("memory  error");
		fclose(fp1);//关闭文件指针
		return;
	}

	result = fread(buf, 1, (filesize - 44), fp1);//每次读一个字节到buf，同时求读的次数
	if (result != filesize - 44)//判断读的次数和文件大小是否一致
	{
		LOGE("reing error!!");
		free(buf);//释放buf
		fclose(fp1);//关闭文件指针
		return;
	}
	if (!player->isfinsh())player->stop();
	player->reset();
	player->sound((unsigned char*)buf, result);
	player->endsound();

	fclose(fp1);//关闭文件指针
	free(buf);//释放buf
	return;
}

void PsycheManager::run()
{
	bool loop = true;
	int sleeptims = 0;
// 	std::string chanl = "1dce5a9b1dd047d5bec6c27d7a98c5d3";
// 	_rtc->Call(chanl);
	unsigned long long timestoken=0;
	unsigned long long timeslog = 0;
	unsigned long long heartcount = 0;
	while (loop)
	{
		Message msg = getq(sleeptims);
		switch (msg.msgType)
		{
			case 0:
				break;
			case -1:
				break;
			case _EXIT_THREAD_:
				loop = false;
				continue;
				break;
			default:
				;
		}

		if (!_pws.state())
		{
			Json::Value json_value;
			Json::FastWriter writer;
			Json::Reader reader;
			std::string postdata;
			json_value["robotId"] = Json::Value(m_robot_id);
			postdata = writer.write(json_value);
			json_value.clear();
			VEC_BYTE post_value;
			VEC_BYTE post_out;;
			post_value.assign(postdata.begin(), postdata.end());
			common_http_set_header(m_http,"Content-type","application/json;charset=UTF-8");
			bool ret=common_http_post(m_http, _tokenurl.c_str(), &post_value, &post_out, 10);
			if (ret)
			{			
				std::string token;
				std::string response;
				response.insert(response.end(), post_out.begin(), post_out.end());
				std::string gbk;
				common_utf8_2gbk(&response, &gbk);
				printf("%s\n",gbk.c_str());
				if (reader.parse(response, json_value))
				{
					if (json_value.isMember("data") && json_value["data"].isObject())
					{
						Json::Value   json_object = json_value["data"];
						if (json_object.isMember("record") && json_object["record"].isObject())
						{
							Json::Value record = json_object["record"];
							if (record.isMember("token") && record["token"].isString())
								token = record["token"].asString();
						}
						if (json_object.isMember("resCode") && json_object["resCode"].isString())
						{
							std::string resCode = json_object["resCode"].asString();
							if (resCode.compare("0000"))
							{
								LOGE("apply token error %s.",resCode.c_str());
								sleeptims = 10000;
								continue;
							}
						}

					}
					if (json_value.isMember("timestamp") && json_value["timestamp"].isString())
					{
						std::string timestamps = json_value["timestamp"].asString();
						timestoken = atoi(timestamps.c_str());
					}
				}
				if (!token.empty())
					_token = token;
			}
			else
			{
				sleeptims = 10000;
				LOGE("Server connect error.");
				continue;
			}

			if (!_token.empty())
			{
				std::string ws_token_url = _wsurl;
				ws_token_url.append("?token=");
				ws_token_url.append(_token);
				_pws.setUrl(ws_token_url.c_str());
				_pws.start();
				play_startupsound();
// 				std::string chanl = "1dce5a9b1dd047d5bec6c27d7a98c5d3";
// 				_rtc->Call(chanl);
			}
		}
		else
		{
			if (_pws.isConnect())
			{
				std::string request = package_websocket_context("robot", "heartbeat", "noAck");
				int retws=_pws.PostCommand(OPCODE::OPCODETEXT, request.c_str(), request.size());
				if (retws == 0)
					heartcount++;
			}
			
		}
		sleeptims = 5000;
		if (common_get_longcur_time_stampms() - timeslog > (60 * 1000))
		{
			timeslog = common_get_longcur_time_stampms();
			LOGI("Psyche system heartbeat count :%d", heartcount);
		}

	}


}
