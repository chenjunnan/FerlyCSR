#pragma once
#include "opencv2\highgui\highgui.hpp"
#include "common_helper.h"
#include "memorySwaper.h"
#include "robot_face_detect_interface.h"

#ifdef HAS_YUNCONG
#include "CWFaceConfig.h"
class robot_face_live_detect_yuncong :
	public Thread_helper
{
public:
	robot_face_live_detect_yuncong();
	virtual ~robot_face_live_detect_yuncong();
	int init();
	bool open();
	void wait_task();
	void add_task(cv::Mat &image);
	void get_result(std::vector<cw_face_res_t >&list);
	virtual void run();
private:
	int get_faceinfo_bymat(const cv::Mat &matImage, cw_face_res_t* pFaceBuffer);
	void *m_pDetHandle;      // ¼ì²â¾ä±ú
	CHANDLE  m_w;
	HANDLE   m_c;
	std::vector<cw_face_res_t> m_res;
};
#endif // HAS_YUNCONG
