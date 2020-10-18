#pragma once
#include "common_helper.h"
#include "VideoStreamReader.h"
class PsycheRTCAgoraVideoPub:public Thread_helper
{
public:
	PsycheRTCAgoraVideoPub();
	int StartUp(std::string streamname);
	virtual ~PsycheRTCAgoraVideoPub();
private:
	VideostreamReader m_reader;
	virtual void run();
};

