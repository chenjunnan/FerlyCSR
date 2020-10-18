#include "robot_ICreader_module.h"
#include "robot_PostMessage.h"

extern const char *MODULE_NAME;

robot_ICreader_module::robot_ICreader_module()
{
	 card_type=0;
	 result_type=0;
	 timeout = 0;
}

bool robot_ICreader_module::message_handler(icreader_message& rcs)
{
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(rcs.content, json_object);
	if (b_json_read)
	{
		std::string gbkstring;
		//common_utf8_2gbk(&rcs.content, &gbkstring);
		//LOGD("Receive face message request %s", gbkstring.c_str());
		if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
		{
			std::string msgtype = json_object["msg_type"].asString();
			common_trim(&msgtype);

			if (msgtype.find("ICCARD_WAIT_REQUEST") != std::string::npos)
			{
				rcs.tasktype = 0;
				if (json_object.isMember("result_type") && json_object["result_type"].isString())
				{
					std::string result_t = json_object["result_type"].asString();
					if (result_t.find("ascii") != -1)
						rcs.type = 0;
					if (result_t.find("bin") != -1)
						rcs.type = 1;
				}
				if (json_object.isMember("card_mode") && json_object["card_mode"].isString())
				{
					std::string card_mode = json_object["card_mode"].asString();
					if (card_mode.find("mag") != -1)
						rcs.card_type = 0;
				}
				if (json_object.isMember("timeout") && json_object["timeout"].isInt())
				{
					rcs.timeout = json_object["timeout"].asInt();
				}

			}
			if (msgtype.find("ICCARD_CANCEL_REQUEST") != std::string::npos)
			{
				rcs.tasktype = 1;
			}
			if (msgtype.find("ICCARD_REMOVE_REQUEST") != std::string::npos)
			{
				rcs.tasktype = 2;
			}
		}
	}
	icreader_message *ptr = new icreader_message;
	*ptr = rcs;
	pushq(rcs.tasktype, "", ptr);
	return true;
}


bool robot_ICreader_module::init(char *port, int type, int result, int timeouts)
{
	if (port == NULL || strlen(port) <= 0)
	{
		LOGE("init port error");
		return false;
	}
		
	if (!m_crt310.Init(port))
	{
		LOGE("Open Device error");
		return false;
	}
// 	if (m_crt310.Reset_Card())
// 	{
// 		LOGE("Reset Device error");
// 		return false;
// 	}
	card_type = type;
	result_type = result;
	timeout = timeouts;
	return  true;
}

int robot_ICreader_module::init()
{
	if (!common_config_get_field_string("iccard", "port", &m_port))///sql config
		m_port="COM3";
	return 0;
}

void robot_ICreader_module::run()
{
	int has_task = 5000;
	Timer_helper now;
	clean_msgqueue();
	std::vector<std::string > result_str;

	std::string temp;
	bool need_hold_card = false;

	char port[63] = { 0 };
	strcpy(port, m_port.c_str());

	while (1)
	{
		Message  msg = getq(has_task);
		switch (msg.msgType)
		{
		case  0:
		{

			if (!msg.msgObject) break;

			icreader_message *rsmsg = (icreader_message *)msg.msgObject;

			if (has_task != 0)
			{
				now.update();
				if (!init(port, rsmsg->card_type, rsmsg->type, rsmsg->timeout))
				{
					has_task = 5000;
					m_crt310.Close_Device();
					rsmsg->return_state = 1;
					need_hold_card = false;
					rsmsg->errors = m_crt310.get_error();
				}
				else
				{
					task_appid = rsmsg->appid;
					task_client_guid = rsmsg->fromguid;
					has_task = 0;
					rsmsg->return_state = 0;
					need_hold_card = true;
				}						
			}
			else
				rsmsg->return_state = 0;

			rsmsg->content = m_json.iccard_wait(*rsmsg);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, rsmsg->content.c_str(), rsmsg->content.length(), rsmsg->fromguid.c_str());
			SAFE_DELETE(rsmsg);

		}
			break;
		case 1:
		{
			if (!msg.msgObject) break;
			has_task = 5000;
			//m_crt310.Cancel_Card();
			//m_crt310.Close_Device();

			icreader_message *rsmsg = (icreader_message *)msg.msgObject;
			rsmsg->return_state = 0;
			rsmsg->content = m_json.iccard_cancel(*rsmsg);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, rsmsg->content.c_str(), rsmsg->content.length(), rsmsg->fromguid.c_str());
			SAFE_DELETE(rsmsg);
		}

			break;
		case 2:
		{
			if (!msg.msgObject) break;
			icreader_message *rsmsg = (icreader_message *)msg.msgObject;
			if (has_task != 0)
			{
				if (!init(port, 0, 0, 60))
				{
					m_crt310.Close_Device();
					rsmsg->return_state = 1;
					rsmsg->errors = m_crt310.get_error();

				}
				else
				{
					rsmsg->return_state = m_crt310.Cancel_Card();
					need_hold_card = false;
					rsmsg->errors = m_crt310.get_error();
					//m_crt310.Close_Device();
				}
				

			}
			else
				rsmsg->return_state = 4001;
			rsmsg->content = m_json.iccard_remove(*rsmsg);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, rsmsg->content.c_str(), rsmsg->content.length(), rsmsg->fromguid.c_str());
			SAFE_DELETE(rsmsg);

		}
			break;
		case  _EXIT_THREAD_:
			m_crt310.Cancel_Card();
			need_hold_card = false;
			m_crt310.Close_Device();
			return;
		case  -1:
			break;
		default:
			LOGE("unknow iccard msg type %d", msg.msgType);
		}

		if (has_task==0)
		{
			switch (card_type)
			{
			case 0:
			{
				int ret = 0;
				if (result_type==0)
					ret = m_crt310.ASCII_Read_Track(result_str);
				if (result_type == 1)
					ret = m_crt310.BIN_Read_Track(result_str);

				if (ret == OP_ERR)
				{
					LOGE("%s", m_crt310.get_error());
				}
				bool has_data = false;
				int error_cardinfo = 0;
				for (int i = 0; i < result_str.size(); i++)
				{
					if (!result_str[i].empty())
					{
						if (result_str[i].compare("has card error") == 0)
						{
							result_str[i].clear();
							error_cardinfo++;
						}
							
						has_data = true;
					}
				}
				

				if (result_str.size() > 0 && has_data)
				{
					icreader_message rsmsg ;
				
					rsmsg.appid = task_appid;
					rsmsg.fromguid = task_client_guid;
					rsmsg.return_state = error_cardinfo == 3 ? 4101 : 0;
					rsmsg.mag_data = result_str;
					rsmsg.type = result_type;
					rsmsg.content = m_json.iccard_result(rsmsg);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, rsmsg.content.c_str(), rsmsg.content.length(), rsmsg.fromguid.c_str());

					need_hold_card = false;
					has_task = 5000;
					common_thread_sleep(300);
					if (error_cardinfo==3)
						m_crt310.Cancel_Card();
					else
						need_hold_card = true;

					//m_crt310.Close_Device();

				}
				if (ret == OP_NO_CARD)
				{

				}
			}
				break;
			default:
				LOGE("error iccard reader %d", card_type)
			}


			if (has_task==0&&now.isreach(timeout * 1000))
			{
				has_task = 5000;
				m_crt310.Cancel_Card();
				//m_crt310.Close_Device();
				need_hold_card = false;
				icreader_message rsmsg;

				rsmsg.appid = task_appid;
				rsmsg.fromguid = task_client_guid;
				rsmsg.return_state = 4022;
				rsmsg.errors = "time out";
				rsmsg.content = m_json.iccard_result(rsmsg);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, rsmsg.content.c_str(), rsmsg.content.length(), rsmsg.fromguid.c_str());

			}
		}
		if (has_task == 5000 && !need_hold_card)
		{
			if (m_crt310.getstate())
				m_crt310.Cancel_Card();
		}

	}
}

robot_ICreader_module::~robot_ICreader_module()
{
	if (state())quit();
}
