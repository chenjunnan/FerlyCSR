#pragma once
#include "robot_face_detect_interface.h"
#ifdef HAS_YUNCONG
class robot_face_detect_yuncong :public robot_face_detect_interface
{
public:
	robot_face_detect_yuncong();
	virtual void setMode(std::string name);
	virtual bool DetectFace(cv::Mat &frame, FaceRect minface, std::vector <FaceRect> &face_postion, std::string ext = "");

	virtual ~robot_face_detect_yuncong();
private:
	void release_yuncong_handle();
	void *detect_object;
	void *Attribute_object;
	int min_face_value;
	int param;
};

#endif // HAS_YUNCONG