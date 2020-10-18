
#include "robot_voice_media.h"
#include <string.h>
#include "common_helper.h"

robot_voice_media::robot_voice_media()
{
	m_media.resize(32);
	m_time = 0;
}

robot_voice_media::robot_voice_media(unsigned int size)
{
	m_media.resize(size);
	m_time = 0;
}

robot_voice_media::robot_voice_media(time_t stime, unsigned char *buffer, unsigned int len)
{
	Set(stime, buffer, len);
}

robot_voice_media::robot_voice_media(robot_voice_media& rvm)
{

	m_media.resize(rvm.m_media.size());
	m_media = rvm.m_media;
	m_time = rvm.m_time;
}


robot_voice_media* robot_voice_media::operator=(robot_voice_media& rvm)
{
	m_media.resize(rvm.m_media.size());
	m_media = rvm.m_media;
	m_time = rvm.m_time;
	return this;
}

robot_voice_media::~robot_voice_media()
{

}

void robot_voice_media::Set(time_t stime, unsigned char *buffer, unsigned int len)
{
	m_media.resize(len);
	m_media.assign(buffer, buffer + len);
	m_time = stime;
}

unsigned char * robot_voice_media::GetMedia()
{
	return m_media.data();
}

void robot_voice_media::Resize(unsigned int size)
{
	m_media.resize(size);
	m_time = 0;
}


unsigned int robot_voice_media::GetMediaLen()
{
	return m_media.size();
}

time_t robot_voice_media::GetMediaTime()
{
	return m_time;
}
