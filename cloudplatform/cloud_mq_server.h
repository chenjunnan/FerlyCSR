#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <string>
#include "cloud_mq_thread.h"
#include "cloud.h"


class cloud_mq_server
{
public:
	cloud_mq_server(cloud_mq_interface *cb=NULL);
	int MQ_Getstate();
	int MQ_Login(const char *ip, const char *port,
		const char *user = "guest", const char *password = "guest",bool heart_on = false);
	bool GetQueueMessage(const char *queuename, const char * bindingkey, const char * exchange = "amq.direct");
	void MQ_Exit();
	int MQ_Push(std::string routing_key, std::vector<char> &data, std::string exchange = "amq.direct");
	~cloud_mq_server();
private:
	void destory();
	amqp_socket_t *m_socket;
	amqp_connection_state_t m_conn;
	cloud_mq_thread *_thread;
	cloud_mq_interface *m_cb;
	std::string  m_user, m_password;
	std::string 	m_hostname;
	int m_port, status;
};
