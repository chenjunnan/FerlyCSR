#pragma once
#include <map>
#include "robot_module_wrap.h"
#include "xCommon.h"
#include "robot_module_thread.h"
#include "typeenum.h"

extern const char *MODULE_NAME;
class robot_module_manager
{
public:
	static robot_module_manager *get_Instance();
	virtual ~robot_module_manager();
	void Init();
	void Start();
	void Stop();
	void ShowModuleList();
	std::string GetModuleList();

	void ShutDownNotify();
	void getModuleVersion(ROBOTSOCKETMESSAGE &rsm);
	void getRobotSerial(ROBOTSOCKETMESSAGE &rsm);

	bool CallModuleProcess(std::string json_type, ROBOTSOCKETMESSAGE &rsm);
	bool CallModuleProcessbyName(std::string module, ROBOTSOCKETMESSAGE &rsm);
	bool CallModuleProcess(ROBOTSOCKETMESSAGE &rsm);

	bool MessageRoute(std::string msgtype, std::string &moudlename);

	bool AppendMessageTypeModule(std::string module, int msgtype);

	bool RemoveMessageTypeModule(std::string module);
	static std::string get_version();
	static void ModulePostMessage(const char *strModuleName, const char *strResult, unsigned long dResultLen,
		const char *desModuleName, const unsigned char *strExtdata, unsigned long  dExtlen);
private:
	bool check_modulename(std::string module_name);
	void AppendModule(std::string dll_path);
	std::string report_nohandler(std::string name);
	void SelfPostResult(ROBOTSOCKETMESSAGE &rsm, std::string &result);

	CHANDLE    m_module_l;
	std::map<std::string, robot_module_wrap*> module_interface;
	std::map<std::string, robot_module_thread*> module_thread;
	std::map<std::string, std::string>  messageType_toModule;
	std::vector<std::string>			module_config_namelist;

	CHANDLE    m_module_messagetype_l;
	std::map<int,std::string>            module_messagetype_list;
	//************************************************//
	std::map<std::string, std::string> module_version;
	std::string							m_serialid;
	int									m_error_serial;

	//************************************************//

	robot_module_manager();
	static robot_module_manager *m_instance;
	static CHANDLE    m_l;
};

