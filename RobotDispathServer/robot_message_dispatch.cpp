#include "robot_message_dispatch.h"
#include "ServerInterface.h"
#include "json\json.h"
#include "robot_client_manager.h"
#include "robot_module_manager.h"
#include "robot_mcs_monitor.h"
#include "robot_message_handler.h"


HWND hwnd;
#define  IFLY_VOICE_SERVICE  0



robot_message_dispatch::robot_message_dispatch()
{
	m_server_NodeID = 0;
}


robot_message_dispatch * robot_message_dispatch::getptr()
{
	static robot_message_dispatch _rmm;
	return &_rmm;
}

robot_message_dispatch::~robot_message_dispatch()
{
	if (state())quit();
}


void robot_message_dispatch::remove(std::string clientguid)
{
	pushq(10, clientguid);
}

int robot_message_dispatch::net_to_host_int(std::vector< char> &buffer,int b_pos)
{
	int ret = 0;
	ret = (int)(((buffer[b_pos  ] << 24) & 0xff000000)|
				((buffer[b_pos+1] << 16) & 0x00ff0000)|
				((buffer[b_pos+2] << 8)  & 0x0000ff00)|
				( buffer[b_pos+3]		 & 0x000000ff));
/*	ret=ntohl(ret);*/
	return ret;
}

short robot_message_dispatch::net_to_host_short(std::vector< char> &buffer, int b_pos)
{
	short ret = 0;
	ret = (short)(((buffer[b_pos    ] << 8) & 0xff00)|
				   (buffer[b_pos + 1]		& 0x00ff));
	/*ret = ntohs(ret);*/
	return ret;
}




int robot_message_dispatch::check_and_get_package_info(std::string guid, ROBOTSOCKETMESSAGE &rsm)
{
	auto it_findf = std::find(m_cache_netdata[guid].buffer.begin(), m_cache_netdata[guid].buffer.end(), 0x03);
	auto it_findl = std::find(m_cache_netdata[guid].buffer.begin(), m_cache_netdata[guid].buffer.end(), 0x16);

	if (m_cache_netdata[guid].buffer.size() < 28)
		return 1;//data not ready;

	if (it_findf == m_cache_netdata[guid].buffer.end())
	{
		m_cache_netdata[guid].buffer.clear();
		return -1;//error data
	}

	if (it_findl == m_cache_netdata[guid].buffer.end())
	{
		if (it_findf == (m_cache_netdata[guid].buffer.end() - 1))
		{
			m_cache_netdata[guid].buffer.erase(m_cache_netdata[guid].buffer.begin(), it_findf);
			return 1;//wait data
		}
		else
		{
			m_cache_netdata[guid].buffer.clear();
			return -1;//error data
		}

	}

	if (it_findl != (it_findf + 1))
	{
		m_cache_netdata[guid].buffer.clear();
		return -1;//error data
	}
	if (it_findf != m_cache_netdata[guid].buffer.begin())
		m_cache_netdata[guid].buffer.erase(m_cache_netdata[guid].buffer.begin(), it_findf);

	if (m_cache_netdata[guid].buffer.size() < 28)
		return 1;//data not ready;

	rsm.sendNode = net_to_host_int(m_cache_netdata[guid].buffer, 4);
	rsm.recvNode = net_to_host_int(m_cache_netdata[guid].buffer, 8);
	rsm.appID    = net_to_host_int(m_cache_netdata[guid].buffer, 12);
	rsm.serverID = net_to_host_int(m_cache_netdata[guid].buffer, 16);
	rsm.eventID  = net_to_host_short(m_cache_netdata[guid].buffer, 20);
	rsm.messageType = net_to_host_short(m_cache_netdata[guid].buffer, 22);
	m_cache_netdata[guid].totle_bytes = net_to_host_int(m_cache_netdata[guid].buffer, 24);
	if (m_cache_netdata[guid].buffer.size()<(m_cache_netdata[guid].totle_bytes+28))
		return 1;

	it_findl = it_findf+28;
	//it_findf = std::find(it_findl, m_cache_netdata[guid].buffer.end(), 0x03);
	it_findf = it_findl + m_cache_netdata[guid].totle_bytes;;

	rsm.json.clear();
	rsm.recvdata.clear();
	if (rsm.messageType == 0)
	{
		rsm.json.insert(rsm.json.begin(), it_findl, it_findf);
	}
	else
	{
		rsm.recvdata.assign(it_findl, it_findf);
	}


	m_cache_netdata[guid].buffer.erase(m_cache_netdata[guid].buffer.begin(), it_findf);

	rsm.fromer_client_guid = guid;

	//common_get_guid(&rsm.package_guid);

	robot_client_manager::getptr()->add_client(guid, rsm.appID);
	return 0;

}

int robot_message_dispatch::need_route_message(ROBOTSOCKETMESSAGE &rsm)
{
	if (rsm.recvNode == 0 && rsm.sendNode == 0 && rsm.serverID == ROBOT_MAIN_SERVER_ID)
		return 0;
	else if (rsm.recvNode == m_server_NodeID&& rsm.serverID == ROBOT_MAIN_SERVER_ID)
		return 0;
	return 1;
}

int robot_message_dispatch::route_message(ROBOTSOCKETMESSAGE &rsm)
{
	printf("search route message\n");
	return 0;
}

void robot_message_dispatch::run()
{

	while (1)
	{
		Message msg = getq();
		
		if (msg.msgType==-1)
			continue;

		switch (msg.msgType)
		{
		case NET_DATA_IN:
			net_data_in_handler(msg);
			break;
		case 10:
			if (!msg.message.empty())
				m_cache_netdata.erase(msg.message);
			break;
		case  _EXIT_THREAD_:
			LOGW("dispatch server thread exit !");
			return;
		default:
			LOGE("unknow message %d--%s--%x",msg.msgType,msg.message.c_str(),msg.msgObject);
		}
	}
}


void robot_message_dispatch::net_data_in_handler(Message &msg)
{
	if (msg.msgObject == NULL)
	{
		LOGE("message data buffer ptr NULL %d--%s--%x", msg.msgType, msg.message.c_str(), msg.msgObject);
		return;
	}
	bool data_ready = false;
	net_msg* nmsg = ((net_msg*)(msg.msgObject));
	if (nmsg->guid.empty())
	{
		LOGE("message data buffer guid NULL %d--%x", msg.msgType, msg.msgObject);
		SAFE_DELETE(nmsg);
		return;
	}
	m_cache_netdata[nmsg->guid].buffer.insert(m_cache_netdata[nmsg->guid].buffer.end(), nmsg->recv_data.begin(), nmsg->recv_data.end());
	
	bool loop = true;
	do 
	{
		ROBOTSOCKETMESSAGE rsm;
		int ret = check_and_get_package_info(nmsg->guid, rsm);
		if (ret == 1 || ret == -1)
			loop = false;
		else
		{
			if (need_route_message(rsm) == 0)
				local_msg_handler(rsm);
			else
				route_message(rsm);
		}
	} while (loop);

	SAFE_DELETE(nmsg);
}

void robot_message_dispatch::local_msg_handler(ROBOTSOCKETMESSAGE &rsm)
{
	Json::Value   json_object;
	Json::Reader  json_reader(Json::Features::strictMode());
	if (rsm.messageType != 0)
		robot_module_manager::get_Instance()->CallModuleProcess(rsm);
	else
	{
		bool b_json_read = json_reader.parse(rsm.json.c_str(), json_object);//TODO simple strcat
		if (b_json_read)
		{
			std::string out;
			common_utf8_2gbk(&rsm.json, &out);
			LOGD("Receive message request %s", out.c_str());
			if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
			{
				std::string msgtype = json_object["msg_type"].asString();
				common_trim(&msgtype);

				if (msgtype.compare("SERVICE_SHOWHIDE_REQUEST") == 0)
				{
					if (hwnd)
					{
						if (IsWindowVisible(hwnd))
						{
							ShowOwnedPopups(hwnd, SW_HIDE);
							ShowWindow(hwnd, SW_HIDE);
						}
						else
						{
							ShowOwnedPopups(hwnd, SW_SHOW);
							ShowWindow(hwnd, SW_SHOW);
						}
					}
				}

				else if (msgtype.compare("hand ok") == 0 || msgtype.compare("handshake") == 0 || 
					msgtype.compare("HANDSHAKE") == 0 || msgtype.compare("ROBOT_VERSION_REQUEST") == 0)
				{

					robot_module_manager::get_Instance()->getModuleVersion(rsm);
				}
				else if (msgtype.compare("ROBOT_SERIALID_REQUEST") == 0 )
				{
					robot_module_manager::get_Instance()->getRobotSerial(rsm);
				}
				else
				{
					robot_module_manager::get_Instance()->CallModuleProcess(msgtype, rsm);
				}

			}
		}
		else
		{
			LOGE("socket json format error:%s", rsm.json.c_str());
		}
	}
	
}


