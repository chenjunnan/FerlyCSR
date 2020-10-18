#pragma once
#include "robot_face_struct.h"
#include "common_helper.h"
#include "opencv2\core\core.hpp"
#include "robot_face_detect_interface.h"


class robot_face_live_interface :
	public Thread_helper
{
public:
	robot_face_live_interface(){};
	virtual int init()=0;
	virtual bool open(std::string streamname)=0;
	virtual void close_live()=0;
	virtual bool handle_image(cv::Mat &image, robot_face_detect_interface *detecter=NULL) = 0;
	virtual void get_result(FaceDetectData &flist)=0;
	virtual ~robot_face_live_interface(){ /*if (state())quit();*/ };
};



