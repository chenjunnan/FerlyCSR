#pragma once
#include "common_helper.h"
#include "memorySwaper.h"
#include "opencv2/opencv.hpp"
#include "opencv2\highgui\highgui_c.h"
#include "camerastreamstruct.h"
#include "CameraDirect.h"



class VideoStream :public Thread_helper
{
public:
	VideoStream(VideoConfig vc);
	bool init_memory();
	bool open();
	bool set(int ,int,bool );
	bool get(int, int&, bool&);
	void getConfig(VideoConfig& vc);
	virtual ~VideoStream();
	bool   online;
private:
	bool open_byopencv();
	virtual void run();
	void close_device();
	VideoConfig m_config;
	HANDLE m_pm;
	cv::VideoCapture m_videocap;
	CvCapture       * m_video;
	CameraDirect m_cd;
};

