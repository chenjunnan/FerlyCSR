#include "ServerInterface.h"
#include <winsock2.h>
#include <map>
#include <list>
#include <ws2tcpip.h>
#include <assert.h>
#include "xCommon.h"
#include "common_helper.h"
#include "robot_client_manager.h"
#include "typeenum.h"
#include "robot_message_dispatch.h"

#define MAX_RECVBUFFER_SIZE (64*1024-1)

ServerInterface::ServerInterface()
{
	recv_buffer = NULL;
	recv_buffer = new char[MAX_RECVBUFFER_SIZE];
	if (recv_buffer)
		assert("memory error");
	m_mapml = common_mutex_create_threadlock();
}


ServerInterface* ServerInterface::getPtr()
{
	static ServerInterface __server;
	return &__server;
}

bool ServerInterface::set_address(const char *ip, int port)
{
	m_listen.set_address(ip, port);
	if (m_listen.set_block(true))
	{
		LOGF("set block error");
		return false;
	}

	if(m_listen.set_socket_default())
	{
		LOGF("set socket default error");
		return false;
	}
	if (m_listen.set_buffer_size(MAX_RECVBUFFER_SIZE, MAX_RECVBUFFER_SIZE))
	{
		LOGF("set socket buffersize error");
		return false;
	}
	if(m_listen.xlisten(5))
	{
		LOGF("llisten error");
		return false;
	}
	return true;
}

int ServerInterface::send_data(net_msg &_msg)
{
	int ret = -1;
	HSocket  temph = NULL;

	{
		LOCK_HELPER_AUTO_HANDLER(m_mapml);

		auto it = m_socket_list.find(_msg.guid);
		if (it == m_socket_list.end())
		{
			robot_client_manager::getptr()->remove_client(_msg.guid);
			LOGE("unfind %s socket ,may it close before!", _msg.guid.c_str());
			goto end;
		}

		if (!it->second->isvalid())
		{
			LOGI("socket not exist !");
			goto end;
		}


		if (_msg.recv_data.size() == 0)
			goto end;

		temph = it->second->get_handle();
	}



	int Total = 0;
	int lenSend = 0;

	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	fd_set wset;
	while (1)
	{
		FD_ZERO(&wset);
		FD_SET(temph, &wset);
		if (select(0, NULL, &wset, NULL, &tv) > 0)//2秒之内可以send，即socket可以写入    
		{
			lenSend = send(temph, _msg.recv_data.data() + Total, _msg.recv_data.size() - Total, 0);
			if (lenSend == -1)
				break;
			Total += lenSend;
			if (Total == _msg.recv_data.size())
			{
				ret = 0;
				break;
			}

		}
		else
			break;
	}
	LOGT("send %d bytes-totle:%d", Total, _msg.recv_data.size());
	if (Total == 0)
		pushq(10, _msg.guid);
	//printf("%lld %s %d send %d bytes", common_get_longcur_time_stampms(), it->second->get_ip_string().c_str(), it->second->get_port(), _msg.recv_data.size());
end:
	return ret;
}



void ServerInterface::run()
{

	fd_set server_fd_read_set;
	fd_set server_fd_write_set;
	int max_fd = -1;
	struct timeval tv;
	std::set<std::string >  remove_guid;
	LOGI("Server listen port:50316");
	while (1)
	{
		tv.tv_sec = 0;
		tv.tv_usec = 200*1000;
		//printf("loop");
		Message msg = getq(0);
		if (msg.msgType == _EXIT_THREAD_)
		{
			LOGW("listen server thread exit !");
			return;
		}
		if (msg.msgType==10)
			remove_one(msg.message);

		FD_ZERO(&server_fd_read_set);
		FD_ZERO(&server_fd_write_set);

		FD_SET(m_listen.get_handle(), &server_fd_read_set);

		auto it = m_socket_list.begin();
		for (; it != m_socket_list.end();it++)
			FD_SET(it->second->get_handle(), &server_fd_read_set);

		int ret = select(0, &server_fd_read_set, &server_fd_write_set, NULL, &tv);
		if (ret < 0)
		{
			LOGE("server select error!!!!");
			continue;
		}
		else if (ret == 0)
			continue;
		else
		{
			if (FD_ISSET(m_listen.get_handle(), &server_fd_read_set))
				new_connect();
			server_recvice_data(server_fd_read_set, remove_guid);
			remove_socket(remove_guid);
		}
	}
	LOGF("Server thread close");
}

void ServerInterface::new_connect()
{

	struct sockaddr_in client_address;
	HSocket in = m_listen.socket_accept(&client_address);
	if (ISSOCKHANDLE(in))
	{
		FSocket *xsket = new FSocket(in);
		std::string guids;
		common_get_guid(&guids);
		if (m_socket_list.size() >= 64)
		{
			LOGE("connected client reach max counts");
			xsket->close();
			delete xsket;
			return;
		}
		LOCK_HELPER_AUTO_HANDLER(m_mapml);
		xsket->set_block(true);
		xsket->set_time_out(2000, 2000, 0);
		xsket->set_socket_default();
		xsket->set_buffer_size(MAX_RECVBUFFER_SIZE, MAX_RECVBUFFER_SIZE);
		m_socket_list[guids] = xsket;
		LOGI("new connection client %s:%d", xsket->get_ip_string().c_str(),xsket->get_port());
	}
	else
		LOGW("accept error %d", in);
}

void ServerInterface::remove_one(std::string guid)
{
	LOCK_HELPER_AUTO_HANDLER(m_mapml);
	auto it = m_socket_list.find(guid);
	if (it != m_socket_list.end())
	{
		FSocket *xsket = it->second;
		if (xsket)
		{
			LOGI("disconnect from %s:%d", it->second->get_ip_string().c_str(), xsket->get_port());
			xsket->close();
			LOGI("close done");
			SAFE_DELETE(xsket)
		}
		robot_client_manager::getptr()->remove_client(guid);
		robot_message_dispatch::getptr()->remove(guid);
		m_socket_list.erase(it);
	}
}


void ServerInterface::remove_socket(std::set<std::string> &remove_guid)
{

	while (remove_guid.size())
	{
		remove_one(*remove_guid.begin());
		remove_guid.erase(remove_guid.begin());
	}
}



void ServerInterface::server_recvice_data(fd_set &server_fd_read_set, std::set<std::string> &remove_guid)
{
	auto it = m_socket_list.begin();
	for (; it != m_socket_list.end(); it++)
	{
		if (FD_ISSET(it->second->get_handle(), &server_fd_read_set))
		{
			memset(recv_buffer, 0x00, MAX_RECVBUFFER_SIZE);

			int ret = it->second->xrecv(recv_buffer, MAX_RECVBUFFER_SIZE);

			if (ret > 0 || ret == SOCKET_RECVMAX)
			{
				LOGT("%s %d recv %d bytes", it->second->get_ip_string().c_str(), it->second->get_port(), ret == SOCKET_RECVMAX ? MAX_RECVBUFFER_SIZE : ret);
				int datalen = ret == SOCKET_RECVMAX ? MAX_RECVBUFFER_SIZE : ret;
				net_msg *netmsg = new net_msg;
				netmsg->guid = it->first;
				netmsg->recv_data.assign(recv_buffer, recv_buffer + datalen);

				Message msg;
				msg.msgType = NET_DATA_IN;
				msg.msgObject = netmsg;
				robot_message_dispatch::getptr()->pushq(msg);
			}
			else
			{
				if (ret == SOCKET_DISCONNECT)
					remove_guid.insert(it->first);
				else
				{
					if (ret == SOCKET_EAGAIN)
					{
						LOGW("socket %s  will wait!!!!?", it->second->get_ip_string().c_str());
					}
					else
					{
						LOGE("socket %s error !", it->second->get_ip_string().c_str());
						remove_guid.insert(it->first);
					}
				}
			}

		}


	}
}

ServerInterface::~ServerInterface()
{
	if (state())
		quit();
	{
		LOCK_HELPER_AUTO_HANDLER(m_mapml);
		while (m_socket_list.size())
		{
			auto it = m_socket_list.begin();
			it->second->close();
			SAFE_DELETE(it->second);
			m_socket_list.erase(it);
		}
		SAFE_DELETE(recv_buffer);
	}
	common_mutex_threadlock_release(m_mapml);
}
