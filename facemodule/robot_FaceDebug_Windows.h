#pragma once
#include <windows.h>
#include "common_helper.h"
#include <opencv2\core\core.hpp>
class robot_FaceDebug_Windows :public Thread_helper
{
public:
	robot_FaceDebug_Windows();
	void ShowMat(cv::Mat &image);

	bool Init(std::string windowsname);
	void exitWindows();
	virtual ~robot_FaceDebug_Windows();
	void Show();
private:
	void ShowVideo(HDC hdc, char* szFrame, int iImgWidth, int iImgHeight);
	virtual void run();
	std::string m_windowstitle;
	std::string m_windowsclassname;
	HWND m_hWnd;
	CHANDLE citr_cache;
	cv::Mat m_cache;
};

