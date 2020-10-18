#pragma once
#include "robot_face_struct.h"
#include "common_helper.h"
class robot_face_process_interface:public Thread_helper
{
public:
	virtual bool init(int no)=0;
	virtual void stop()=0;
	virtual void set_debug(bool flag)=0;
	virtual void set_identify(bool b)=0;
	virtual void set_faceinfo(bool b)=0;
	virtual int  start_detect_task(FaceMessage &msg) = 0;
	virtual void get_status(face_status_info &fd) = 0;
	virtual void stop_detect() = 0;
	virtual void start_wait_face_image(FaceMessage) = 0;
	virtual void start_wait_normal_image(FaceMessage) = 0;
	virtual void stop_wait_face_image() = 0;
	//virtual bool face_register(FaceMessage registerinfo) = 0;
	virtual ~robot_face_process_interface(){}
};


class robot_face_process_local_interface :public robot_face_process_interface
{
public:
	virtual void get_streamname(std::vector< std::string>&) = 0;
	virtual int  camera_message(int result, std::string name, FaceMessage &outmsg) = 0;
	virtual void get_processer_content(face_process_content& content) = 0;
	virtual ~robot_face_process_local_interface(){}
};
