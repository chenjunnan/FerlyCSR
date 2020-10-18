#pragma once
#include <list>
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\core\core.hpp"
#include "robot_face_detect_interface.h"
#include "robot_face_live_interface.h"
#ifdef HAS_YUNCONG
#include "CWFaceConfig.h"
#include "robot_face_live_detect_yuncong.h"
class robot_face_live_handle_yuncong :public robot_face_live_interface
{
public:
	robot_face_live_handle_yuncong();
	virtual ~robot_face_live_handle_yuncong();
	virtual int init();
	virtual bool open(std::string streamname);
	virtual void close_live();
	virtual bool handle_image(cv::Mat &image, robot_face_detect_interface *detecter = NULL);
	virtual void get_result(FaceDetectData &flist);

private:
	virtual void run();
	bool malloc_buffer();
	void get_stand_frame(unsigned char * buffer, unsigned int data_len, cv::Mat &frame);
	int GetNisLiveStateByMat(std::vector<cw_face_res_t> &visVec, std::vector<cw_face_res_t> &nirVec, FaceDetectData &facedata);
	void *m_pNisLiveHandle;  // ºìÍâ»îÌå¾ä±ú
	std::vector<unsigned char> image_buffer;
	robot_face_live_detect_yuncong detecter[2];
	CHANDLE  m_w;
	std::string m_streamname;
	std::vector<cw_face_res_t> m_result;
	FaceDetectData face_result;
	HANDLE		m_c;
	HANDLE       m_pm;
	bool m_congcastrate;
};

#endif // HAS_YUNCONG