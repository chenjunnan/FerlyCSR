#include "robot_expression_module.h"
#include "robot_json_rsp.h"
#include "robot_PostMessage.h"
#include <algorithm>

#define ROBOT_EXPRESSION_APP 1001
extern const char *MODULE_NAME;
robot_expression_module::robot_expression_module()
{
}


void robot_expression_module::init()
{
	std::string request = "{\"msg_type\":\"DISPATCH_REGISTER_MESSAGE_MONITOR_REQUEST\",\"name\":\"client_service\"}";
	robot_PostMessage::getPostManager()->post(MODULE_NAME, request.c_str(), request.size(), "dispath_service");
}

void robot_expression_module::stop()
{
	m_tasklist.clear();
}

int robot_expression_module::message_handler(Expression_task &rcs)
{
	remove_overdue();
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
			if (msgtype.compare("EXPRESSION_CHANGE_REQUEST") == 0)
			{
				for (auto it = expression_client_list.begin(); it != expression_client_list.end(); it++)
				{
					robot_PostMessage::getPostManager()->post(MODULE_NAME, rcs.content.c_str(), rcs.content.length(), it->c_str());
					rcs.expressionpost_list.insert(*it);
				}
				if (rcs.expressionpost_list.size() == 0)
				{
					LOGW("unconnect expression controller client");
					rcs.content = robot_json.default_expression_return(336);
					rcs.return_state = 336;
					robot_PostMessage::getPostManager()->post(MODULE_NAME, rcs.content.c_str(), rcs.content.length(), rcs.fromguid.c_str());
				}else
					m_tasklist.push_back(rcs);
			}
			if (msgtype.compare("EXPRESSION_CHANGE_RESPONSE") == 0)
			{
				if (rcs.appid == ROBOT_EXPRESSION_APP&&m_tasklist.size() != 0)
				{
					auto findit = m_tasklist.end();
					for (auto it = m_tasklist.begin(); it != m_tasklist.end();it++)
					{
						if (it->expressionpost_list.find(rcs.fromguid) != it->expressionpost_list.end())
						{
							findit = it;
							break;
						}
							
					}
					if (findit != m_tasklist.end())
					{
						findit->expressionpost_list.erase(rcs.fromguid);
						if (findit->expressionpost_list.size() == 0)
						{	
							rcs.content.erase(std::remove(rcs.content.begin(), rcs.content.end(), '\\n'), rcs.content.end());
							robot_PostMessage::getPostManager()->post(MODULE_NAME, rcs.content.c_str(), rcs.content.length(), findit->fromguid.c_str());
							m_tasklist.erase(findit);
						}
					}

				}
				else
				{
					LOGW("error expression response message");
				}
			}
			if (rcs.fromguid.compare("dispath_service")==0 && rcs.appid == 16 && msgtype.compare("DISPATCH_CLIENT_CONNECT_NOTIFY") == 0)
			{
				if (json_object.isMember("guid") && json_object["guid"].isString())
				{
					std::string uid = json_object["guid"].asString();
					if (json_object.isMember("appid") && json_object["appid"].isString())
					{
						std::string  appidstr = json_object["appid"].asString();
						int appid = atoi(appidstr.c_str());
						if (appid == ROBOT_EXPRESSION_APP)
						{
							expression_client_list.insert(uid);
						}
					}

				}
			}
			if (rcs.fromguid.compare("dispath_service") == 0 && rcs.appid == 16 && msgtype.compare("DISPATCH_CLIENT_DISCONNECT_NOTIFY") == 0)
			{
				if (json_object.isMember("guid") && json_object["guid"].isString())
				{
					std::string uid = json_object["guid"].asString();
					bool express_disconnect = false;
					if (expression_client_list.find(uid) != expression_client_list.end())
					{
						expression_client_list.erase(uid);
						express_disconnect = true;
					}
						
					if (express_disconnect)
					{
						auto it = m_tasklist.begin();
						for (; it != m_tasklist.end(); it++)
						{
							for (auto setit = it->expressionpost_list.begin(); setit != it->expressionpost_list.end(); it++)
							{
								if (setit->compare(uid) == 0)
								{
									it->expressionpost_list.erase(uid);
									break;
								}
							}
						}
					}
					else
					{
						LOGI("client disconnect clean history message");
						for (auto it = m_tasklist.begin(); it != m_tasklist.end();)
						{
							if (it->fromguid.compare(uid) == 0)
								it=m_tasklist.erase(it);
							else
								it++;
						}

					}

				}
			}
		}
	}
	return 0;

}

void robot_expression_module::remove_overdue()
{
	auto it = m_tasklist.begin();
	for (; it != m_tasklist.end(); )
	{
		if ((common_get_longcur_time_stampms() - it->times) > 10 * 1000)
			it=m_tasklist.erase(it);
		else
			it++;
	}
}

robot_expression_module::~robot_expression_module()
{
}
