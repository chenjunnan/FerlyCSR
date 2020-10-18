#pragma once
#include "common_helper.h"
#include <string>
#include "memorySwaper.h"
#include <list>
#include "robot_voice_media.h"

#define  DEFINE_SAMPLE_RATE			16000
#define  MEDIA_METE				    8


class robot_voice_data_interface
{
public:
	robot_voice_data_interface(){}
	virtual void inFlush() = 0;
	virtual void inData(robot_voice_media &data) = 0;
	virtual ~robot_voice_data_interface(){};
protected:
private:
};


class robot_media_publish :public Thread_helper
{
public:
	robot_media_publish(robot_voice_data_interface*);

	int init(std::string streamname,std::string client_name);

	void close_memory();

	int get_samplerate();

	virtual ~robot_media_publish();
	
private:
	virtual void run();


	std::string		m_streamstring;
	std::string		m_name;

	HANDLE			m_pm;
	int             m_samplerate;

	robot_voice_media  m_buffer;

	robot_voice_data_interface *    m_client;
};

