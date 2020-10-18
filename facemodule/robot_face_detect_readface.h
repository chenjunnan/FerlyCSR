#pragma once
#include "robot_face_detect_interface.h"

#ifdef HAS_READFACE
#include "RSCommon.h"
#include "RSFaceSDK.h"
class robot_face_detect_readface :public robot_face_detect_interface
{
public:
	robot_face_detect_readface();
	
	virtual void setMode(std::string name = "");
	virtual bool DetectFace(cv::Mat &frame, FaceRect minface, std::vector <FaceRect> &face_postion, std::string ext = "");
	virtual ~robot_face_detect_readface();

private:

	bool AuthLicense;

	// ����׷�پ��
	RSHandle m_PfaceTrack;
	// ��������
	RSHandle m_pfaceLiveDet;
	// ���������
	RSHandle m_pfaceDetect;
	// �������Լ����
	RSHandle m_pfaceAttribute;
	// �������������
	RSHandle m_pfaceQuality;
};


#endif // HAS_READFACE

