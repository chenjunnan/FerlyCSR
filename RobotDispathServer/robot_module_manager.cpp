#include "robot_module_manager.h"
#include <set>
#include "typeenum.h"
#include "robot_message_handler.h"
#include "json\writer.h"
#include "robot_mcs_monitor.h"
#include "json\reader.h"
#include <sys\stat.h>
#include <fstream>
#include <iostream>
#include <string>
#include <iosfwd>

robot_module_manager * robot_module_manager::m_instance = NULL;
CHANDLE robot_module_manager::m_l = common_mutex_create_threadlock();

const char *MODULE_NAME = "dispath_service";
const char *UNKNOWN_NAME = "unname_dispath";
const char *CONFIG_FILE = "module.config";


robot_module_manager::robot_module_manager()
{
	
	m_error_serial = 0;
	m_module_messagetype_l = common_mutex_create_threadlock();
}

void robot_module_manager::AppendModule(std::string dll_path)
{
	robot_module_wrap *module_ptr = new robot_module_wrap;
	bool ok = false;
	std::string Mname;
	do
	{
		if (module_ptr->Load(dll_path))
		{
			Mname = module_ptr->getModuleName();

			if (Mname.empty())
			{
				LOGE("Module %s name is empty.", dll_path.c_str());
				break;
			}
			else
			{
				auto it = module_interface.find(Mname);
				if (it != module_interface.end())
				{
					LOGE("The module already exist (%s) whose name is %s(%s).", it->first.c_str(), Mname.c_str(), dll_path.c_str());
					break;
				}

				std::vector<std::string> funlist;
				funlist = module_ptr->getHandleList();

				if (funlist.size() == 0)
				{
					LOGW("The module name %s's handle list is empty .", Mname.c_str());
					ok = true;
					break;
				}
				else
				{
					int i = 0;
					for (; i < funlist.size(); i++)
					{
						auto it = messageType_toModule.find(funlist[i]);
						if (it != messageType_toModule.end())
						{
							LOGE("Same Message Type :%s -%s\t %s -%s", it->first.c_str(), it->second.c_str(), Mname.c_str(), funlist[i].c_str());
							break;
						}
					}

					if (i == funlist.size())
					{
						i = 0;
						for (; i < funlist.size(); i++)
						{
							messageType_toModule[funlist[i]] = Mname;
							LOGI("Append message table:\n%-10s\t%50s", Mname.c_str(), funlist[i].c_str());
						}

						ok = true;

					}


				}
			}
		}
		else
		{
			break;
		}

	} while (0);

	if (!ok)
		delete module_ptr;
	else
	{
		module_config_namelist.push_back(Mname);
		module_interface[Mname] = module_ptr;
	}

}

void robot_module_manager::Init()
{
	std::string out;
	int result = 0;
	struct stat statBuf;

	char exeFullPath[MAX_PATH]; // MAX_PATH在WINDEF.h中定义了，等于260  
	memset(exeFullPath, 0, MAX_PATH);

	GetModuleFileName(NULL, exeFullPath, MAX_PATH);
	std::string exepath = exeFullPath;
	if (exepath.rfind("\\") != -1)
	{
		exepath = exepath.substr(0, exepath.rfind("\\"));
		common_fix_path(&exepath);
		exepath.append(CONFIG_FILE);
	}
	else
		exepath = CONFIG_FILE;

	m_error_serial = common_get_robot_serialID(&m_serialid);

	if (m_error_serial == 0)
	{
		LOGI("Robot serial id:%s", m_serialid.c_str());
	}
	else
	{
		LOGW("check robot serial error:%d", m_error_serial);
	}


	result = stat(exepath.c_str(), &statBuf);
	if (result < 0)
	{
		LOGE("read module file %s failed ,is not exist !", exepath.c_str())
		return  ;
	}

	std::ifstream ifi;
	ifi.open(exepath.c_str(), std::ios::in);

	if (ifi.fail())
	{
		LOGE("Unable to open config file %s !", exepath.c_str())
		return  ;
	}	
	std::set < std::string > modulepathlist;
	std::string readInfo;
	while (std::getline(ifi, readInfo))
	{
		if (!readInfo.empty())
		{
			common_trim(&readInfo);

			if (readInfo[0] == ';')
			{
				continue;
			}
			std::string dll_path = readInfo;
			if (dll_path.find(".dll") != -1)
			{
				if (modulepathlist.find(dll_path) == modulepathlist.end())
				{
					AppendModule(dll_path);
					modulepathlist.insert(dll_path);
				}				
				else
					LOGW("same module path %s", dll_path.c_str());
			}
			
			
		}
	}
	ifi.close();

	auto ptr = module_config_namelist.begin();
	for (; ptr != module_config_namelist.end(); ptr++)
	{
		if (module_interface.find(*ptr) != module_interface.end())
		{
			if (module_interface[*ptr]->Init())
				module_thread[*ptr] = new robot_module_thread(module_interface[*ptr]);
			else
			{
				LOGW("module %s init failed.", ptr->c_str());
			}
		}
	}
	for (auto it = module_thread.begin(); it != module_thread.end(); it++)
	{
		if (module_interface.find(it->first) != module_interface.end())
			module_version[it->first]=module_interface[it->first]->getModuleVersion();
	}
	LOGI("%s totle load %d modules.", MODULE_NAME, module_thread.size());
}

void robot_module_manager::Start()
{
	auto ptr = module_config_namelist.begin();
	for (; ptr != module_config_namelist.end(); ptr++)
	{
		if ( module_thread.find(*ptr) != module_thread.end())
			module_thread[*ptr]->start();
	}
}

void robot_module_manager::Stop()
{
	auto ptr = module_config_namelist.rbegin();
	for (; ptr != module_config_namelist.rend(); ptr++)
	{
		if ( module_thread.find(*ptr) != module_thread.end())
			module_thread[*ptr]->quit();
	}
}

void robot_module_manager::ShowModuleList()
{
	printf("==============================moudle list=====================================\n");
	for (auto it = module_thread.begin(); it != module_thread.end(); it++)
		printf("%s\n",it->first.c_str());
	printf("==============================moudle list=====================================\n");
}

std::string robot_module_manager::GetModuleList()
{
	Json::Value   json_object;
	Json::FastWriter json_writer;
	json_object["msg_type"] = Json::Value("DISPATCH_GET_MODULE_INFORMATION_RESPONSE");
	for (auto it = module_thread.begin(); it != module_thread.end(); it++)
	{
		Json::Value   json_module;
		json_module["name"] = Json::Value(it->first);
		json_object["module_list"].append(json_module);
	}
	return json_writer.write(json_object);
}

void robot_module_manager::ShutDownNotify()
{
	Message msg;
	ROBOTSOCKETMESSAGE *rep = new ROBOTSOCKETMESSAGE;
	Json::Value   json_object;
	Json::FastWriter json_writer;
	json_object["msg_type"] = Json::Value("ROBOT_SHUTDOWN_NOTIFY");
	rep->task_name = "ROBOT_SHUTDOWN_NOTIFY";
	rep->json = json_writer.write(json_object);;
	msg.msgType = ROBOT_RESULT_MESSAHE_ALL;
	msg.msgObject = rep;
	robot_message_handler::getptr()->pushq(msg);
}

void  robot_module_manager::SelfPostResult(ROBOTSOCKETMESSAGE &rsm,std::string &result)
{
	Message msg;
	ROBOTSOCKETMESSAGE *rep = new ROBOTSOCKETMESSAGE;
	rep->appID = rsm.appID; rep->messageType = rsm.messageType;
	rep->to_client_guid = rsm.fromer_client_guid;
	rep->task_name = rsm.task_name;
	rep->json = result;
	msg.msgType = ROBOT_RESULT_MESSAHE;
	msg.msgObject = rep;
	robot_message_handler::getptr()->pushq(msg);
}

void robot_module_manager::getModuleVersion(ROBOTSOCKETMESSAGE &rsm)
{

	std::string mainver = get_version();
	Json::Value   json_object;
	Json::FastWriter json_writer;
	json_object["msg_type"] = Json::Value("ROBOT_VERSION_RESPONSE");
	for (auto it = module_version.begin(); it != module_version.end(); it++)
	{
		Json::Value   json_module;
		json_module["name"] = Json::Value(it->first);
		json_module["version"] = Json::Value(it->second);
		json_object["modulelist"].append(json_module);
	}
	
	json_object["robotversion"] = Json::Value(mainver.c_str());
	std::string module_verlist = json_writer.write(json_object);
	rsm.task_name = "ROBOT_VERSION_REQUEST";
	SelfPostResult(rsm, module_verlist);
}

void robot_module_manager::getRobotSerial(ROBOTSOCKETMESSAGE &rsm)
{
	Json::Value   json_object;
	Json::FastWriter json_writer;
	json_object["msg_type"] = Json::Value("ROBOT_SERIALID_RESPONSE");
	if (m_error_serial==0)
		json_object["robotserial"] = Json::Value(m_serialid.c_str());
	json_object["error"] = Json::Value(m_error_serial);
	std::string robot_serial = json_writer.write(json_object);
	rsm.task_name = "ROBOT_SERIALID_REQUEST";
	SelfPostResult(rsm, robot_serial);
}

std::string robot_module_manager::report_nohandler(std::string name)
{
	std::string str;
	Json::Value   json_object;
	Json::FastWriter json_writer;
	json_object["msg_type"] = Json::Value("ROBOT_ERROR_RESPONSE");
	str.append("UNKNOW MESSAGE");
	json_object["msg"] = Json::Value(name.c_str());
	json_object["error_msg"] = Json::Value(str.c_str());
	return json_writer.write(json_object);
}

bool robot_module_manager::MessageRoute(std::string msgtype, std::string &moudlename)
{
	if (messageType_toModule.find(msgtype) != messageType_toModule.end()&&
		module_thread.find(messageType_toModule[msgtype]) != module_thread.end())
	{
		
		moudlename = messageType_toModule[msgtype];
		return true;
	}

	return false;
}

bool  robot_module_manager::CallModuleProcess(std::string json_type, ROBOTSOCKETMESSAGE &rsm)
{
	Message msg;
	msg.msgType = MESSAGE_IN_TYPE;
	Thread_helper *th = NULL;
	std::string nodename;
	if (messageType_toModule.find(json_type) != messageType_toModule.end() &&
		module_thread.find(messageType_toModule[json_type]) != module_thread.end())
		th = module_thread[messageType_toModule[json_type]];

	if (th == NULL)
	{
		SelfPostResult(rsm, report_nohandler(json_type));
		LOGE("cant find <%s> handler ,maybe not register funtion or no module.", json_type.c_str());
		return false;
	}

	ROBOTSOCKETMESSAGE *post_object = new ROBOTSOCKETMESSAGE;
	*post_object = rsm;
	post_object->task_name = json_type;
	post_object->to_client_guid = nodename;
	msg.msgObject = post_object;
	th->pushq(msg);
	return true;
}

bool robot_module_manager::CallModuleProcessbyName(std::string module, ROBOTSOCKETMESSAGE &rsm)
{
	Message msg;
	ROBOTSOCKETMESSAGE *post_object = new ROBOTSOCKETMESSAGE;
	*post_object = rsm;
	msg.msgType = MESSAGE_MONITOR_TYPE;
	msg.msgObject = post_object;
	if (module_thread.find(module) != module_thread.end())
	{
		module_thread[module]->pushq(msg);
		return true;
	}
	SAFE_DELETE(post_object);
	LOGE("cant find <%s> module.", module.c_str());
	return false;
}

bool robot_module_manager::CallModuleProcess(ROBOTSOCKETMESSAGE &rsm)
{
	LOCK_HELPER_AUTO_HANDLER(m_module_messagetype_l);
	if (module_messagetype_list.find(rsm.messageType) == module_messagetype_list.end())
	{
		LOGE("cant find <%d> register.", rsm.messageType);
		return false;;
	}
	Message msg;
	msg.msgType = MESSAGE_BINARY_TYPE;
	ROBOTSOCKETMESSAGE *post_object = new ROBOTSOCKETMESSAGE;
	*post_object = rsm;
	msg.msgObject = post_object;
	if (module_thread.find(module_messagetype_list[rsm.messageType]) != module_thread.end())
	{		
		module_thread[module_messagetype_list[rsm.messageType]]->pushq(msg);
		return true;

	}
	SAFE_DELETE(post_object);
	LOGE("cant find <%s> module.", module_messagetype_list[rsm.messageType].c_str());
	return false;
}

bool robot_module_manager::AppendMessageTypeModule(std::string module, int msgtype)
{
	LOCK_HELPER_AUTO_HANDLER(m_module_messagetype_l);
	if (module_messagetype_list.find(msgtype) != module_messagetype_list.end())
	{
		LOGE("message type <%d> has register by %s.", msgtype, module_messagetype_list[msgtype].c_str());
		return false;
	}
	module_messagetype_list[msgtype] = module;
	LOGI("message type <%d>  register <%s>.", msgtype,module.c_str());
	return true;
}

bool robot_module_manager::RemoveMessageTypeModule(std::string module)
{
	LOCK_HELPER_AUTO_HANDLER(m_module_messagetype_l);
	for (auto it = module_messagetype_list.begin(); it != module_messagetype_list.end();)
	{
		if (it->second.compare(module) == 0)
		{
			LOGI("message type <%d>  register <%s>.", it->first, module.c_str());
			it = module_messagetype_list.erase(it);
		}
		else
			it++;
	}
	return true;
}


bool robot_module_manager::check_modulename(std::string module_name)
{
	return module_thread.find(module_name) != module_thread.end();//mutex
}


void robot_module_manager::ModulePostMessage(const char *strModuleName, const char *strResult, unsigned long dResultLen, const char *desModuleName, const unsigned char *strExtdata, unsigned long dExtlen)
{
	ROBOTSOCKETMESSAGE *post_object = new ROBOTSOCKETMESSAGE;
	Thread_helper * thread = NULL;
	Message msg;
	msg.msgObject = post_object;
	std::string moudle_out;
	monitor_info minfo;

	post_object->fromer_client_guid = strModuleName;
	minfo.module_name = strModuleName;
	minfo.module_type = 2;
	std::string msgtype;

	if (strExtdata == NULL)
	{
		Json::Value   json_object;
		Json::Reader  json_reader(Json::Features::strictMode());
		post_object->json = strResult;//only text
		if (json_reader.parse(post_object->json.c_str(), json_object))
		{
			if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
			{
				msgtype = json_object["msg_type"].asString();
				common_trim(&msgtype);
				post_object->task_name = msgtype;
				minfo.message_type = msgtype;
			}
		}
	}
	else
	{
		post_object->messageType = dResultLen;
		post_object->senddata.assign(strExtdata, strExtdata + dExtlen);
	}
		


	if (desModuleName == NULL)
	{
		msg.msgType = ROBOT_RESULT_MESSAHE_ALL;
		post_object->to_client_guid = "MESSAGE_NOTIFY";
		thread=robot_message_handler::getptr();//gei all client		
	}else if (robot_module_manager::get_Instance()->check_modulename(desModuleName)||
		(strcmp(desModuleName, UNKNOWN_NAME) == 0 && !msgtype.empty() && robot_module_manager::get_Instance()->MessageRoute(msgtype, moudle_out)))
	{
		std::string modulename;
		if (strcmp(desModuleName, UNKNOWN_NAME) == 0)
			modulename = moudle_out;
		else
			modulename = desModuleName;

		msg.msgType = MESSAGE_IN_TYPE;
		post_object->to_client_guid = modulename;

		thread = robot_module_manager::get_Instance()->module_thread[modulename];//gei other module
	}
	else if (strcmp(desModuleName, MODULE_NAME) == 0 && robot_module_manager::get_Instance()->check_modulename(strModuleName))
	{
		msg.msgType = MESSAGE_SLEF_TYPE;
		post_object->appID = ROBOT_MAIN_SERVER_ID;
		post_object->fromer_client_guid = MODULE_NAME;
		post_object->to_client_guid = strModuleName;
		thread = robot_module_manager::get_Instance()->module_thread[strModuleName];//gei root
	}else
	{
		if (strlen(desModuleName) > 36)
		{
			minfo.guid = desModuleName; minfo.guid_type = 2;
			post_object->to_client_guid = desModuleName;
			msg.msgType = ROBOT_RESULT_MESSAHE;	
			thread = robot_message_handler::getptr();//gei client
		}
		else
		{
			LOGE("%s unknow desModuleName [%s] mseeage :%s ", strModuleName, desModuleName, strResult);
		}

	}

	std::set<std::string> ntf_module = robot_mcs_monitor::getptr()->get_name_monitor_list(minfo);

	ROBOTSOCKETMESSAGE temp_rsm;

	if (ntf_module.size() != 0)
		temp_rsm = *post_object;


	if (thread)
	{
		thread->pushq(msg);
	}
	else
	{
		SAFE_DELETE(post_object);
	}
	

	if (ntf_module.size() != 0)
	{
		for (auto it = ntf_module.begin(); it != ntf_module.end(); it++)
			robot_module_manager::get_Instance()->CallModuleProcessbyName(*it, temp_rsm);
	}
}

robot_module_manager * robot_module_manager::get_Instance()
{
	if (m_instance == NULL)
	{
		common_mutex_threadlock_lock(m_l);
		if (m_instance == NULL)
		{
			m_instance = new robot_module_manager;
		}
			
		common_mutex_threadlock_unlock(m_l);
	}
	return m_instance;
}

robot_module_manager::~robot_module_manager()
{
	common_mutex_threadlock_release(m_module_messagetype_l);
}


std::string  robot_module_manager::get_version()
{
	SYSTEMTIME st;
	char szDate[1024];
	ZeroMemory(szDate, sizeof(szDate));
	ZeroMemory(&st, sizeof(SYSTEMTIME));
	sprintf_s(szDate, 1024, "%s", __DATE__);

	st.wYear = atoi(szDate + 7);

	st.wDay = szDate[5] - '0';
	if (szDate[4] != ' ')
	{
		st.wDay += (szDate[4] - '0') * 10;
	}

	switch (szDate[0])
	{
	case 'A':
		if (szDate[1] == 'p')
		{
			st.wMonth = 4;
		}
		else
		{
			st.wMonth = 8;
		}
		break;

	case 'D':
		st.wMonth = 12;
		break;

	case 'F':
		st.wMonth = 2;
		break;

	case 'J':
		if (szDate[1] == 'a')
		{
			st.wMonth = 1;
		}
		else
		{
			if (szDate[2] == 'n')
			{
				st.wMonth = 6;
			}
			else
			{
				st.wMonth = 7;
			}
		}
		break;

	case 'M':
		if (szDate[2] == 'r')
		{
			st.wMonth = 3;
		}
		else
		{
			st.wMonth = 5;
		}
		break;

	case 'N':
		st.wMonth = 11;
		break;

	case 'O':
		st.wMonth = 10;
		break;

	case 'S':
		st.wMonth = 9;
		break;
	}

	ZeroMemory(szDate, sizeof(szDate));
	sprintf_s(szDate, 1024, "%s", __TIME__);

	st.wHour = (szDate[0] - '0') * 10 + (szDate[1] - '0');
	st.wMinute = (szDate[3] - '0') * 10 + (szDate[4] - '0');
	st.wSecond = (szDate[6] - '0') * 10 + (szDate[7] - '0');
	sprintf_s(szDate, 1024, "%s.%02d.%04d.%d%02d%02d%02d%02d%02d", VERSION_MODULE, MODULE_CLASS, SUBVERSION, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	std::string temp = szDate;
	return temp;
}

