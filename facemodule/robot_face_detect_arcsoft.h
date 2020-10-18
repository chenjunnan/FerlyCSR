#pragma once
#include "robot_face_detect_interface.h"
#ifdef HAS_ARCSOFT

#include "arcsoft_face_sdk.h"
#include "merror.h"

class robot_face_detect_arcsoft :public robot_face_detect_interface
{
public:
	robot_face_detect_arcsoft();
	virtual void setMode(std::string name);
	virtual bool DetectFace(cv::Mat &frame, FaceRect minface, std::vector <FaceRect> &face_postion, std::string ext = "");

	bool init_engine(int minface_th);

	virtual ~robot_face_detect_arcsoft();

	void destory_handle();

private:
	MHandle handle;
	MHandle handle_image;
	bool  isInitFailed;
};


#endif // HAS_ARCSOFT
