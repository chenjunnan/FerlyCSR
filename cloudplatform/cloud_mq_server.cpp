#include "cloud_mq_server.h"
#include "utils.h"

cloud_mq_server::cloud_mq_server(cloud_mq_interface *cb) :m_cb(cb)
{
	m_port = 0;
	m_socket = NULL;
	_thread = NULL;
}


int cloud_mq_server::MQ_Getstate()
{
	return m_conn == NULL ? 0 : 1;
}

int cloud_mq_server::MQ_Login(const char *ip, const char *port,
	const char *user /*= "guest"*/,const char *password /*= "guest"*/, bool heart_on /*= false*/)
{
	destory();

	m_hostname = ip;
	m_port = atoi(port);
	m_user = user;
	m_password = password;
	bool success = true;

	do 
	{
		amqp_rpc_reply_t ret;

		m_conn = amqp_new_connection();
		CHECK_MQRET(m_conn,"[MQ] create failed")

		m_socket = amqp_tcp_socket_new(m_conn);
		CHECK_MQRET(m_socket, "[MQ] creating TCP socke")

		status = amqp_socket_open(m_socket, m_hostname.c_str(), m_port);
		CHECK_MQRET(!status, "[MQ] opening TCP socke")


		ret = amqp_login(m_conn, "/", AMQP_DEFAULT_MAX_CHANNELS, AMQP_DEFAULT_FRAME_SIZE, heart_on ? 10 : 0, AMQP_SASL_METHOD_PLAIN,
			m_user.c_str(), m_password.c_str());

		if (!die_on_amqp_error(ret, "Logging in"))
		{
			success = false; 
			break;
		}

		CHECK_MQRET(ret.reply_type == AMQP_RESPONSE_NORMAL, "[MQ] Logging in")

		amqp_channel_open_ok_t *channelr = amqp_channel_open(m_conn, 1);
		ret = amqp_get_rpc_reply(m_conn);

		if (!die_on_amqp_error(ret, "Opening channel"))
		{
			success = false;
			break;
		}

		CHECK_MQRET(ret.reply_type == AMQP_RESPONSE_NORMAL, "[MQ] Opening channel")

	} while (0);

	if (!success)
	{
		destory();
		return -1;

	}


	return 0;
}

bool cloud_mq_server::GetQueueMessage(const char *queuename,const char * bindingkey, const char * exchange /*= "amq.direct"*/)
{
	if (!m_cb)
		return false;
	_thread = new cloud_mq_thread(m_conn, m_cb);
	if (!_thread->Bind_Route(queuename, bindingkey, exchange))
		return false;
	return _thread->start();
}

void cloud_mq_server::MQ_Exit()
{
	if (_thread)
	{
		if (_thread->state())
			_thread->quit();
		SAFE_DELETE(_thread);
	}
	destory();
}

int cloud_mq_server::MQ_Push(std::string routing_key, std::vector<char> &data, std::string exchange/*="amq.direct"*/)
{
	if (!m_conn) return -1;
	uint64_t start_time = now_microseconds();

	int sent = 0;
	int previous_sent = 0;

	amqp_bytes_t message_bytes;

	message_bytes.len = data.size();
	message_bytes.bytes = data.data();
	int rt = amqp_basic_publish(m_conn, 1, amqp_cstring_bytes(exchange.c_str()),
		amqp_cstring_bytes(routing_key.c_str()), 0, 0, NULL,
		message_bytes);

	if (rt == AMQP_STATUS_OK)
		return 0;
	else
	{
		LOGE("[MQ] send error %d %s", rt, amqp_error_string2(rt));
		return 1;
	}
}

cloud_mq_server::~cloud_mq_server()
{
	destory();
}

void cloud_mq_server::destory()
{
	m_port = 0;
	m_socket = NULL;
	if (m_conn)
	{
		LOGW("destory MQ");
		die_on_amqp_error(amqp_channel_close(m_conn, 1, AMQP_REPLY_SUCCESS),
			"Closing channel");
		die_on_amqp_error(amqp_connection_close(m_conn, AMQP_REPLY_SUCCESS),
			"Closing connection");
		die_on_error(amqp_destroy_connection(m_conn), "Ending connection");
		m_conn = NULL;
	}
}
