#pragma once
#include <string>
#include <map>
#include <vector>
#include <set>
#include "xCommon.h"

struct client_ntf
{
	std::set<std::string > socket_list;
	bool	navi_ntf;
	bool	voice_ntf;
	bool	face_ntf;
	client_ntf()
	{
		navi_ntf = true;
		voice_ntf = true;
		face_ntf = true;
	}
};
class robot_client_manager
{
public:
	static robot_client_manager *getptr();
	void add_client(std::string guid, int appid);
	void remove_client(std::string);
	bool exist_client(std::string guid);
	void change_client_ntf(int appid,int type,int value);// type 0 navi,1 voice, 2 face  4 voice_result
	std::vector<std::string> get_ntf_list(int type);
	int  get_appid(std::string guid);
	bool get_guid(int appid, std::string &guid);
	bool get_client_state(std::string, int type);
	std::string get_client_list();

	void append_client_monitor(std::string);
	void remove_client_monitor(std::string);

	~robot_client_manager();
protected:
	robot_client_manager();
	std::map<int, client_ntf>   m_clientlist;
	std::map<std::string, int>  m_guid_appid;

	std::vector<std::string> monitor_list;

	CHANDLE							m_ml;

	std::string    client_connect_strformat;
	std::string    client_disconnect_strformat;
};

