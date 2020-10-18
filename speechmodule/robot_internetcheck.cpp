#include "robot_internetcheck.h"
#include "xCommon.h"
#include <windows.h>
#include <wininet.h>

robot_internetcheck::robot_internetcheck(internet_notify *ine) :m_timeout(10)
{
	m_http = common_http_create();
	common_http_set_log(m_http, true);
	m_cb = ine;
}

void robot_internetcheck::setTimeout(int timeout)
{
	m_timeout = timeout*1000;
}

robot_internetcheck::~robot_internetcheck()
{
	common_http_release(m_http);
}

std::string robot_internetcheck::imp_get(std::string url, int t)
{
	std::string out;
	VEC_BYTE vec_str;
	if (!common_http_get(m_http, url.c_str(), &vec_str, t))
		return out;

	out.insert(out.end(), vec_str.begin(), vec_str.end());
	return out;

}

void robot_internetcheck::run()
{
	std::string url = "http:\/\/www.baidu.com\/";
	bool connect_status=true;
	
	while (1)
	{
		Message msg = getq(m_timeout);
		if (msg.msgType == _EXIT_THREAD_)break;

		bool connect_now = false;;
		connect_now = InternetCheckConnection(url.c_str(), 1, 0);


// 
// 		std::string reponse = imp_get(url, m_timeout);
// 		if (!reponse.empty())connect_now = true;

		if (connect_now != connect_status)
		{
			m_cb->netnotify(connect_now);
		}


		connect_status = connect_now;
	}
}
