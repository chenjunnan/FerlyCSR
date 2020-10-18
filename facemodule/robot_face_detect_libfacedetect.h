#pragma once
#include "robot_face_detect_interface.h"
#include <map>
#include <string>

typedef int* (*DETECT_FACE_FUN)(unsigned char * result_buffer, //buffer memory for storing face detection results, !!its size must be 0x20000 Bytes!!
	unsigned char * gray_image_data, int width, int height, int step, //input image, it must be gray (single-channel) image!
	float scale, //scale factor for scan windows
	int min_neighbors, //how many neighbors each candidate rectangle should have to retain it
	int min_object_width, //Minimum possible face size. Faces smaller than that are ignored.
	int max_object_width , //Maximum possible face size. Faces larger than that are ignored. It is the largest posible when max_object_width=0.
	int doLandmark );

//define the buffer size. Do not change the size!
#define DETECT_BUFFER_SIZE 0x20000


class robot_face_detect_libfacedetect :public robot_face_detect_interface
{
public:
	robot_face_detect_libfacedetect();
	virtual void setMode(std::string name);
	virtual bool DetectFace(cv::Mat &frame, FaceRect minface, std::vector <FaceRect> &face_postion, std::string ext = "");
	virtual ~robot_face_detect_libfacedetect();
private:
	std::map<std::string, DETECT_FACE_FUN> fun_list;
	std::string							   m_curfun;
	int										max_yaw;
	unsigned char m_result_buffer[DETECT_BUFFER_SIZE];

};

