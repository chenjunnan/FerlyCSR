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

	// ÈËÁ³×·×Ù¾ä±ú
	RSHandle m_PfaceTrack;
	// »îÌå¼ì²â¾ä±ú
	RSHandle m_pfaceLiveDet;
	// ÈËÁ³¼ì²â¾ä±ú
	RSHandle m_pfaceDetect;
	// ÈËÁ³ÊôÐÔ¼ì²â¾ä±ú
	RSHandle m_pfaceAttribute;
	// ÈËÁ³ÖÊÁ¿¼ì²â¾ä±ú
	RSHandle m_pfaceQuality;
};


#endif // HAS_READFACE

