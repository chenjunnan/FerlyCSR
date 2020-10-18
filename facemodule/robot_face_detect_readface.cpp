#include "robot_face_detect_readface.h"
#include <iostream>
#include <fstream> 

#ifdef HAS_READFACE
#include "RSFaceSDK.h"
#include "opencv2\imgproc\imgproc.hpp"
#include "robot_face_readface_singleton.h"
robot_face_detect_readface::robot_face_detect_readface()
{
	AuthLicense = false;

	m_PfaceTrack = NULL;

	m_pfaceLiveDet = NULL;

	m_pfaceDetect = NULL;

	m_pfaceAttribute = NULL;

	m_pfaceQuality = NULL;
}


void robot_face_detect_readface::setMode(std::string name /*= ""*/)
{
	if (AuthLicense)return;
	if (!robot_face_readface_singleton::getInstance()->isLicense())
		return;
	RSHandle m_Plicense = robot_face_readface_singleton::getInstance()->getRSHandle();

	rsInitFaceTrack(&m_PfaceTrack, m_Plicense);
	if (m_PfaceTrack == NULL)
	{
		LOGE("rsInitFaceTrack failed");
		return ;
	}

	rsInitLivenessDetect(&m_pfaceLiveDet, m_Plicense);
	if (m_pfaceLiveDet == NULL)
	{
		LOGE("rsInitLivenessDetect failed");
		return;
	}
	rsInitFaceDetect(&m_pfaceDetect, m_Plicense);
	if (m_pfaceDetect == NULL)
	{
		LOGE("rsInitFaceDetect failed");
		return;
	}
	rsInitAttributeDetect(&m_pfaceAttribute, m_Plicense);
	if (m_pfaceAttribute == NULL)
	{
		LOGE("rsInitAttributeDetect failed");
		return;
	}
	rsInitFaceQuality(&m_pfaceQuality, m_Plicense);
	if (m_pfaceQuality == NULL)
	{
		LOGE("rsInitFaceQuality failed");
		return;
	}

	AuthLicense = true;
}

bool robot_face_detect_readface::DetectFace(cv::Mat &frame, FaceRect minface, std::vector <FaceRect> &face_postion, std::string ext /*= ""*/)
{
	if (!AuthLicense)
		return false;

	face_postion.clear();
	
	unsigned long  startt = common_get_cur_time_stampms();


	rs_face *pFaceArray = NULL;
	int faceCount = 0;
	int result = rsRunFaceTrack(m_PfaceTrack, frame.data,
		PIX_FORMAT_BGR888,
		frame.cols,
		frame.rows,
		frame.step,
		RS_IMG_CLOCKWISE_ROTATE_0,
		&pFaceArray,
		&faceCount);
	if (result == -1 || faceCount <= 0)
	{
		return false;
	}

	for (int loop = 0; loop < faceCount; loop++)
	{
		rs_face mFace = pFaceArray[loop];
		FaceRect faces;



		faces.x = mFace.rect.left;
		faces.y = mFace.rect.top;
		faces.width = mFace.rect.width;
		faces.height = mFace.rect.height;

		faces.width = (faces.x + faces.width)>frame.cols ? (frame.cols - faces.x - 1) : faces.width;
		faces.height = (faces.y + faces.height)> frame.rows ? (frame.rows - faces.y - 1) : faces.height;


		if (faces.x<0 || faces.y<0 ||
			faces.width == 0 || faces.height == 0 ||
			faces.x + faces.width>frame.cols ||
			faces.y + faces.height>frame.rows)
		{
			continue;
		}

		int image_xcentor = frame.cols / 2;
		int image_ycentor = frame.rows / 2;

		double x_centor = 0, y_centor = 0;
		x_centor = faces.x + faces.width / 2;
		y_centor = faces.y + faces.height / 2;

		x_centor = x_centor / image_xcentor - 1;
		y_centor = y_centor / image_ycentor - 1;

		faces.xoffset = x_centor;
		faces.yoffset = y_centor;

		faces.trackId = mFace.trackId;

		faces.score = mFace.score;

		faces.pitch = mFace.pitch;
		faces.roll = mFace.roll;
		faces.yaw = mFace.yaw;
		faces.has3D = true;
	
		if (faces.score > 0.55&&abs(mFace.pitch) < 20 &&
			abs(mFace.roll) < 20 &&
			abs(mFace.yaw) < 20)
			faces.takephoto = true;

		if (faces.score>0.25)
			faces.valid = 1;
		rs_face_attr pAttrVal;

		int okret = rsRunAttributeDetect(m_pfaceAttribute, frame.data, PIX_FORMAT_BGR888, frame.cols, frame.rows, frame.step, &mFace, &pAttrVal);
		if (okret == 0)
		{
			faces.sex = pAttrVal.gender == -1 ? 0 : (pAttrVal.gender==0?2:1); // 0: female, 1: male, -1: not sure
			faces.age = pAttrVal.age;
			faces.smile = pAttrVal.smile;
			faces.beauty = pAttrVal.beauty;
			//faces.data.setDetectData("readface", &mFace, sizeof(rs_face));
		}

	//	printf("%d %0.5f %d %0.5f \n", faces.sex, faces.score, faces.trackId, mFace.confidence);

		face_postion.push_back(faces);

	}
	releaseFaceDetectResult(pFaceArray, faceCount);
	//LOGI("cost %d\tface %d ", common_get_cur_time_stampms() - startt, face_postion.size());

	if (face_postion.size() > 0)
		return true;
	return false;

}

robot_face_detect_readface::~robot_face_detect_readface()
{
	if (m_pfaceQuality != NULL)
	{
		rsUnInitFaceQuality(&m_pfaceQuality);
		m_pfaceQuality = NULL;
	}

	if (m_pfaceAttribute != NULL)
	{
		rsUnInitAttributeDetect(&m_pfaceAttribute);
		m_pfaceAttribute = NULL;
	}

	if (m_pfaceDetect != NULL)
	{
		rsUnInitFaceDetect(&m_pfaceDetect);
		m_pfaceDetect = NULL;
	}
	if (m_pfaceLiveDet != NULL)
	{
		rsUnInitLivenessDetect(&m_pfaceLiveDet);
		m_pfaceLiveDet = NULL;
	}

	if (m_PfaceTrack != NULL)
	{
		rsUnInitFaceTrack(&m_PfaceTrack);
		m_PfaceTrack = NULL;
	}


}

#endif // HAS_READFACE

