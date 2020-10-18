#pragma once
#include "FSocket.h"
#include <map>
#include <list>
#include <vector>
#include "xCommon.h"
#include "common_helper.h"
#include <set>


class net_msg
{
public:
	net_msg(){  }
	~net_msg(){}
	std::string guid;
	std::vector<char> recv_data;
protected:
private:
};

class ServerInterface :public Thread_helper
{
public:
	static ServerInterface* getPtr();
	bool set_address(const char *ip, int port);
	int  send_data(net_msg &data);
	virtual ~ServerInterface();
private:
	virtual void run();
	void server_recvice_data(fd_set &server_fd_read_set, std::set<std::string> &remove_guid);
	void new_connect();
	void remove_one(std::string guid);
	void remove_socket(std::set<std::string> &remove_guid);
	ServerInterface();
	std::map<std::string, FSocket *>			m_socket_list;

	FSocket										m_listen;
	CHANDLE									    m_mapml;
	char *										recv_buffer;
};

