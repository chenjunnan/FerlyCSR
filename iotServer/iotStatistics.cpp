#include "iotStatistics.h"


iotStatistics::iotStatistics()
{
}


iotStatistics::~iotStatistics()
{
}

void iotStatistics::SetID(std::string ip, std::string id)
{
	std::unique_lock<std::mutex> g(m_mapl);
	m_state[ip].id = id;
}

void iotStatistics::Boxonline(std::string ip, std::string id)
{
	std::unique_lock<std::mutex> g(m_mapl);
	_online(ip, id);
	return;
}

void iotStatistics::_online(std::string ip, std::string id)
{
	unsigned long long nowtime = common_get_cur_time_stamps();
	if (m_state[ip].online)
		m_state[ip].offlinetime = m_state[ip].hasdatatime;

	m_state[ip].onlinetime = nowtime;
	m_state[ip].online = true;
	m_state[ip].id = id;
	if (m_state[ip].offlinetime == 0)
		return;

	if (!m_state[ip].changenet)
	{
		int offtime = nowtime - m_state[ip].offlinetime;
		m_state[ip].offlinesum.push_back(offtime);
		m_state[ip].toteloffline += (offtime);
	}
	m_state[ip].changenet = false;

}

void iotStatistics::changeNet(std::string ip)
{
	std::unique_lock<std::mutex> g(m_mapl);
	m_state[ip].changenet = true;
}

void iotStatistics::hasdata(std::string ip)
{
	std::unique_lock<std::mutex> g(m_mapl);
	if (m_state[ip].onlinetime == 0 || (m_state[ip].onlinetime != 0 && !m_state[ip].online))
		_online(ip, m_state[ip].id);
	
	m_state[ip].hasdatatime = common_get_cur_time_stamps();

}

void iotStatistics::Boxoffline(std::string ip)
{
	std::unique_lock<std::mutex> g(m_mapl);
	if (!m_state[ip].online)
		return;
	m_state[ip].offlinetime = m_state[ip].hasdatatime;
	m_state[ip].totelonline += (m_state[ip].offlinetime - m_state[ip].onlinetime);
	m_state[ip].online = false;
}

void iotStatistics::run()
{
	while (true)
	{
		Message msg = getq(5 * 1000);
		if (msg.msgType == _EXIT_THREAD_)
		{

			return;
		}
		m_mapl.lock();
		if (m_state.size() == 0)
		{
			m_mapl.unlock();
			continue;
		}
		FILE *fp=NULL;
		unsigned long long nowtime = common_get_cur_time_stamps();
		fp = fopen("iotstate.txt","w+");
		if (fp)
		{
			for each (auto var in m_state)
			{
				fprintf(fp, "\n%s \tid:%s\t%s持续%d(s)\t", var.first.c_str(), var.second.id.c_str(), var.second.online ? "在线" : "离线", var.second.online ? (nowtime-var.second.onlinetime):(nowtime-var.second.offlinetime));
				unsigned long long toteloff,totelon;
				
				if (var.second.online)
				{
					totelon = var.second.totelonline + (nowtime - var.second.onlinetime);
					toteloff = var.second.toteloffline;
				}
				else
				{
					totelon = var.second.totelonline ;
					toteloff = var.second.toteloffline + (nowtime - var.second.offlinetime);
				}
				
				fprintf(fp, "总共在线：%llu （s）\t总共离线：%llu（s）\t离线比率 %0.5f%\n离线时长：", totelon,toteloff, ((double)toteloff) / ((double)toteloff + totelon) * 100);
				for (auto it = var.second.offlinesum.begin(); it != var.second.offlinesum.end();it++)
					fprintf(fp, "%d（s） ",*it);
			}
			fclose(fp);
		}
		m_mapl.unlock();
	}

}
