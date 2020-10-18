#pragma once
#include "robot_face_detect_interface.h"
#include "SeetaFace\FaceDetection\include\face_detection.h"
class robot_face_detect_seetaface :public robot_face_detect_interface
{
public:
	robot_face_detect_seetaface();
	virtual void setMode(std::string name);
	virtual bool DetectFace(cv::Mat &frame, FaceRect minface, std::vector <FaceRect> &face_postion, std::string ext = "");
	virtual ~robot_face_detect_seetaface();
private:
	double max_yaw;
	seeta::FaceDetection *m_detector;
};

