#include "iotServerForWifi50315.h"
#include "xCommon.h"
#include "robot_PostMessage.h"
#include "json\json.h"

#define PORT_IOT 8088
#define SPIT_COMMAND_WAIT  10


iotServerForWifi50315::iotServerForWifi50315()
{
	m_ml = common_mutex_create_threadlock();
}


iotServerForWifi50315::~iotServerForWifi50315()
{
	common_mutex_threadlock_release(m_ml);
}

int iotServerForWifi50315::command(xSocketUserInterface* h,std::string cmd,int ts)
{
	m_socket_map[h].ready = false;
	LOGBD("%s [ %s ]write :[ %s ]", m_socket_map[h].ip.c_str(), m_socket_map[h].id.c_str(), cmd.c_str());
	std::string out;
	while (out.size()%2!=0||out.size()<10)
		out.append(cmd);

	int ret = h->Write((unsigned char*)out.c_str(), out.size(), ts);
	m_socket_map[h].active_last_time = common_get_longcur_time_stampms();
	if (ret == out.size())
		return cmd.size();
	LOGBE("%s [ %s ] write %s failed", m_socket_map[h].ip.c_str(), m_socket_map[h].id.c_str(), cmd.c_str());
	m_socket_map[h].ready = true;
	return ret;
}


void iotServerForWifi50315::clean_overdue_task(std::string &id, std::list<MessageRequest>& _list, const char *merr)
{
	if (id.empty())return;

	for (auto it = _list.begin(); it != _list.end();)
	{
		if ((common_get_longcur_time_stampms() - it->send_time) >it->context.timeout)
		{
			LOGE("box %s list %s timeout error", id.c_str(), merr);
			if (!it->context.guid.empty())
			{
				std::string out = JsonPackage(id, it->type, 2);

				robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length(), it->context.guid.c_str());

			}
			it=_list.erase(it);
		}
		else
			it++;
	}


}

void iotServerForWifi50315::clean_overdue_donetask(std::string &id, std::list<MessageRequest>& _list)
{
	if (id.empty())return;

	for (auto it = _list.begin(); it != _list.end();)
	{
		if ((common_get_longcur_time_stampms() - it->send_time )> it->context.timeout)
		{
			LOGE("box %s timeout error not ready ", id.c_str());

			if (!it->context.guid.empty())
			{
				robot_PostMessage::getPostManager()->post(MODULE_NAME, it->resultjson.c_str(), it->resultjson.length(), it->context.guid.c_str());
			}



			it = _list.erase(it);
		}
		else
			it++;
	}


}


int iotServerForWifi50315::timeout()
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	int next_timeout = 50;
	for (auto it = m_socket_map.begin(); it != m_socket_map.end();it++)
	{
		int next_wait_spit_time = SPIT_COMMAND_WAIT;
		clean_overdue_task(it->second.id, it->second.readysend_list,"write");
		clean_overdue_task(it->second.id, it->second.wait_list,"read");
		clean_overdue_donetask(it->second.id, it->second.report_list);

		if (it->second.id.empty())
			next_wait_spit_time = 2000;

		if (it->second.ready && ((common_get_longcur_time_stampms() - it->second.active_last_time) >= next_wait_spit_time))
		{
			if (!it->second.readysend_list.empty())
			{
				msg_context temp;
				temp.id = it->second.id;
				temp = it->second.readysend_list.front().context;
				Post_Command(it->second.readysend_list.front().type, it->first, &temp,common_get_longcur_time_stampms());
				it->second.readysend_list.pop_front();
			}
			else
			{
				if (common_get_longcur_time_stampms() - it->second.active_last_time > 2 * 1000)
					HeartBeat(it->first);
			}
		}
		else
		{
			unsigned long long tims = common_get_longcur_time_stampms();
			if (!it->second.readysend_list.empty() && it->second.ready && (tims - it->second.active_last_time < SPIT_COMMAND_WAIT))
			{
				next_timeout = abs((int)(SPIT_COMMAND_WAIT - (tims - it->second.active_last_time))) < next_timeout ?
					abs((int)(SPIT_COMMAND_WAIT - (tims - it->second.active_last_time))) : next_timeout;
			}
		}
		

		if (it->second.online&&((common_get_longcur_time_stampms() - it->second.active_last_time) > 15 * 1000))
		{
			//it->second.ready = true;
			it->second.online = false;
			it->second.lastdata.clear();
			while (!it->second.wait_list.empty())
			{
				PostOneTimeoutMessage(it->second.wait_list.front(), it->second.id);
				it->second.wait_list.pop_front();
			}


			while (!it->second.report_list.empty())
			{
				MessageRequest waitmessage = it->second.report_list.front();

				robot_PostMessage::getPostManager()->post(MODULE_NAME, waitmessage.resultjson.c_str(), waitmessage.resultjson.length(), waitmessage.context.guid.c_str());

				it->second.report_list.pop_front();
			}
			m_file_state.Boxoffline(it->second.ip);
			//LOGE("[ %s ] device offline", it->second.id.c_str());
			LOGBE("%s [ %s ] device offline", it->second.ip.c_str(),it->second.id.c_str());
		}

		if ((common_get_longcur_time_stampms() - it->second.active_last_time > 15 * 1000))
			HeartBeat(it->first);
		
	}

	return abs(next_timeout)>50 ? 50 : (abs(next_timeout) == 0 ? 2 : abs(next_timeout));
}

void iotServerForWifi50315::HeartBeat(xSocketUserInterface* it)
{
	std::string cmd = getCommandHeadstr(GETLIST);
	command(it, cmd.c_str(), 1);
}

void iotServerForWifi50315::run()
{
	SocketServerInterface * server=common_socket_create(this);
	m_file_state.start();
	if (server->Listen(PORT_IOT) != 0)
	{
		LOGE("IOT error listen %d", PORT_IOT);
		common_socket_destory(server);
		return;
	}
	else
	{
		LOGI("IOT server listen %d", PORT_IOT);
	}

	int sleep_time = 50;
	while (1)
	{
		Message msg = getq(sleep_time);

		if (msg.msgType == _EXIT_THREAD_)break;

		msg_context *msgc = (msg_context *)msg.msgObject;
		if (msgc != NULL)
		{
			msg_process(msgc, msg.msgType);
			SAFE_DELETE(msgc);
		}
		sleep_time=timeout();

	}
// 	for (auto it = m_socket_map.begin(); it != m_socket_map.end();it++)
// 		it->first->Close();
	//m_socket_map.clear();
	m_file_state.quit();
	server->Close();
	common_socket_destory(server);
}

void iotServerForWifi50315::AppendPortList(Json::Value &json_object_base)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	for (auto it = m_socket_map.begin(); it != m_socket_map.end(); it++)
	{
		Json::Value   json_box;
		if (it->second.type == DBOX)
			json_box["type"] = Json::Value("box");
		if (it->second.type == DFRAME)
			json_box["type"] = Json::Value("frame");
		json_box["id"] = Json::Value(it->second.id.c_str());
		json_box["ip"] = Json::Value(it->second.ip.c_str());
		json_box["online"] = Json::Value(it->second.online);
		json_box["port"] = Json::Value(it->second.port);
		json_box["ready"] = Json::Value(it->second.ready);
		json_box["waitwrite_size"] = Json::Value(it->second.readysend_list.size());
		json_box["waitread_size"] = Json::Value(it->second.wait_list.size());
		json_box["waitready_size"] = Json::Value(it->second.report_list.size());
		json_box["access"] = Json::Value("Wifi mode");
		json_object_base["box_list"].append(json_box);
	}
}


// 
// GETLIST = 99,
// OPENDOOR,
// CLOSEDOOR,
// GETDOORSTATUS,
// GETPAPERSTATUS,
// GETBOXBATTERY,
// SETBOXAP,
// SETFRAMEADSORB_OPEN,
// SETFRAMEADSORB_CLOSE,


void iotServerForWifi50315::msg_process(msg_context * msgc, int messagetype)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	if (messagetype == GETLIST)
	{
		std::string out=getIotlist();
		robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length(),
			msgc->guid.c_str());
		return;
	}
	xSocketUserInterface *  handler=getHandler(messagetype, msgc);

	if (handler)
	{
		if (!m_socket_map[handler].online)
		{
			std::string out = JsonPackage(msgc->id,messagetype, 4);

			robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length(),
				msgc->guid.c_str());
			return;
		}
		if (m_socket_map[handler].readysend_list.size()>=3)
		{
			std::string out = JsonPackage(msgc->id, messagetype, 5);

			robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length(),
				msgc->guid.c_str());

			return;
		}

		MessageRequest mr(messagetype, common_get_longcur_time_stampms());
		mr.context = *msgc;
		m_socket_map[handler].readysend_list.push_back(mr);
	}
	else
	{
		std::string out = JsonPackage(msgc->id, messagetype, 3);
		robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length(), msgc->guid.c_str());
		LOGW("cant find ID %s box or frame", msgc->id.c_str());
	}
}

xSocketUserInterface * iotServerForWifi50315::getHandler(int messagetype, msg_context * msgc)
{
	std::string  serachID;

	int messageto = DBOX;
	if (messagetype == SETFRAMEADSORB_OPEN || messagetype == SETFRAMEADSORB_CLOSE)
	{
		messageto = DFRAME;
		if (msgc->mainid.empty())
		{
			for (auto it = m_socket_map.begin(); it != m_socket_map.end(); it++)
			{
				if (it->second.type == DFRAME)
				{
					serachID = it->second.id;
					break;
				}
			}
		}
		else
			serachID = msgc->mainid;
	}
	else
		serachID = msgc->id;

	if (serachID.empty())
		return NULL;

	return find_handle(serachID, messageto);
}

std::string iotServerForWifi50315::getIotlist()
{
	Json::Value   json_object_post;
	Json::FastWriter json_writer;
	json_object_post["msg_type"] = Json::Value("IOT_GETID_LIST_RESPONSE");
	json_object_post["error"] = 0;
	json_object_post["msg"] = Json::Value("success");
	for (auto it = m_socket_map.begin(); it != m_socket_map.end(); it++)
	{
		Json::Value   json_box;
		if (it->second.type==DBOX)
			json_box["type"] = Json::Value("box");
		if (it->second.type == DFRAME)
			json_box["type"] = Json::Value("frame");
		json_box["id"] = Json::Value(it->second.id.c_str());
		json_box["ip"] = Json::Value(it->second.ip.c_str());
		json_box["online"] = Json::Value(it->second.online);
		json_box["port"] = Json::Value(it->second.port);
		json_box["ready"] = Json::Value(it->second.ready);
		json_box["waitwrite_size"] = Json::Value(it->second.readysend_list.size());
		json_box["waitread_size"] = Json::Value(it->second.wait_list.size());
		json_box["waitready_size"] = Json::Value(it->second.report_list.size());
		json_object_post["box_list"].append(json_box);
	}
	return json_writer.write(json_object_post);
}


std::string iotServerForWifi50315::getCommandHeadstr(int type)
{
	if (type == GETLIST)
		return "GetID ";
	if (type == OPENDOOR)
		return "OpenDoor";
	if (type == CLOSEDOOR)
		return "CloseDoor ";
	if (type == GETDOORSTATUS)
		return "GetDoor ";
	if (type == GETPAPERSTATUS)
		return "GetPaper ";
	if (type == GETBOXBATTERY)
		return "GetBat ";
	if (type == SETBOXAP)
		return "SetAP";
	if (type == SETFRAMEADSORB_OPEN)
		return "Open";
	if (type == SETFRAMEADSORB_CLOSE)
		return "Close";
	return "";
}


void iotServerForWifi50315::Post_Command(int type, xSocketUserInterface * handler, msg_context * msgc,unsigned long long times)
{
	int ret = 0;
	std::string command_str;
	command_str=packageCommand(type, msgc);

	if (!command_str.empty())
		ret = command(handler, command_str, msgc->timeout);
	//ret = handler->Write((unsigned char*)command_str.c_str(), command_str.size(), msgc->tiemout);

	if (ret == command_str.size())
	{
		MessageRequest mr(type, times); mr.context = *msgc;
		m_socket_map[handler].wait_list.push_back(mr);
	}
	else
	{
		if (!msgc->guid.empty() /*&& (ret <= 0)*/)
		{
			std::string out = JsonPackage(m_socket_map[handler].id,type, 1);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length(), msgc->guid.c_str());
		}
	}
}

std::string  iotServerForWifi50315::packageCommand(int type, msg_context * msgc)
{
	std::string command_str = getCommandHeadstr(type);

	if (type == OPENDOOR)
	{
		char mixRect[BOX_MAX_LAYER][2];
		memset(mixRect, '0', 2 * BOX_MAX_LAYER*sizeof(char));
		for (auto it = msgc->door_list.begin(); it != msgc->door_list.end(); it++)
		{
			if (it->first<BOX_MAX_LAYER)
				mixRect[it->first][it->second] = '1';
			else
			{
				LOGBE("over max limit %d---%d", it->first, it->second);
			}

		}

		for (int i = 0; i < BOX_MAX_LAYER; i++)
		{
			command_str.push_back('-');
			command_str.push_back(mixRect[i][0]);
			command_str.push_back(mixRect[i][1]);
		}
		command_str.push_back(' ');
	}

	if (type == SETBOXAP)
	{
		command_str.push_back('-');
		command_str.push_back(msgc->mode + '0');
		command_str.push_back(' ');
	}
	if (type == SETFRAMEADSORB_OPEN)
	{
		command_str.push_back('-');
		command_str.append(msgc->id);
		command_str.push_back(' ');
	}
	if (type == SETFRAMEADSORB_CLOSE)
	{
		command_str.push_back('-');
		command_str.append(msgc->id);
		command_str.push_back(' ');
	}
	return command_str;
}

void iotServerForWifi50315::onConnect(xSocketUserInterface* handle)
{

}

void iotServerForWifi50315::onAccept(xSocketUserInterface* handle)
{
	socket_context temp;
	int localport=0;
	handle->GetInfo(temp.ip, temp.port, localport,temp.p_hsocket);
	//LOGD("connect %s: %d", temp.ip.c_str(), temp.port);
	
	if (0 == temp.port)
	{
		LOGBE("connect error %s: %d", temp.ip.c_str(), temp.port);
		handle->Close();
	}
	else
	{
		LOGBI("connect %s: %d", temp.ip.c_str(), temp.port);
		LOCK_HELPER_AUTO_HANDLER(m_ml);
		temp.active_last_time = common_get_longcur_time_stampms();
		m_socket_map[handle] = temp;
		for (auto it = m_socket_map.begin(); it != m_socket_map.end();)
		{
			if (it->first != handle&&it->second.ip.compare(temp.ip) == 0 )//default same box id same ip  //todo
			{
				m_file_state.Boxoffline(m_socket_map[handle].ip);
				LOGBE("%s [%s:%d] device close", it->second.id.c_str(), it->second.ip.c_str(), it->second.port);
				m_socket_map[handle].id = it->second.id;
				m_socket_map[handle].type = it->second.type;
				shifttask(it->first, handle);
				it = m_socket_map.erase(it);
				
			}
			else
				it++;
		}
		

	}
}



void iotServerForWifi50315::onRead(xSocketUserInterface* handle, VEC_BYTE data)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	if (m_socket_map.find(handle) == m_socket_map.end())
	{
		LOGBE("maybe close before ,cant find handle!!!");
		return;
	}
	if (data.size() == 0)return;

	std::string read_str;
	read_str.assign(data.begin(), data.end());


	if (m_socket_map[handle].lastdata.compare(read_str) != 0)
	{
		LOGBD("%s [ %s ]read :[ %s ]", m_socket_map[handle].ip.c_str(), m_socket_map[handle].id.c_str(), read_str.c_str());
	}
	m_file_state.hasdata(m_socket_map[handle].ip);
	m_socket_map[handle].lastdata = read_str;
	m_socket_map[handle].online = true;

	
	//printf("%s [%s]\n", m_socket_map[handle].ip.c_str(), read_str.c_str());

	m_socket_map[handle].m_cache.append(read_str);

	while (m_socket_map[handle].m_cache.find(' ') != std::string::npos)
	{
		int beginpos = m_socket_map[handle].m_cache.find(' ');

		std::string order = m_socket_map[handle].m_cache.substr(0, beginpos);

		m_socket_map[handle].m_cache.erase(0, beginpos + 1);

	
		process_STMresponse(order, handle);

		if (m_socket_map.find(handle) == m_socket_map.end())break;
	}
}

void iotServerForWifi50315::process_STMresponse(std::string &order, xSocketUserInterface* handle)
{
	if (order.compare("Ready") == 0)
	{
		m_socket_map[handle].active_last_time = common_get_longcur_time_stampms();
		if (m_socket_map[handle].id.empty())
		{
			if (m_socket_map[handle].readysend_list.size() == 0 && m_socket_map[handle].wait_list.size()==0)
			{
				MessageRequest mr(GETLIST, common_get_longcur_time_stampms());
				m_socket_map[handle].readysend_list.push_back(mr);
			}
			// 			common_thread_sleep(1000);
		}
		else
			report(handle);

/*		if (m_socket_map.find(handle) != m_socket_map.end())*/
		m_socket_map[handle].ready = true;
	}

	else
		makeresponse(handle, order);
}



void iotServerForWifi50315::makeresponse(xSocketUserInterface* handle, std::string result)
{
	if (m_socket_map.find(handle) == m_socket_map.end())
	{
		LOGBE("error find handle!!!");
		return;
	}

	std::string out;
	if (result.empty())return;

	if (m_socket_map[handle].wait_list.size() != 0)
	{
		MessageRequest waitmessage = *m_socket_map[handle].wait_list.begin();

		if (waitmessage.type == GETLIST)
		{
			if (m_socket_map[handle].id.empty() && result.size()>1)
			{
				if (result.at(0) == 'B')
					m_socket_map[handle].type = DBOX;
				else if (result.at(0) == 'F')
					m_socket_map[handle].type = DFRAME;
				else
					goto end;
				result.erase(result.begin());

				for (auto it = m_socket_map.begin(); it != m_socket_map.end();)
				{
					if (it->first != handle&&it->second.id.compare(result) == 0 && it->second.type == m_socket_map[handle].type)
					{
						LOGBE("%s [%s:%d] device close", it->second.id.c_str(), it->second.ip.c_str(), it->second.port);
						shifttask(it->first, handle);

						it=m_socket_map.erase(it);
					}
					else
						it++;
				}
				
				m_socket_map[handle].id = result;
				//m_socket_map[handle].wait_list.pop_front();
				//m_socket_map[handle].ready = true;
				m_file_state.SetID(m_socket_map[handle].ip, m_socket_map[handle].id);
				//LOGI("online [%d] %s device", m_socket_map[handle].type, result.c_str());
				LOGBI("online [%d] %s device [%s:%d]", m_socket_map[handle].type, result.c_str(), m_socket_map[handle].ip.c_str(), m_socket_map[handle].port);
			}
			
		}
		if (waitmessage.type == OPENDOOR || waitmessage.type == SETFRAMEADSORB_OPEN || waitmessage.type == SETFRAMEADSORB_CLOSE || waitmessage.type == SETBOXAP)
		{
			if (result.compare("OK") == 0)
				waitmessage.resultjson = JsonPackage(m_socket_map[handle].id,waitmessage.type, 0);
			else
				waitmessage.resultjson = JsonPackage(m_socket_map[handle].id, waitmessage.type, 6);

			m_socket_map[handle].report_list.push_back(waitmessage);
		}
		if (waitmessage.type == GETDOORSTATUS || waitmessage.type == GETPAPERSTATUS)
		{
			std::vector<std::string>	vecstr = split(result, '-');
			if (vecstr.size()==0)
				waitmessage.resultjson = JsonPackage(m_socket_map[handle].id, waitmessage.type, 7);
			else
				waitmessage.resultjson = JsonPackage(m_socket_map[handle].id, waitmessage.type, vecstr, 0);

			m_socket_map[handle].report_list.push_back(waitmessage);
		}
		if (waitmessage.type == GETBOXBATTERY)
		{
			std::vector<std::string>	vecstr = split(result, '-');
			if (vecstr.size() == 2)
			{
				int value = atoi(vecstr[1].c_str());
				waitmessage.resultjson = JsonPackage(m_socket_map[handle].id, waitmessage.type, value, 0);
			}else
				waitmessage.resultjson = JsonPackage(m_socket_map[handle].id, waitmessage.type, 7);

			m_socket_map[handle].report_list.push_back(waitmessage);
		}
end:
		m_socket_map[handle].wait_list.pop_front();


	}
}

void iotServerForWifi50315::shifttask(xSocketUserInterface* oldh, xSocketUserInterface*newh)
{	
	oldh->Close();

	if (!m_socket_map[oldh].readysend_list.empty())
		m_socket_map[newh].readysend_list = m_socket_map[oldh].readysend_list;
	m_socket_map[oldh].readysend_list.clear();

	while (!m_socket_map[oldh].wait_list.empty())
	{
		PostOneTimeoutMessage(m_socket_map[oldh].wait_list.front(), m_socket_map[oldh].id);
		m_socket_map[oldh].wait_list.pop_front();
	}

	while (!m_socket_map[oldh].report_list.empty())
	{
		MessageRequest waitmessage = m_socket_map[oldh].report_list.front();

		robot_PostMessage::getPostManager()->post(MODULE_NAME, waitmessage.resultjson.c_str(), waitmessage.resultjson.length(), waitmessage.context.guid.c_str());

		m_socket_map[oldh].report_list.pop_front();
	}

}

void iotServerForWifi50315::report(xSocketUserInterface* handle)
{
	if (m_socket_map.find(handle) == m_socket_map.end())
	{
		LOGBE("error find handle!!!");
		return;
	}

	if (m_socket_map[handle].report_list.size() != 0)
	{
		MessageRequest waitmessage = m_socket_map[handle].report_list.front();

		robot_PostMessage::getPostManager()->post(MODULE_NAME, waitmessage.resultjson.c_str(), waitmessage.resultjson.length(), waitmessage.context.guid.c_str());

		m_socket_map[handle].report_list.pop_front();

		if (waitmessage.type == SETBOXAP)
		{
			m_file_state.changeNet(m_socket_map[handle].ip);
			//handle->Close();
			RemoveDevice(handle);
		}

	}
// 	if (m_socket_map[handle].readysend_list.size() != 0)
// 	{
// 		msg_context temp;
// 		temp.id = m_socket_map[handle].id;
// 		temp.guid = m_socket_map[handle].readysend_list.front().guid;
// 		temp.tiemout = m_socket_map[handle].readysend_list.front().timeout;
// 		Post_Command(m_socket_map[handle].readysend_list.front().type, handle,
// 			&temp, m_socket_map[handle].readysend_list.front().send_time);
// 		m_socket_map[handle].readysend_list.pop_front();
// 	}
// 	else
// 		;// m_socket_map[handle].ready = true;
}



xSocketUserInterface * iotServerForWifi50315::find_handle(std::string id,int type)
{
	for (auto it = m_socket_map.begin(); it != m_socket_map.end(); it++)
	{
		if (it->second.type==type&&it->second.id.compare(id) == 0)
			return it->first;
	}
	return NULL;
}

void iotServerForWifi50315::onWrite(xSocketUserInterface* handle, int error)
{

}

void iotServerForWifi50315::onClose(xSocketUserInterface* handle)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	RemoveDevice(handle);
}

void iotServerForWifi50315::PostOneTimeoutMessage(MessageRequest&msr,std::string id)
{
	if (!msr.context.guid.empty())
	{
		std::string out = JsonPackage(id, msr.type, 2);

		robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length(), msr.context.guid.c_str());
	}
}

void iotServerForWifi50315::PostRemainMessage(xSocketUserInterface* handle)
{
	if (m_socket_map.find(handle) == m_socket_map.end())
	{
		LOGBE("error find handle!!!");
		return;
	} 	
	
	while (!m_socket_map[handle].readysend_list.empty())
	{
		PostOneTimeoutMessage(m_socket_map[handle].readysend_list.front(), m_socket_map[handle].id);
		m_socket_map[handle].readysend_list.pop_front();
	}

	while (!m_socket_map[handle].wait_list.empty())
	{
		PostOneTimeoutMessage(m_socket_map[handle].wait_list.front(), m_socket_map[handle].id);
		m_socket_map[handle].wait_list.pop_front();
	}

	while (!m_socket_map[handle].report_list.empty())
	{
		MessageRequest waitmessage = m_socket_map[handle].report_list.front();

		robot_PostMessage::getPostManager()->post(MODULE_NAME, waitmessage.resultjson.c_str(), waitmessage.resultjson.length(), waitmessage.context.guid.c_str());

		m_socket_map[handle].report_list.pop_front();
	}
}

void iotServerForWifi50315::RemoveDevice(xSocketUserInterface* handle)
{
	for (auto it = m_socket_map.begin(); it != m_socket_map.end(); it++)
	{
		if (it->first == handle)
		{
			m_file_state.Boxoffline(it->second.ip);
			LOGBE("%s [%s:%d] device close", it->second.id.c_str(), it->second.ip.c_str(), it->second.port);
			PostRemainMessage(handle);
			//handle->Close();
			m_socket_map.erase(it);
			break;
		}
	}
}

void iotServerForWifi50315::onEvent(int error)
{
	LOGBD("iot state %d",error);
}

