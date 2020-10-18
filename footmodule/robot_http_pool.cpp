#include "robot_http_pool.h"
#include "common_helper.h"

robot_http_pool* robot_http_pool::m_ins = NULL;
CHANDLE robot_http_pool::m_msinglelocker = common_mutex_create_threadlock();

robot_http_pool::robot_http_pool()
{
}


void robot_http_pool::init()
{

	for (int i = 0; i < MAX_POOL;i++)
	{
		m_http[i] =common_http_create();
		if (m_http[i] == NULL)
		{
			LOGE("create http error");
		}
		m_finish_map.push_back(false);
	}
	m_maplocker = common_mutex_create_threadlock();
}

robot_http_pool* robot_http_pool::getSingle()
{
	if (m_ins == NULL)
	{
		LOCK_HELPER_AUTO_HANDLER(m_msinglelocker);
		if (m_ins == NULL)
		{
			m_ins = new robot_http_pool();
			m_ins->init();
		}
	}
	return m_ins;
}

void robot_http_pool::ExitAll()
{
	for (int i = 0; i < MAX_POOL; i++)
	{
		if (m_http[i] != NULL)
			common_http_cancle(m_http[i]);
	}
}

bool robot_http_pool::getRequest(std::string url, VEC_BYTE&result, int timeout/*=5*/)
{
	int requestIndex = -1;
	requestIndex = AllocHttpIndex();
	if (requestIndex < 0)
	{
		LOGE("Full http ...");
		return false;
	}
	//printf("user %d http\n", requestIndex);
	bool ret = common_http_get(m_http[requestIndex], url.c_str(), &result, timeout);
	FreeHttpIndex(requestIndex);
	//printf("free %d http\n", requestIndex);
	if (!ret)
		return false;
	return true;
}

int robot_http_pool::AllocHttpIndex()
{
	LOCK_HELPER_AUTO_HANDLER(m_msinglelocker);
	int requestIndex = -1;
	for (int i = 0; i < m_finish_map.size(); i++)
	{
		if (m_finish_map[i] == false)
		{
			requestIndex = i;
			m_finish_map[i] = true;
			break;
		}
	}
	return requestIndex;
}


void robot_http_pool::FreeHttpIndex(int i)
{
	LOCK_HELPER_AUTO_HANDLER(m_msinglelocker);
	if (i > 0 && i < m_finish_map.size())
		m_finish_map[i] = false;
}

bool robot_http_pool::postRequest(std::string url, VEC_BYTE&vaule, VEC_BYTE&result, int timeout /*= 5*/)
{
	int requestIndex = -1;
	requestIndex = AllocHttpIndex();
	if (requestIndex < 0)
	{
		LOGE("Full http ...");
		return false;
	}
	//printf("user %d http\n",requestIndex);
	bool ret = common_http_post(m_http[requestIndex], url.c_str(), &vaule, &result, timeout);
	FreeHttpIndex(requestIndex);
	//printf("free %d http\n", requestIndex);
	if (!ret)
		return false;
	return true;
}

robot_http_pool::~robot_http_pool()
{
}
