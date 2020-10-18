#include "robot_message_handler.h"
#include "robot_client_manager.h"
#include "ServerInterface.h"
#include "typeenum.h"


robot_message_handler::robot_message_handler()
{
}


robot_message_handler * robot_message_handler::getptr()
{
	static robot_message_handler _rmah;
	return &_rmah;
}

robot_message_handler::~robot_message_handler()
{
	if (state())quit();
}


void robot_message_handler::add_vectorchar_int(int value, std::vector<char > &out)
{
	/*value = htonl(value);*/
	out.push_back((char)((value & 0xFF000000) >> 24));
	out.push_back((char)((value & 0x00FF0000) >> 16));
	out.push_back((char)((value & 0x0000FF00) >> 8));
	out.push_back((char)((value & 0x000000FF)));
}

void robot_message_handler::add_vectorchar_short(int value, std::vector<char > &out)
{
	/*value = htons(value);*/
	out.push_back((char)((value & 0x0000FF00) >> 8));
	out.push_back((char)((value & 0x000000FF)));
}

void robot_message_handler::add_head_package_response(ROBOTSOCKETMESSAGE *rsmsg, std::vector<char > &out)
{
	out.clear();
	out.push_back(SOCKET_FIRST_HEAD);
	out.push_back(SOCKET_SECOND_HEAD);
	out.push_back(0x00);
	out.push_back(0x00);
	add_vectorchar_int(SOCKET_SENDNODE_LOCAL, out);
	add_vectorchar_int(SOCKET_RECVNODE_LOCAL, out);
	add_vectorchar_int(rsmsg->appID, out);
	add_vectorchar_int(ROBOT_MAIN_SERVER_ID, out);
	add_vectorchar_short(rsmsg->eventID, out);
	add_vectorchar_short(rsmsg->messageType, out);
	if (rsmsg->messageType == 0)
	{
		add_vectorchar_int(rsmsg->json.length(), out);
		out.insert(out.end(), rsmsg->json.begin(), rsmsg->json.end());
	}
	else
	{
		add_vectorchar_int(rsmsg->senddata.size(), out);
		out.insert(out.end(), rsmsg->senddata.begin(), rsmsg->senddata.end());
	}

}


void robot_message_handler::message_convert_result(Message &msg)
{
	if (!msg.msgObject) return;
	ROBOTSOCKETMESSAGE *rsmsg = (ROBOTSOCKETMESSAGE *)msg.msgObject;
	net_msg      nmsg;
	if (rsmsg==NULL||MESSAGE_NORESULT == rsmsg->task_state)
		goto end;

	if (rsmsg->messageType == 0)
	{
		if (rsmsg->json.size() != 0)
		{
			std::string out;
			common_utf8_2gbk(&rsmsg->json, &out);
			LOGD("Send message response %s", out.c_str());
		}
	}
	if (rsmsg->json.size() != 0 || rsmsg->senddata.size() != 0)
	{
		rsmsg->appID = robot_client_manager::getptr()->get_appid(rsmsg->to_client_guid);
		nmsg.guid = rsmsg->to_client_guid;
		add_head_package_response(rsmsg, nmsg.recv_data);
		ServerInterface::getPtr()->send_data(nmsg);
	}


end:
	SAFE_DELETE(rsmsg);

}

void robot_message_handler::message_convert_result_all(Message &msg)
{
	if (!msg.msgObject) return;
	ROBOTSOCKETMESSAGE *rsmsg = (ROBOTSOCKETMESSAGE *)msg.msgObject;
	net_msg      nmsg;
	if (rsmsg == NULL || MESSAGE_NORESULT == rsmsg->task_state)
		goto end;
	if (rsmsg->messageType == 0)
	{
		if (rsmsg->json.size() != 0)
		{
			std::string out;
			common_utf8_2gbk(&rsmsg->json, &out);
			LOGD("Send message response %s", out.c_str());
		}
	}

	if (rsmsg->json.size() != 0 || rsmsg->senddata.size() != 0)
	{
		std::vector<std::string > ntf_list = robot_client_manager::getptr()->get_ntf_list(2);
		for (int i = 0; i < ntf_list.size(); i++)
		{
			nmsg.guid = ntf_list[i];
			rsmsg->appID = robot_client_manager::getptr()->get_appid(ntf_list[i]);
			add_head_package_response(rsmsg, nmsg.recv_data);
			ServerInterface::getPtr()->send_data(nmsg);
		}
	}

end:
	SAFE_DELETE(rsmsg);

}


void robot_message_handler::run()
{
	while (1)
	{
		Message msg = getq();

		if (msg.msgType == -1)
			continue;

		switch (msg.msgType)
		{
		case ROBOT_RESULT_MESSAHE:
			message_convert_result(msg);//todo thread pool
			break;
		case ROBOT_RESULT_MESSAHE_ALL:
			message_convert_result_all(msg);//todo thread pool
			break;
		case _EXIT_THREAD_:
			LOGW("handler server thread exit !");
			return ;
		default:
			LOGE("unknow message %d--%s--%x", msg.msgType, msg.message.c_str(), msg.msgObject);
		}
	}
}
