#pragma once
#include <stdlib.h>
#include <string>
#include <vector>
#include "VideoStream.h"
#include <Windows.h>
#include "CameraDirect.h"
#include "DeviceNotify.h"

struct  CAMERA_MESSAGE
{
	int appid = 0;
	int messagetype = 0;
	std::string from;
	std::string context;
};



class VideoManager
{
public:
	static VideoManager *get();
	bool Init();
	bool update_device();
	bool remove_device();
	bool addvideo(VideoConfig vp);
	bool start();
	void stop_all();
	int message_handler(CAMERA_MESSAGE &cm);
	virtual ~VideoManager();
	bool video_start(int index);
	bool video_stop(int index);
private:	
	VideoManager();

	bool video_set(int index, int t, int v, bool a);
	bool video_get(int index, int t, int &v, bool &a);
	CHANDLE						m_ml;
	std::map<int, VideoStream*> m_videomap;
	std::map<std::string, std::set<std::string> > m_listenlist;
	std::map<std::string, int> m_stream_name_id;
	std::vector<Camera> m_devicelist;
	CDeviceNotify    m_dnotify;

};

