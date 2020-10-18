#include "robot_client_manager.h"
#include "xCommon.h"
#include "common_helper.h"
#include "typeenum.h"
#include "json\writer.h"
#include "robot_mcs_monitor.h"
#include "robot_module_manager.h"


robot_client_manager::robot_client_manager()
{
	m_ml = common_mutex_create_threadlock();

	Json::Value   json_object;
	Json::FastWriter json_writer;
	json_object["msg_type"] = Json::Value("DISPATCH_CLIENT_CONNECT_NOTIFY");
	json_object["appid"] = Json::Value("%d");
	json_object["guid"] = Json::Value("%s");

	client_connect_strformat=json_writer.write(json_object); 
	 
	json_object.clear();
	json_object["msg_type"] = Json::Value("DISPATCH_CLIENT_DISCONNECT_NOTIFY");
	json_object["guid"] = Json::Value("%s");
	client_disconnect_strformat = json_writer.write(json_object); 
}

 

robot_client_manager * robot_client_manager::getptr()
{
	static robot_client_manager _o;
	return &_o;

}



void robot_client_manager::add_client(std::string guid, int appid)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);

	if (m_clientlist[appid].socket_list.find(guid) == m_clientlist[appid].socket_list.end())
	{
		m_clientlist[appid].socket_list.insert(guid);
		m_guid_appid[guid] = appid;
		if (appid == ROBOT_EXPRESSION_APP)
		{
			LOGI("expression client connectting");
			m_clientlist[ROBOT_EXPRESSION_APP].navi_ntf = false;
			m_clientlist[ROBOT_EXPRESSION_APP].voice_ntf = false;
			m_clientlist[ROBOT_EXPRESSION_APP].face_ntf = false;
		}
		std::set<std::string> ntf_module = robot_mcs_monitor::getptr()->get_client_monitor();
		if (ntf_module.size() != 0)
		{
			ROBOTSOCKETMESSAGE rns;
			char buffer[1024] = { 0 };
			sprintf(buffer, client_connect_strformat.c_str(), appid,guid.c_str());
			rns.json = buffer;
			rns.appID = ROBOT_MAIN_SERVER_ID;
			rns.fromer_client_guid = MODULE_NAME;
			for (auto it = ntf_module.begin(); it != ntf_module.end(); it++)
			{
				robot_module_manager::get_Instance()->CallModuleProcessbyName(*it, rns);
			}
		}

	}

}



void robot_client_manager::change_client_ntf(int appid, int type, int value)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml)
	auto it = m_clientlist.find(appid);
	if (it == m_clientlist.end()) return;

	switch (type)
	{
	case 1:
		m_clientlist[appid].navi_ntf = value;
		break;
	case 2:
		m_clientlist[appid].voice_ntf = value;
		break;
	case 3:
		m_clientlist[appid].face_ntf = value;
		break;
	}

}

std::vector<std::string> robot_client_manager::get_ntf_list(int type)
{
	std::vector<std::string> list;
	auto it = m_clientlist.begin();
	for (; it != m_clientlist.end(); it++)
	{
		switch (type)
		{
		case 1:
			if (!it->second.navi_ntf)
				continue;
			break;
		case 2:
			if (!it->second.voice_ntf)
				continue;
			break;
		case 3:
			if (!it->second.face_ntf)
				continue;
			break;
		}
		auto it_array = it->second.socket_list.begin();
		for (; it_array != it->second.socket_list.end(); it_array++)
			list.push_back(*it_array);

	}
	return list;
}

int robot_client_manager::get_appid(std::string guid)
{
	if (m_guid_appid.find(guid) != m_guid_appid.end())
		return m_guid_appid[guid];
	return 0;
}

bool robot_client_manager::get_guid(int appid,std::string &guid)
{
	guid.clear();
	if (m_clientlist.find(appid) == m_clientlist.end() || m_clientlist[appid].socket_list.size()<1)
		return false;
	guid = *m_clientlist[appid].socket_list.begin();
	return true;
}


void robot_client_manager::remove_client(std::string guid)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	auto it = m_clientlist.begin();
	for (; it != m_clientlist.end();it++)
	{
		auto it_array = it->second.socket_list.begin();
		for (; it_array != it->second.socket_list.end(); it_array++)
		{
			if (it_array->compare(guid) == 0)
			{
				if (it->first == ROBOT_EXPRESSION_APP)
					LOGI("expression client disconnectting");
				it->second.socket_list.erase(it_array);
				break;
			}
				
		}
		if (it->second.socket_list.size() == 0)
		{
			 
			m_clientlist.erase(it);
			break;
		}

	}
	if (m_guid_appid.find(guid) != m_guid_appid.end())
		m_guid_appid.erase(m_guid_appid.find(guid));

	std::set<std::string> ntf_module = robot_mcs_monitor::getptr()->get_client_monitor();
	if (ntf_module.size() != 0)
	{
		ROBOTSOCKETMESSAGE rns;
		char buffer[1024] = { 0 };
		sprintf(buffer, client_disconnect_strformat.c_str(), guid.c_str());
		rns.json = buffer;
		rns.appID = ROBOT_MAIN_SERVER_ID;
		rns.fromer_client_guid = MODULE_NAME;
		for (auto it = ntf_module.begin(); it != ntf_module.end(); it++)
		{
			robot_module_manager::get_Instance()->CallModuleProcessbyName(*it, rns);
		}
	}
	robot_mcs_monitor::getptr()->offline_name(guid);
}

bool robot_client_manager::get_client_state(std::string guid, int type)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	auto it = m_clientlist.begin();
	for (; it != m_clientlist.end(); it++)
	{
		auto it_array = it->second.socket_list.begin();
		for (; it_array != it->second.socket_list.end(); it_array++)
		{
			if (it_array->compare(guid) == 0)
				goto here ;
		}
	}
	here:
	if (it == m_clientlist.end()) return false;

	switch (type)
	{
	case 1:return it->second.navi_ntf;
	case 2:return it->second.voice_ntf;
	case 3:return it->second.face_ntf;
	}
	return false;
	
}

std::string robot_client_manager::get_client_list()
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	Json::Value   json_object;
	Json::FastWriter json_writer;
	json_object["msg_type"] = Json::Value("DISPATCH_GET_CLIENT_INFORMATION_RESPONSE");
	for (auto it = m_guid_appid.begin(); it != m_guid_appid.end(); it++)
	{
		Json::Value   json_client;
		json_client["appid"] = it->second;
		json_client["guid"] = it->first;
		json_object["client_list"].append(json_client);
	}
	return json_writer.write(json_object);
}

robot_client_manager::~robot_client_manager()
{
	common_mutex_threadlock_release(m_ml);
}
