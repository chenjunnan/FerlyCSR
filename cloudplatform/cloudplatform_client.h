#pragma once
#include "cloud.h"
#include "cloud_mq_server.h"
#include <list>
#include <map>
#include "json/json.h"


struct TaskList
{
	TaskList(int id, std::string s)
	{
		taskid = id;
		taskinfo = s;
		finish = false;
	}
	int taskid;
	bool finish;
	std::string taskinfo;
};

struct remote_request
{
	std::string msgtype;
	std::string callbackurl;
	std::string httprequest_type ;
	std::string resultdata;
	unsigned long long time = 0;;
	bool remove_flag = false;
};

class cloudplatform_client :
	public cloud_mq_interface,
	public Thread_helper
{
public:
	cloudplatform_client();
	int Init(std::string ip, std::string port, std::string user, std::string password);
	int StartRun();
	void Stop();

	int HandleMessage(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata = NULL,
		unsigned long dExtlen = 0);

	void GetRId(Json::Value &json_object, const char * strFrom);

	

	virtual void MessageIn(std::vector<char> &data);
	virtual void ThreadQuit();
	virtual ~cloudplatform_client();

	virtual void run();
private:
	void clean_delay();
	std::string repost(int error);
	std::string repost_MQ(int error);
	void test();
	void check_connect_cloud();
	void tasklink_handler(Json::Value &json_object, const char * strFrom);
	void MQmessage_handler(Json::Value &json_object, const char * strFrom);
	void RPC_MessageTimeout(remote_request &data);
	int RPC_MessageHttp(remote_request &data);
	void RPC_handler(Json::Value   &json_object, std::string &msgtype);
	cloud_mq_server m_mq;
	std::string m_ip;
	std::string m_port;
	std::string m_user;
	std::string m_password;
	std::string m_robot_id;
	std::string m_cloud_url;
	std::string m_cloud_CBurl;
	std::string m_queue_name;
	std::string m_url_head;
	int m_task_id;
	CHANDLE		m_guard_data;
	CHANDLE     m_http;
	CHANDLE     m_http_user;
	bool isOffline;
	std::list<TaskList> m_offline_task;
	std::map<std::string, std::list<remote_request> > m_map_remote;
	std::list<remote_request>						  m_error_retry_list;
};

