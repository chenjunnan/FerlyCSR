#include "cloudplatform_client.h"
#include "json\json.h"
#include "robot_PostMessage.h"

extern const char * MODULE_NAME;
#define EXCHANGE_NAME "voucher.robot.exchange"

cloudplatform_client::cloudplatform_client() :m_mq(this)
{
	m_http		= common_http_create();
	m_http_user = common_http_create();
	m_guard_data = common_mutex_create_threadlock();
	m_task_id = -1;
}



int cloudplatform_client::Init(std::string ip, std::string port, std::string user, std::string password)
{
	if (ip.empty() || atoi(port.c_str()) <= 0 || atoi(port.c_str())  >= 65535)
		return 0;

	m_ip=ip;
	m_port=port;
	m_user=user;
	m_password=password;
	return 1;
}

int cloudplatform_client::StartRun()
{
	return start();
}

void cloudplatform_client::Stop()
{
	quit();
}

std::string cloudplatform_client::repost(int error)
{
	Json::Value   json_object;
	Json::FastWriter json_writer;
	json_object["msg_type"] = Json::Value("CLOUD_POST_MESSAGE_RESPONSE");
	json_object["error"] = Json::Value(error);
	return json_writer.write(json_object);
}	

std::string cloudplatform_client::repost_MQ(int error)
{
	Json::Value   json_object;
	Json::FastWriter json_writer;
	json_object["msg_type"] = Json::Value("CLOUD_POSTMQ_MESSAGE_RESPONSE");
	json_object["error"] = Json::Value(error);
	return json_writer.write(json_object);
}

int cloudplatform_client::HandleMessage(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata /*= NULL*/, unsigned long dExtlen /*= 0*/)
{
	Json::Value   json_object;
	Json::Reader  json_reader;

	if (dMessageType == 0)
	{
		
		bool b_json_read = json_reader.parse(strMessage, json_object);
		if (b_json_read)
		{
			if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
			{
				std::string msgtype = json_object["msg_type"].asString();
				common_trim(&msgtype);
				if (msgtype.compare("CLOUD_POST_MESSAGE_REQUEST") == 0)
					tasklink_handler(json_object, strFrom);
				else if (msgtype.compare("CLOUD_POSTMQ_MESSAGE_REQUEST") == 0)
					MQmessage_handler(json_object, strFrom);
				else if (msgtype.compare("CLOUD_GETTASKID_REQUEST") == 0)
					GetRId(json_object, strFrom);
				else
					RPC_handler(json_object,msgtype);
			}

		}
		return 1;
	}
	
	return 1;
}


void cloudplatform_client::GetRId(Json::Value &json_object, const char * strFrom)
{
	Json::Value taskMessage_json;
	Json::FastWriter json_writer;
	
	taskMessage_json["msg_type"] = Json::Value("CLOUD_GETTASKID_RESPONSE");

	if (m_task_id != -1)
	{
		taskMessage_json["taskRID"] = m_task_id;
		json_object["error"] = Json::Value(0);
	}else
		json_object["error"] = Json::Value(1);

	std::string out = json_writer.write(taskMessage_json);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length(), strFrom);
}

void cloudplatform_client::test()
{
	if (m_mq.MQ_Getstate())
	{
		std::vector<char> data;
		for (int i = 0; i < 256; i++)
			data.push_back(i);
		m_mq.MQ_Push(m_robot_id.c_str(), data, EXCHANGE_NAME);
	}

}

void cloudplatform_client::MessageIn(std::vector<char> &data)
{
	std::string temp;
	temp.assign(data.begin(), data.end());
	Json::Value   json_object;
	Json::Reader  json_reader;
	Json::FastWriter json_writer;
	std::string show_text;
	common_utf8_2gbk(&temp, &show_text);
	LOGI("[MQ]:recv: %s", show_text.c_str());



	bool b_json_read = json_reader.parse(temp, json_object);
	if (b_json_read)
	{
		if (json_object.isMember("messageType") && json_object["messageType"].isString())
		{
			std::string mstype = json_object["messageType"].asString();
			if (mstype.compare("1") == 0)
			{

				if (json_object.isMember("taskInfo") && json_object["taskInfo"].isObject())
				{
					Json::Value taskinfo = json_object["taskInfo"];
					Json::Value temp;
					temp["msg_type"] = Json::Value("CLOUD_RECEIVE_MESSAGE_NOTIFY");
					temp["task"] = Json::Value(taskinfo);
					std::string msg = json_writer.write(temp);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, msg.c_str(), msg.length());

				}
				if (json_object.isMember("taskRecordId") && json_object["taskRecordId"].isInt())
				{
					m_task_id = json_object["taskRecordId"].asInt();
				}
				else
				{
					m_task_id = -1;
				}
				isOffline = false;
				LOGI("MQ catch message task id: %d ,message type :%s", m_task_id, mstype.c_str());
			}
			else if (mstype.compare("2") == 0)
			{
				LOCK_HELPER_AUTO_HANDLER(m_guard_data)
					std::string msg;
				int id_task;
				if (json_object.isMember("taskInfo") && json_object["taskInfo"].isObject())
				{
					Json::Value taskinfo = json_object["taskInfo"];
					Json::Value temp;
					temp["msg_type"] = Json::Value("CLOUD_RECEIVE_MESSAGE_NOTIFY");
					temp["task"] = Json::Value(taskinfo);
					msg = json_writer.write(temp);
				}
				if (json_object.isMember("taskRecordId") && json_object["taskRecordId"].isInt())
				{
					id_task = json_object["taskRecordId"].asInt();
				}
				else
					id_task = -1;
				isOffline = true;
				m_offline_task.push_back(TaskList(id_task, msg));
				if (m_offline_task.size() == 0)
				{
					m_offline_task.begin()->finish = true;
					robot_PostMessage::getPostManager()->post(MODULE_NAME, msg.c_str(), msg.length());
				}
				LOGI("MQ catch message task id: %d ,message type :%s", m_task_id, mstype.c_str());

			}
			else if (mstype.compare("3") == 0)
			{
				LOCK_HELPER_AUTO_HANDLER(m_guard_data)
				std::string json_mesage, json_type;
				std::string json_url;
				std::string type_url;
				if (json_object.isMember("robot_request") && json_object["robot_request"].isObject())
				{
					Json::Value json_robot = json_object["robot_request"];
					if (json_robot.isMember("messagecontext") && json_robot["messagecontext"].isString())
					{
						json_mesage = json_robot["messagecontext"].asString();

						if (!json_mesage.empty())
						{
							Json::Value   json_object1;
							bool b_jso = json_reader.parse(json_mesage.c_str(), json_object1);
							if (b_jso)
							{
								if (json_object1.isMember("msg_type") && json_object1["msg_type"].isString())
									json_type = json_object1["msg_type"].asString();
							}
							else
							{
								LOGE("json message context data error!!");
							}
						}

					}
					else if (json_robot.isMember("messagecontext") && json_robot["messagecontext"].isObject())
					{
						Json::Value json_robot_message = json_object["messagecontext"];

						if (json_robot_message.isMember("msg_type") && json_robot_message["msg_type"].isString())
							json_type = json_robot_message["msg_type"].asString();

						json_mesage = json_writer.write(json_robot_message);
					}

					
					if (json_robot.isMember("cburl") && json_robot["cburl"].isString())
						json_url = json_robot["cburl"].asString();
					if (json_robot.isMember("urltype") && json_robot["urltype"].isString())
						type_url = json_robot["urltype"].asString();
				}
				if (!json_mesage.empty() && !json_type.empty() && !json_url.empty())
				{
					int pos = json_type.find("REQUEST");
					if (pos != -1)
					{
						json_type.erase(pos, pos + strlen("REQUEST"));
						json_type.append("RESPONSE");
					}

					remote_request remote_request_t1;
					robot_PostMessage::getPostManager()->post(MODULE_NAME, json_mesage.c_str(), json_mesage.length(), "unname_dispath");
					remote_request_t1.callbackurl = m_url_head+json_url;
					remote_request_t1.httprequest_type = type_url;
					remote_request_t1.time = common_get_cur_time_stamps();
					remote_request_t1.msgtype = json_type;
					m_map_remote[json_type].push_back(remote_request_t1);
				}
				else
				{
					LOGE("json_mesage: %s,json_type:%s,json_url:%s .data error!!", json_mesage.c_str(), json_type.c_str(), json_url.c_str());
				}

			}
			else
			{
				//other module//TODO
			}

		}
	}

}

void cloudplatform_client::ThreadQuit()
{
	pushq(2);
}

cloudplatform_client::~cloudplatform_client()
{
	common_http_release(m_http_user);
	common_http_release(m_http);
	common_mutex_threadlock_release(m_guard_data);
}

void cloudplatform_client::clean_delay()
{
	LOCK_HELPER_AUTO_HANDLER(m_guard_data);
	for (auto it = m_map_remote.begin(); it != m_map_remote.end(); )
	{
		for (auto ita = it->second.begin(); ita != it->second.end(); )
		{
			if ((common_get_cur_time_stamps() - ita->time) > 10 * 60)
			{
				LOGE("call module message %s tiemout remove!!!", it->first.c_str());
				RPC_MessageTimeout(*ita);
				ita=it->second.erase(ita);
			}
			else
				ita++;
		}
		if (it->second.size() == 0)
			it = m_map_remote.erase(it);
		else
			it++;
	}

	for (auto it = m_error_retry_list.begin(); it != m_error_retry_list.end(); it++)
	{
		if ((common_get_cur_time_stamps() - it->time) > 60 * 60)
			it->remove_flag = true;
		else
		{
			int httpret = RPC_MessageHttp(*it);
			if (httpret == 0 || httpret == 2)
				it->remove_flag = true;
		}
	}


	for (auto it = m_error_retry_list.begin(); it != m_error_retry_list.end();) 
	{  
		if (it->remove_flag)
		{
			LOGE("rpc response message %s [%s] tiemout remove!!!", it->resultdata.c_str(), it->callbackurl.c_str());
			it = m_error_retry_list.erase(it);
		}
		else 
			++it;
	}
}

void cloudplatform_client::run()
{
	common_config_get_field_string("cloud_cfg", "robotserial", &m_robot_id);

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

	if (!common_config_get_field_string("cloud_cfg", "url", &m_cloud_url))
		return;

	if (!common_config_get_field_string("cloud_cfg", "callbackurl", &m_cloud_CBurl))
		return;

	common_config_get_field_string("cloud_cfg", "head", &m_url_head);

	while (1)
	{
		check_connect_cloud();
		Message msg = getq(3000);
		if (msg.msgType == _EXIT_THREAD_)break;
		if (msg.msgType == 2)	m_mq.MQ_Exit();
		clean_delay();
	}
	m_mq.MQ_Exit();
}

void cloudplatform_client::check_connect_cloud()
{
	if (!m_mq.MQ_Getstate())
	{
		VEC_BYTE       http_reponse;
		VEC_BYTE       post_value;
		std::string    post_head = "data=";
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["code"] = Json::Value(m_robot_id);
		std::string data = json_writer.write(json_object);
		post_head.append(data);
		post_value.assign(post_head.begin(), post_head.end());

		if (common_http_post(m_http, m_cloud_url.c_str(), &post_value, &http_reponse, 20) && http_reponse.size() > 0)
		{
			std::string reponse;
			reponse.assign(http_reponse.begin(), http_reponse.end());
			common_utf8_2gbk(&reponse, &reponse);
			Json::Value   json_object;
			Json::Reader  json_reader;

			if (json_reader.parse(reponse.c_str(), json_object))
			{
				LOCK_HELPER_AUTO_HANDLER(m_guard_data)
					if (json_object.isMember("resMsg") && json_object["resMsg"].isString())
					{
						std::string resmsg = json_object["resMsg"].asString();

						if (resmsg.compare("成功") == 0)
						{
							std::string queuename, robotid, name;
							if (json_object.isMember("record") && json_object["record"].isObject())
							{
								Json::Value   json_record = json_object["record"];
								if (json_record.isMember("code") && json_record["code"].isString())
								{
									robotid = json_record["code"].asString();
								}
								if (robotid.compare(m_robot_id) != 0)return;

								if (json_record.isMember("name") && json_record["name"].isString())
								{
									name = json_record["name"].asString();
									LOGI("my name is %s", name.c_str());
								}
								if (json_record.isMember("queneName") && json_record["queneName"].isString())
								{
									queuename = json_record["queneName"].asString();

									if (!queuename.empty())
									{
										int ret = m_mq.MQ_Login(m_ip.c_str(), m_port.c_str(), m_user.empty() ? NULL : m_user.c_str(), m_password.empty() ? NULL : m_password.c_str(),true);
										if (ret == 0)
										{
											//test();
											if (m_mq.GetQueueMessage(queuename.c_str(), m_robot_id.c_str(), EXCHANGE_NAME))
											{
												m_queue_name = queuename.c_str();
												LOGI("connect cloud success!!!");
											}
											else
											{
												m_mq.MQ_Exit();
											}

										}
									}

								}

							}
						}
					}

			}
			else
			{
				LOGE("error http reponse %s", reponse.c_str());
			}
		}
		else
		{
			;// LOGE("cant post http request");
		}

	}

}

void cloudplatform_client::tasklink_handler(Json::Value   &json_object, const char * strFrom)
{
	VEC_BYTE       http_reponse;
	VEC_BYTE       post_value;
	std::string    post_head = "data=";
	std::string taskmessage;
	int taskId;
	Json::Value   json_object_post;
	Json::Reader  json_reader;
	Json::Value taskMessage_json;
	Json::FastWriter json_writer;

	if (json_object.isMember("taskRID") && json_object["taskRID"].isInt())
		m_task_id = json_object["taskRID"].asInt();
	if (m_task_id!=-1)
		json_object_post["taskRecordId"] = Json::Value(m_task_id);

	json_object_post["robotId"] = Json::Value(m_robot_id);
	std::string status;
	int timeout_t = 20;
	if (json_object.isMember("taskStatus") && json_object["taskStatus"].isString())
	{
		status = json_object["taskStatus"].asString();
		json_object_post["taskStatus"] = Json::Value(status);
	}
	if (json_object.isMember("timeout") && json_object["timeout"].isInt())
		timeout_t = json_object["timeout"].asInt();
	if (json_object.isMember("taskId") && json_object["taskId"].isInt())
	{
		taskId = json_object["taskId"].asInt();
		json_object_post["taskId"] = Json::Value(taskId);
	}
	if (json_object.isMember("taskMessage") && json_object["taskMessage"].isString())
	{
		taskmessage = json_object["taskMessage"].asString();

		//json_object_post["taskMessage"] = Json::Value(taskmessage);
		if (!taskmessage.empty())
		{
			bool b_json_read = json_reader.parse(taskmessage, taskMessage_json);
			if (b_json_read)
				json_object_post["taskMessage"] = Json::Value(taskMessage_json);
		}

	}
	if (json_object.isMember("taskMessage") && json_object["taskMessage"].isObject())
	{
		 taskMessage_json = json_object["taskMessage"];
		json_object_post["taskMessage"] = Json::Value(taskMessage_json);
	}

	if (isOffline)
	{
		LOCK_HELPER_AUTO_HANDLER(m_guard_data);
		std::string data = repost(0);
		robot_PostMessage::getPostManager()->post(MODULE_NAME, data.c_str(), data.length(), strFrom);

		if (status.compare("success") == 0)
		{
			for (auto it = m_offline_task.begin(); it != m_offline_task.end(); it++)
			{
				if (!it->finish)
				{
					m_offline_task.begin()->finish = true;
					robot_PostMessage::getPostManager()->post(MODULE_NAME, it->taskinfo.c_str(), it->taskinfo.length());
					break;
				}
			}

		}


	}
	else
	{
		std::string data = json_writer.write(json_object_post);
		post_head.append(data);
		post_value.assign(post_head.begin(), post_head.end());
		LOGD("cloud post data :%s", data.c_str());

		if (common_http_post(m_http, m_cloud_CBurl.c_str(), &post_value, &http_reponse, timeout_t) && http_reponse.size() > 0)
		{
			LOGI("MQ post task %d -> %d success", m_task_id, taskId);
			data = repost(0);
		}
		else
		{
			LOGE("MQ post task %d -> %d failed", m_task_id, taskId);
			data = repost(1);
		}
		if (http_reponse.size() != 0)
		{
			std::string temp_http;
			temp_http.assign(http_reponse.begin(), http_reponse.end());
			common_utf8_2gbk(&temp_http, &temp_http);
			LOGD("HTTP reponse: %s", temp_http.c_str());

		}

		robot_PostMessage::getPostManager()->post(MODULE_NAME, data.c_str(), data.length(), strFrom);
	}


	// 					std::vector<char> data_v;
	// 					data_v.assign(data.begin(), data.end());
	//					data = repost(m_mq.MQ_Push(queue_name.c_str(),data_v));*/
}

void cloudplatform_client::MQmessage_handler(Json::Value &json_object, const char * strFrom)
{
	std::string message_context;
	std::string out_json;
	if (json_object.isMember("message") && json_object["message"].isString())
		message_context = json_object["message"].asString();
	int ret = 1;

	cloud_mq_server   cmqs;
	int retmq = cmqs.MQ_Login(m_ip.c_str(), m_port.c_str(), m_user.empty() ? NULL : m_user.c_str(), m_password.empty() ? NULL : m_password.c_str(),true);

	if (retmq == 0 && !message_context.empty())
	{
		std::vector<char> data;
		for (int i = 0; i < message_context.size(); i++)
			data.push_back(message_context[i]);
		LOGD("cloud mq post data :%s", message_context.c_str());
		ret = cmqs.MQ_Push(m_robot_id.c_str(), data, EXCHANGE_NAME);
		cmqs.MQ_Exit();
	}
	out_json = repost_MQ(ret);

	robot_PostMessage::getPostManager()->post(MODULE_NAME, out_json.c_str(), out_json.length(), strFrom);
}


void  cloudplatform_client::RPC_MessageTimeout(remote_request &data)
{
	Json::Value   json_object;
	std::string    post_head = "data=";
	Json::Value postdata;
	postdata["robotId"] = Json::Value(m_robot_id);
	json_object["msg_type"] = Json::Value(data.msgtype);
	json_object["error"] = Json::Value(100);
	json_object["errmsg"] = Json::Value("timeout 10 min");
	postdata["messagecontext"] = Json::Value(json_object);
	Json::FastWriter jsonWrite;
	std::string strJson = jsonWrite.write(postdata);
	post_head.append(strJson);
	data.resultdata = post_head;
	if (RPC_MessageHttp(data)==1)
	{
		data.time = common_get_cur_time_stamps();
		m_error_retry_list.push_back(data);
	}
}


int  cloudplatform_client::RPC_MessageHttp(remote_request &data)
{
	VEC_BYTE       http_reponse;
	VEC_BYTE       post_value;
	Json::Value   json_object;
	Json::Reader  json_reader;
	std::string reponse;

	if (data.httprequest_type.compare("GET") == 0)
	{
		if (!common_http_get(m_http_user, data.callbackurl.c_str(), &http_reponse, 60))
		{
			;// LOGD("Error get %s", msg.callbackurl.c_str());
			return 1;
		}
	}
	else
	{
		if (!data.resultdata.empty())
		{
			post_value.assign(data.resultdata.begin(), data.resultdata.end());
			LOGD("post :%s", data.resultdata.c_str());
			if (!common_http_post(m_http_user, data.callbackurl.c_str(), &post_value, &http_reponse, 60))
			{
				;// LOGD("Error post %s", msg.callbackurl.c_str());
				return 1;
			}
		}
		else
		{
			LOGE("POST data is empty!");
			return 2;
		}

	}

	if (!http_reponse.empty())
	{
		reponse.assign(http_reponse.begin(), http_reponse.end());
		common_utf8_2gbk(&reponse, &reponse);
		LOGD("HTTP %s reponse: %s", data.callbackurl.c_str(), reponse.c_str());
	}

	if (json_reader.parse(reponse.c_str(), json_object))
	{
		if (json_object.isMember("resMsg") && json_object["resMsg"].isString())
		{
			std::string resmsg = json_object["resMsg"].asString();

			if (resmsg.compare("成功") == 0)
			{
				LOGI("callback %s success", data.callbackurl.c_str());
				return 0;
			}
		}
	}
	return 2;
}

void cloudplatform_client::RPC_handler(Json::Value   &json_object, std::string &msgtype)
{
	LOCK_HELPER_AUTO_HANDLER(m_guard_data);
	if (m_map_remote.find(msgtype) != m_map_remote.end())
	{
		if (m_map_remote[msgtype].size() != 0)
		{
			remote_request msg = *m_map_remote[msgtype].begin();
			m_map_remote[msgtype].erase(m_map_remote[msgtype].begin());

			std::string    post_head = "data=";
			Json::Value postdata;
			postdata["robotId"] = Json::Value(m_robot_id);
			postdata["messagecontext"] = Json::Value(json_object);
			Json::FastWriter jsonWrite;
			std::string strJson = jsonWrite.write(postdata);
			post_head.append(strJson);
			msg.resultdata = post_head;
			if (RPC_MessageHttp(msg)==1)
			{
				msg.time = common_get_cur_time_stamps();
				m_error_retry_list.push_back(msg);
			}
		}
	}
	else
	{
		LOGD("none history %s", msgtype.c_str());
	}
}
