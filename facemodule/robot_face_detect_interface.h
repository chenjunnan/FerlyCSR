#pragma once
#include "robot_face_struct.h"

class robot_face_detect_interface
{
public:
	robot_face_detect_interface(){ 	
	};
	virtual void setMode(std::string name=""){};
	virtual bool DetectFace(cv::Mat &frame, FaceRect minface, std::vector <FaceRect> &face_postion,std::string ext="") = 0;
	virtual ~robot_face_detect_interface(){};

};

