#include "robot_mcs_monitor.h"
#include "common_helper.h"
#include <mutex>

static robot_mcs_monitor *rmcs_o=NULL;
static std::mutex     g_rmcs_m;

robot_mcs_monitor::robot_mcs_monitor()
{
	m_client_monitorlist_l = common_mutex_create_threadlock();
	m_module_monitorlist_l = common_mutex_create_threadlock();
	m_maplist_l = common_mutex_create_threadlock();
}


robot_mcs_monitor * robot_mcs_monitor::getptr()
{
	if (rmcs_o == NULL)
	{
		g_rmcs_m.lock();
		if (rmcs_o == NULL)
			rmcs_o = new robot_mcs_monitor();
		g_rmcs_m.unlock();
	}
	return rmcs_o;
}

void robot_mcs_monitor::append_client_monitor(std::string module_name)
{
	LOCK_HELPER_AUTO_HANDLER(m_client_monitorlist_l);
	LOGI("append client infomation monitor -- %s", module_name.c_str());
	client_monitorlist.insert(module_name);
}

void robot_mcs_monitor::remove_client_monitor(std::string module_name)
{
	LOCK_HELPER_AUTO_HANDLER(m_client_monitorlist_l);
	LOGI("remove client infomation monitor -- %s", module_name.c_str());
	if (client_monitorlist.find(module_name) != client_monitorlist.end())
		client_monitorlist.erase(module_name);
}

std::set<std::string> robot_mcs_monitor::get_client_monitor()
{
	LOCK_HELPER_AUTO_HANDLER(m_client_monitorlist_l);
	return client_monitorlist;
}

void robot_mcs_monitor::append_module_monitor(std::string module_name)
{
	LOCK_HELPER_AUTO_HANDLER(m_module_monitorlist_l);
	LOGI("append module monitor -- %s", module_name.c_str());
	module_monitorlist.insert(module_name);

}

void robot_mcs_monitor::remove_module_monitor(std::string module_name)
{
	LOCK_HELPER_AUTO_HANDLER(m_module_monitorlist_l);
	LOGI("remove module monitor -- %s", module_name.c_str());
	if (module_monitorlist.find(module_name) != module_monitorlist.end())
		module_monitorlist.erase(module_name);
}


std::set<std::string> robot_mcs_monitor::get_module_monitor()
{
	LOCK_HELPER_AUTO_HANDLER(m_module_monitorlist_l);
	return client_monitorlist;
}

void robot_mcs_monitor::append_name_monitor(std::string module_name, std::string need_name, int type)
{
	LOCK_HELPER_AUTO_HANDLER(m_maplist_l);
	LOGI("[%s] append message monitor  %s", module_name.c_str(), need_name.c_str());
	map_list[need_name].insert(module_name);
	module_map_type[module_name][need_name] = type;
}

void robot_mcs_monitor::remove_name_monitor(std::string module_name, std::string need_name)
{
	LOCK_HELPER_AUTO_HANDLER(m_maplist_l);
	LOGI("[%s] remove message monitor  %s", module_name.c_str(), need_name.c_str());
	if (map_list.find(need_name) != map_list.end())
	{
		if (map_list[need_name].find(module_name) != map_list[need_name].end())
		{
			map_list[need_name].erase(module_name);
			if (map_list[need_name].size() == 0)
				map_list.erase(need_name);
		}

	}


	if (module_map_type.find(module_name) != module_map_type.end())
	{
		if (module_map_type[module_name].find(need_name) != module_map_type[module_name].end())
		{
			module_map_type[module_name].erase(need_name);
			if (module_map_type[module_name].size() == 0)
				module_map_type.erase(module_name);
		}

	}

}

std::set<std::string> robot_mcs_monitor::get_name_monitor_list(monitor_info &info)
{
	LOCK_HELPER_AUTO_HANDLER(m_maplist_l);
	std::set<std::string>  module_list,guid_list,message_list,result;
	if (!info.module_name.empty() && map_list.find(info.module_name) != map_list.end())
		module_list = map_list[info.module_name];

	if (!info.guid.empty() && map_list.find(info.guid) != map_list.end())
		guid_list = map_list[info.guid];

	if (!info.message_type.empty() && map_list.find(info.message_type) != map_list.end())
		message_list = map_list[info.message_type];

	for (auto i = guid_list.begin(); i != guid_list.end(); i++)
		module_list.insert(*i);

	for (auto i = message_list.begin(); i != message_list.end(); i++)
		module_list.insert(*i);

	for (auto it = module_list.begin(); it != module_list.end(); it++)
	{
		if (module_map_type.find(*it) != module_map_type.end())
		{
			if (module_map_type[*it].find(info.module_name) != module_map_type[*it].end() &&
				(module_map_type[*it][info.module_name] == 0 || module_map_type[*it][info.module_name] == info.module_type))
				result.insert(*it);
			if (module_map_type[*it].find(info.guid) != module_map_type[*it].end() &&
				(module_map_type[*it][info.guid] == 0 || module_map_type[*it][info.guid] == info.guid_type))
				result.insert(*it);
			if (module_map_type[*it].find(info.message_type) != module_map_type[*it].end())
				result.insert(*it);
		}
	}

	return result;

}


void robot_mcs_monitor::offline_name(std::string name)
{
	LOCK_HELPER_AUTO_HANDLER(m_maplist_l);
	if (map_list.find(name) == map_list.end())return;
	std::set<std::string > mlist = map_list[name];
	map_list.erase(name);
	for (auto i = mlist.begin(); i != mlist.end(); i++)
	{
		for (auto it = module_map_type.begin(); it !=module_map_type.end();it++)
		{
			if (it->first.compare(*i) == 0 && it->second.find(name) != it->second.end())
			{
				it->second.erase(name);
				break;
			}
		}
	}
}

robot_mcs_monitor::~robot_mcs_monitor()
{
	common_mutex_threadlock_release(m_maplist_l);
	common_mutex_threadlock_release(m_module_monitorlist_l);
	common_mutex_threadlock_release(m_client_monitorlist_l);
}
