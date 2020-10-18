#include "cloud_mq_thread.h"
#include "utils.h"


cloud_mq_thread::cloud_mq_thread(amqp_connection_state_t amqphandle, cloud_mq_interface *cb) : m_conn(amqphandle), m_cb(cb)
{

}

cloud_mq_thread::~cloud_mq_thread()
{
}

bool cloud_mq_thread::Bind_Route(const char *ququname,const char *bindingkey, const char *exchange)
{
	amqp_bytes_t queuename;
	m_queue_name = ququname;
	m_bindingkey = bindingkey;
	m_exchange = exchange;

	queuename=amqp_cstring_bytes(ququname);

	amqp_queue_bind_ok_t *ret=amqp_queue_bind(m_conn, 1, queuename, amqp_cstring_bytes(exchange),
		amqp_cstring_bytes(bindingkey), amqp_empty_table);


	if (!die_on_amqp_error(amqp_get_rpc_reply(m_conn), "Binding queue"))
	{
		return false;
	}
		


	return true;
}

void cloud_mq_thread::run()
{
	std::vector<char> temp;
	amqp_bytes_t queuename;
	queuename = amqp_cstring_bytes(m_queue_name.c_str());
	Timer_helper cleantime;


	do 
	{
		amqp_basic_qos(m_conn, 1, 0, 1, 1);
		if (!die_on_amqp_error(amqp_get_rpc_reply(m_conn), "QOS"))
			break;
		amqp_basic_consume(m_conn, 1, queuename, amqp_empty_bytes, 1, 0,0,
			amqp_empty_table);

		if (!die_on_amqp_error(amqp_get_rpc_reply(m_conn), "Consuming"))
			break;

		while (1)
		{

			Message ms = getq(0);
			if (ms.msgType == _EXIT_THREAD_)break;

			timeval to;
			to.tv_sec = 1;
			to.tv_usec = 0;
			amqp_rpc_reply_t res;
			amqp_envelope_t envelope;


			amqp_maybe_release_buffers(m_conn);

			res = amqp_consume_message(m_conn, &envelope, &to, 0);
			printf("MQ get message ret :%d [%d]\n", res.reply_type, res.library_error);

			if (res.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION && res.library_error == AMQP_STATUS_TIMEOUT)
			{
				amqp_destroy_envelope(&envelope);
				if (cleantime.isreach(1000 * 60*30))
					break;
			}
			else
			{
				if (AMQP_RESPONSE_NORMAL != res.reply_type) {
					LOGE("[MQ] Call amqp_consume_message failed. "
						"ret.reply_type[%d] ret.reply.id[%x] ret.library_error[-0x%x:%s].",
						res.reply_type,
						res.reply.id,
						0 - res.library_error,
						amqp_error_string2(res.library_error));
					amqp_destroy_envelope(&envelope);
					break;
				}

		
				temp.assign((char*)envelope.message.body.bytes, (char*)envelope.message.body.bytes + envelope.message.body.len);


				int ack_ret = amqp_basic_ack(m_conn, 1, envelope.delivery_tag, 0);
				if (ack_ret != 0)
				{
					LOGE("[MQ] ACK error:%d", ack_ret);
					amqp_destroy_envelope(&envelope);
	
					break;
				}
				if (m_cb)m_cb->MessageIn(temp);

				cleantime.update();
	

				LOGI("Delivery %u, exchange %.*s routingkey %.*s\n",
					(unsigned)envelope.delivery_tag, (int)envelope.exchange.len,
					(char *)envelope.exchange.bytes, (int)envelope.routing_key.len,
					(char *)envelope.routing_key.bytes);

				if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
					LOGI("Content-type: %.*s\n",
						(int)envelope.message.properties.content_type.len,
						(char *)envelope.message.properties.content_type.bytes);
				}


				amqp_destroy_envelope(&envelope);
			}
		


		}

		amqp_queue_unbind_ok_t *unbindret = amqp_queue_unbind(m_conn, 1, queuename,
			amqp_cstring_bytes(m_exchange.c_str()), amqp_cstring_bytes(m_bindingkey.c_str()), amqp_empty_table);

		die_on_amqp_error(amqp_get_rpc_reply(m_conn), "Unbinding");


	} while (0);



	LOGW("Exit MQ Clint");
	m_cb->ThreadQuit();
}
