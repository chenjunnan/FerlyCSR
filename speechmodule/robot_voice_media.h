#pragma once
#include <crtdefs.h>
#include <vector>
class robot_voice_media
{
public:
	robot_voice_media();
	robot_voice_media(unsigned int);
	robot_voice_media(time_t stime,unsigned char *buffer,unsigned int );
	robot_voice_media(robot_voice_media&);
	robot_voice_media* operator=(robot_voice_media&);
	~robot_voice_media();
	void Set(time_t stime, unsigned char *buffer, unsigned int);
	void  Resize(unsigned int size);
	unsigned char*  GetMedia();
	unsigned int GetMediaLen();
	time_t GetMediaTime();
private:
	std::vector<unsigned char>  m_media;
	time_t       m_time;
};

