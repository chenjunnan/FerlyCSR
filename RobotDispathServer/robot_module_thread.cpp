#include "robot_main_app.h"
#include "robot_module_thread.h"
#include <assert.h>
#include "typeenum.h"
#include "json\writer.h"
#include "json\reader.h"
#include "xCommon.h"
#include "robot_client_manager.h"
#include "robot_module_manager.h"
#include "robot_mcs_monitor.h"



robot_module_thread::robot_module_thread(robot_module_wrap * module)
{
	assert(module!=NULL);
	m_module = module;
}

robot_module_thread::~robot_module_thread()
{
	if (state())quit();
}

void robot_module_thread::run()
{
	if (!m_module) return;
	m_module->Start();

	bool loop = true;
	std::string module_name = m_module->getModuleName();;
	while (loop)
	{
		Message msg = getq();
		switch (msg.msgType)
		{
		case MESSAGE_IN_TYPE:
		{
			ROBOTSOCKETMESSAGE *rbmsg = (ROBOTSOCKETMESSAGE *)msg.msgObject;

			if (rbmsg != NULL)
			{
				m_module->ProcessData(rbmsg->appID, rbmsg->messageType, rbmsg->fromer_client_guid.c_str(), rbmsg->json.c_str(), rbmsg->json.size());

				rbmsg->to_client_guid = module_name;
				monitor_info minfo;
				minfo.guid = rbmsg->fromer_client_guid; minfo.guid_type = 1; minfo.module_name = module_name; minfo.module_type = 1; minfo.message_type = rbmsg->task_name;
				std::set<std::string> ntf_module = robot_mcs_monitor::getptr()->get_name_monitor_list(minfo);
				if (ntf_module.size() != 0)
				{
					for (auto it = ntf_module.begin(); it != ntf_module.end(); it++)
						robot_module_manager::get_Instance()->CallModuleProcessbyName(*it, *rbmsg);
				}
				
			}

			SAFE_DELETE( rbmsg);
		}
			break;
		case MESSAGE_SLEF_TYPE:
		{
			ROBOTSOCKETMESSAGE *rbmsg = (ROBOTSOCKETMESSAGE *)msg.msgObject;
			if (rbmsg != NULL)
			{
				selfhandler(rbmsg, module_name);
				if (!rbmsg->response.empty())
					m_module->ProcessData(rbmsg->appID, 0, rbmsg->fromer_client_guid.c_str(), rbmsg->response.c_str(), rbmsg->response.size());
			}
				
			SAFE_DELETE(rbmsg);
		}
		break;
		case MESSAGE_MONITOR_TYPE:
		{
			ROBOTSOCKETMESSAGE *rbmsg = (ROBOTSOCKETMESSAGE *)msg.msgObject;
			if (rbmsg != NULL)
			{
				if (rbmsg->appID != ROBOT_MAIN_SERVER_ID)
				{
					Json::Value   json_object;
					Json::FastWriter  json_writer;
					json_object["msg_type"] = Json::Value("DISPATCH_MONITOR_MESSAGE_NOTIFY");
					json_object["content"] = Json::Value(rbmsg->json);
					json_object["appid"] = Json::Value(rbmsg->appID);
					json_object["messageType"] = Json::Value(rbmsg->messageType);
					json_object["from"] = Json::Value(rbmsg->fromer_client_guid);
					if (!rbmsg->to_client_guid.empty())
						json_object["to"] = Json::Value(rbmsg->to_client_guid);
					std::string report = json_writer.write(json_object);
					m_module->ProcessData(ROBOT_MAIN_SERVER_ID, 0, MODULE_NAME, report.c_str(), report.size());
				}
				else
				{
					m_module->ProcessData(rbmsg->appID, rbmsg->messageType, rbmsg->fromer_client_guid.c_str(), rbmsg->json.c_str(), rbmsg->json.size());
				}

			}
				
			SAFE_DELETE(rbmsg);
		}
			break;
		case MESSAGE_BINARY_TYPE:
		{
			ROBOTSOCKETMESSAGE *rbmsg = (ROBOTSOCKETMESSAGE *)msg.msgObject;
			if (rbmsg != NULL)
				m_module->ProcessData(rbmsg->appID, rbmsg->messageType, rbmsg->fromer_client_guid.c_str(), NULL, 0, (unsigned char *)rbmsg->recvdata.data(), rbmsg->recvdata.size());
			SAFE_DELETE(rbmsg);
		}
			break;
		case _EXIT_THREAD_:
			loop = false;
			break;
		default:;
		}
	}
	m_module->Stop();
}

void robot_module_thread::selfhandler(ROBOTSOCKETMESSAGE * rbmsg, std::string module_name)
{
	Json::Value   json_object;
	Json::Reader  json_reader(Json::Features::strictMode());
	std::string msgtype;
	bool b_json_read = json_reader.parse(rbmsg->json.c_str(), json_object);
	if (b_json_read)
	{
		std::string out;
		if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
		{
			msgtype = json_object["msg_type"].asString();
			common_trim(&msgtype);
		}
	}
	else
	{
		LOGE("module json format error:%s", rbmsg->json.c_str());
		return;
	}

	if (msgtype.compare("DISPATCH_GET_CLIENT_INFORMATION_REQUEST") == 0)
		rbmsg->response = robot_client_manager::getptr()->get_client_list();

	if (msgtype.compare("DISPATCH_GET_MODULE_INFORMATION_REQUEST") == 0)
		rbmsg->response = robot_module_manager::get_Instance()->GetModuleList();

	if (msgtype.compare("DISPATCH_REGISTER_MESSAGE_MONITOR_REQUEST") == 0 ||
		msgtype.compare("DISPATCH_UNREGISTER_MESSAGE_MONITOR_REQUEST") == 0)
	{
		std::string name, mode;
		if (json_object.isMember("name") && json_object["name"].isString())
		{
			name = json_object["name"].asString();
			common_trim(&name);
		}
		if (json_object.isMember("mode") && json_object["mode"].isString())
		{
			mode = json_object["mode"].asString();
			common_trim(&mode);
		}
		int type = 0;
		if (!mode.empty())
		{
			if (mode.compare("receive") == 0)type = 2;
			if (mode.compare("transmit") == 0)type = 1;
		}

		if (name.compare("client_service") == 0)
		{
			if (msgtype.compare("DISPATCH_REGISTER_MESSAGE_MONITOR_REQUEST") == 0)
				robot_mcs_monitor::getptr()->append_client_monitor(module_name);
			if (msgtype.compare("DISPATCH_UNREGISTER_MESSAGE_MONITOR_REQUEST") == 0)
				robot_mcs_monitor::getptr()->remove_client_monitor(module_name);
		}
		else if (name.compare("module_service") == 0)
		{

		}
		else
		{
			if (msgtype.compare("DISPATCH_REGISTER_MESSAGE_MONITOR_REQUEST") == 0)
				robot_mcs_monitor::getptr()->append_name_monitor(module_name, name, type);
			if (msgtype.compare("DISPATCH_UNREGISTER_MESSAGE_MONITOR_REQUEST") == 0)
				robot_mcs_monitor::getptr()->remove_name_monitor(module_name, name);
		}

	}

	if (msgtype.compare("DISPATCH_REGISTER_MESSAGETYPE_REQUEST") == 0)
	{
		if (json_object.isMember("messagetype") && json_object["messagetype"].isInt())
		{
			int msgtype= json_object["messagetype"].asInt();
			bool ret= robot_module_manager::get_Instance()->AppendMessageTypeModule(module_name, msgtype);
			Json::Value   json_object;
			Json::FastWriter json_writer;
			json_object["msg_type"] = Json::Value("DISPATCH_REGISTER_MESSAGETYPE_RESPONSE");
			json_object["error"] = Json::Value(ret?0:1);		
			rbmsg->response = json_writer.write(json_object);
		}
	}
	if (msgtype.compare("DISPATCH_SHUTDOWN_REQUEST") == 0)
	{
		std::string module_namei = m_module->getModuleName();;
		LOGW("%s will close service", module_namei.c_str());
		robot_main_app::getinstance()->main_close();
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("DISPATCH_SHUTDOWN_RESPONSE");
		json_object["error"] = Json::Value(0);
		rbmsg->response = json_writer.write(json_object);
	}
}
