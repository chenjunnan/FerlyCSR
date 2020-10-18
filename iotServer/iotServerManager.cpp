#include "iotServerManager.h"
#include "iotServerForWifi50315.h"
#include "json\json.h"
#include "common_helper.h"
#include "iotServerForLoRa.h"
#include "robot_PostMessage.h"
extern const char *MODULE_NAME;

iotServerManager::iotServerManager()
{
}


bool iotServerManager::Init()
{
	m_module_list["wifi_server"] = new iotServerForWifi50315();
	m_module_list["LoRa_server"] = new iotServerForLoRa();;

	for each (auto var in m_module_list)
	{
		;
	}
	return m_module_list.size() == 0 ? false:true;
}

bool iotServerManager::Start()
{
	common_log_create(MODULE_NAME);
	common_log_config(MODULE_NAME, true, true, 0);
	for each (auto var in m_module_list)
		var.second->start();
	return true;
}

bool iotServerManager::Stop()
{
	for each (auto var in m_module_list)
		var.second->quit();
	return true;
}

int iotServerManager::SetProcessData(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata /*= NULL*/, unsigned long dExtlen /*= 0*/)
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
				int messagetype = -1;
				msg_context msp;
				Thread_helper *Object=NULL;
				std::string module;
				messagetype = getMessgaeType(msgtype);

				if (messagetype > 0 && messagetype < 200)
				{
					if (json_object.isMember("id") && json_object["id"].isString())
						msp.id = json_object["id"].asString();

					if (json_object.isMember("mid") && json_object["mid"].isString())
						msp.mainid = json_object["mid"].asString();

					if (json_object.isMember("timeout") && json_object["timeout"].isInt())
						msp.timeout = json_object["timeout"].asInt() * 1000;

					if (json_object.isMember("mode") && json_object["mode"].isInt())
						msp.mode = json_object["mode"].asInt();

					if (json_object.isMember("module") && json_object["module"].isString())
						module = json_object["module"].asString();

					if (json_object.isMember("openlist") && json_object["openlist"].isArray())
					{
						Json::Value openarray = json_object["openlist"];
						for (int i = 0; i < openarray.size(); i++)
						{
							if (openarray[i].isObject())
							{
								if (openarray[i].isMember("layer") && openarray[i]["layer"].isInt())
								{
									if (openarray[i].isMember("surface") && openarray[i]["surface"].isString())
									{
										int surfacei = 0;
										std::string surfacestr = openarray[i]["surface"].asString();
										if (surfacestr.compare("front")==0)surfacei = 0;
										if (surfacestr.compare("back")==0)surfacei = 1;
										int layeri=openarray[i]["layer"].asInt();
										//printf("in %d---%d\n",layeri,surfacei);
										msp.door_list.insert(msp.door_list.end(), std::make_pair(layeri, surfacei));
									}
										
								}
							}
						}
					}

					if (module.empty())
					{
						if (messagetype == GETLIST)
							GetAllList(strFrom);
						else if (messagetype < SETBOXAP)//change LoRa
							Object = m_module_list["LoRa_server"];
						else
							Object = m_module_list["wifi_server"];
					}
					else
					{
						if (m_module_list.find(module) != m_module_list.end())
							Object = m_module_list[module];
						else
							LOGE("unknow module %s.", module.c_str());
					}


				}

						
				msp.guid = strFrom;

				if (Object)Object->pushq(messagetype, "", new msg_context(msp));
			}
		}
	}
	return 0;
}


void iotServerManager::GetAllList(const char * strFrom)
{
	Json::Value   json_object_post;
	Json::FastWriter json_writer;
	json_object_post["msg_type"] = Json::Value("IOT_GETID_LIST_RESPONSE");
	json_object_post["error"] = 0;
	json_object_post["msg"] = Json::Value("success");
	for each (auto var in m_module_list)
		var.second->AppendPortList(json_object_post);
	std::string result = json_writer.write(json_object_post);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, result.c_str(), result.length(), strFrom);
}

int iotServerManager::getMessgaeType(std::string &msgtype)
{
	if (msgtype.compare("IOT_GETID_LIST_REQUEST") == 0)
		return  GETLIST;
	if (msgtype.compare("IOT_OPEN_DOOR_REQUEST") == 0)
		return OPENDOOR;
	if (msgtype.compare("IOT_CLOSE_DOOR_REQUEST") == 0)
		return CLOSEDOOR;
	if (msgtype.compare("IOT_GET_DOOR_STATUS_REQUEST") == 0)
		return GETDOORSTATUS;
	if (msgtype.compare("IOT_GET_PAPER_STATUS_REQUEST") == 0)
		return GETPAPERSTATUS;
	if (msgtype.compare("IOT_GET_BOX_BATTERY_REQUEST") == 0)
		return GETBOXBATTERY;
	if (msgtype.compare("IOT_SET_BOXAP_REQUEST") == 0)
		return SETBOXAP;
	if (msgtype.compare("IOT_ENABLE_FRAME_ADSORB_REQUEST") == 0)
		return SETFRAMEADSORB_OPEN;
	if (msgtype.compare("IOT_DISENABLE_FRAME_ADSORB_REQUEST") == 0)
		return SETFRAMEADSORB_CLOSE;
	return -1;
}

iotServerManager::~iotServerManager()
{
	while (m_module_list.size()!=0)
	{
		delete m_module_list.begin()->second;
		m_module_list.erase(m_module_list.begin());
	}
}
