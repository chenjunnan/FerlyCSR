#pragma once
#include <string>
#include <set>
#include <map>
#include "xCommon.h"
struct monitor_info
{
	std::string module_name;
	int			module_type;
	std::string guid;
	int			guid_type;
	std::string message_type;
	monitor_info(){
		module_type = 0;
		guid_type = 0;
	}
};


class robot_mcs_monitor
{
public:

	static robot_mcs_monitor *getptr();
	void append_client_monitor(std::string module_name);
	void remove_client_monitor(std::string module_name);
	std::set<std::string> get_client_monitor();


	void append_module_monitor(std::string module_name);
	void remove_module_monitor(std::string module_name);
	std::set<std::string> get_module_monitor();

	void append_name_monitor(std::string module_name,std::string need_name,int type);
	void remove_name_monitor(std::string module_name, std::string need_name);
	std::set<std::string> get_name_monitor_list(monitor_info &info);
	void offline_name(std::string);

private:
	robot_mcs_monitor();

	CHANDLE												m_maplist_l;
	std::map<std::string, std::set<std::string> >		map_list;			
	std::map<std::string, std::map<std::string,int>>	module_map_type;	

	CHANDLE											m_client_monitorlist_l;
	std::set<std::string>							client_monitorlist;

	CHANDLE											m_module_monitorlist_l;
	std::set<std::string>							module_monitorlist;

	~robot_mcs_monitor();
};

