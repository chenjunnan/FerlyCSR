#pragma once
#include "common_helper.h"
#include <amqp.h>
#include "cloud.h"

class cloud_mq_thread :public Thread_helper
{
public:
	cloud_mq_thread(amqp_connection_state_t amqphandle, cloud_mq_interface *cb);
	virtual ~cloud_mq_thread();
	bool Bind_Route(const char *ququname, const char *bindingkey, const char *exchange = "amq.direct");
	virtual void run();
private:
	cloud_mq_interface *m_cb;
	amqp_connection_state_t m_conn;
	std::string m_queue_name;
	std::string m_bindingkey;
	std::string m_exchange;
};
