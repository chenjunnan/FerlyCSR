#include "robot_face_detect_yuncong.h"
#ifdef HAS_YUNCONG
#include "CWFaceConfig.h"
#include "CWFaceDetection.h"
#include "CWFaceVersion.h"
#include "CWFaceAttribute.h"
#include "robot_face_struct.h"
#include "xCommon.h"



robot_face_detect_yuncong::robot_face_detect_yuncong()
{
	Attribute_object = NULL;
	detect_object=NULL;
	min_face_value = 0;
	param = CW_OP_ALL;
}



void robot_face_detect_yuncong::setMode(std::string name)
{
	char version[1024] = { 0 };
	cw_errcode_t ret = cwGetFaceSDKVersion(version, 1024);
	if (ret != CW_SDKLIT_OK)
	{
		LOGE("yun cong cwGetFaceSDKVersion error:%d.", ret);
		return;
	}
	LOGI("yun cong sdk version :%s.", version);
	int max_handle = cwGetMaxHandlesNum(NULL);
	LOGI("yun cong max handles %d.",max_handle);

	if (name.compare("image") == 0)
		param = CW_OP_DET | CW_OP_KEYPT | CW_OP_ALIGN | CW_OP_QUALITY;
}

bool robot_face_detect_yuncong::DetectFace(cv::Mat &frame, FaceRect minface, std::vector <FaceRect> &face_postion, std::string ext)
{
	face_postion.clear();
	if (frame.empty() || frame.data == nullptr || frame.rows == 0 || frame.cols == 0)
		return false;

	int new_minface = (minface.width + minface.height) / 2;
	if (new_minface != min_face_value)
	{
		release_yuncong_handle();

		cw_errcode_t errCode;
		detect_object = cwCreateDetHandle(&errCode, DETECT_YUNCONG_MODLE, LICENCE);
		if (detect_object&&errCode == CW_SDKLIT_OK)
		{
			cw_det_param_t param;
			cwGetFaceParam(detect_object, &param);
			param.minSize = new_minface;
			min_face_value = new_minface;
			param.maxSize = 1000;
			param.pConfigFile = DETECT_YUNCONG_MODLE;    // 设置接口功能参数
			cwSetFaceParam(detect_object, &param);
			cwSetFaceBufOrder(detect_object, 1);
		}
		else
		{
			LOGE("create yun cong handle failed %d.", errCode);
			return false;
		}
		Attribute_object = cwCreateAttributeHandle(&errCode, FACEATTRICONFIG_YUNCONG_MODLE, NULL);
		if (errCode != CW_SDKLIT_OK || !Attribute_object)
		{
			LOGE("yun cong cwCreateAttributeHandle error:%d.", errCode);
			return false;
		}
	}
	if (detect_object==NULL)
		return false;

	int detectparam=param;
	if (ext.compare("image") == 0)
		detectparam = CW_OP_DET | CW_OP_KEYPT | CW_OP_ALIGN | CW_OP_QUALITY;
	if (ext.compare("live") == 0)
		detectparam = CW_OP_DET | CW_OP_KEYPT | CW_OP_QUALITY;

	cw_img_t srcImg;
	srcImg.frameId = 0;
	srcImg.data = (char*)frame.data;
	//srcImg.dataLen = frame.total();
	srcImg.width = frame.cols;
	srcImg.height = frame.rows;
	srcImg.angle = CW_IMAGE_ANGLE_0;
	srcImg.format = CW_IMAGE_BGR888;
	srcImg.mirror = CW_IMAGE_MIRROR_NONE;

	if (frame.channels() == 1)
	{
		srcImg.format = CW_IMAGE_GRAY8;
	}
	else if (frame.channels() == 3)
	{
		srcImg.format = CW_IMAGE_BGR888;
	}
	else if (frame.channels() == 4)
	{
		srcImg.format = CW_IMAGE_BGRA8888;
	}
	else
	{
		return false;
	}
	cw_face_res_t faceBuffers[MAX_NUM_FACES];
	int iFaceNum = 0;


	unsigned long startt = common_get_cur_time_stampms();
	cw_errcode_t errDet = cwFaceDetection(detect_object, &srcImg, faceBuffers, MAX_NUM_FACES, &iFaceNum, detectparam);

	//printf("cost1 %d\tface \n", common_get_cur_time_stampms() - startt);
	if (errDet != CW_SDKLIT_OK)
	{
		LOGE("Face detect Error, Code:  %d", errDet);
		return false;
	}
	if (iFaceNum < 1)
	{
		return false;
	}
	int facecnt = 0;

	bool track_person = false;


	for (int i = 0; i < iFaceNum; i++)
	{
		FaceRect faces;

		faces.x = faceBuffers[i].faceRect.x;
		faces.y = faceBuffers[i].faceRect.y;
		faces.width = faceBuffers[i].faceRect.width;
		faces.height = faceBuffers[i].faceRect.height;
		//faces.score = faceBuffers[i].quality.scores[0];
		//printf("score:%d", (int)faces.score);
		faces.width = (faces.x + faces.width)>frame.cols ? (frame.cols - faces.x-1) : faces.width;
		faces.height = (faces.y + faces.height)> frame.rows ? (frame.rows - faces.y-1) : faces.height;


		if (faces.x<0 || faces.y<0 ||
			faces.width == 0 || faces.height == 0 ||
			faces.x + faces.width>frame.cols ||
			faces.y + faces.height>frame.rows)
		{
			continue;
		}
	
		int image_xcentor = srcImg.width / 2;
		int image_ycentor = srcImg.height / 2;

		double x_centor = 0,y_centor=0;
		x_centor = faces.x + faces.width / 2;
		y_centor = faces.y + faces.height / 2;

		x_centor = x_centor / image_xcentor-1;
		y_centor = y_centor / image_ycentor-1;

		faces.xoffset= x_centor;
		faces.yoffset = y_centor;

		faces.trackId = faceBuffers[i].trackId;

		if (faceBuffers[i].quality.errcode == CW_QUALITY_OK)
		{
			faces.score = faceBuffers[i].quality.scores[0];
		}
			


		if (faceBuffers[i].detected == 1 && faceBuffers[i].quality.errcode == CW_QUALITY_OK)
		{
			if (abs(faceBuffers[i].quality.scores[6]) > 0.5 &&
				abs(faceBuffers[i].quality.scores[7]) > 0.5 && faceBuffers[i].quality.scores[16] > 0.11&& faceBuffers[i].quality.scores[2] > 0.15)
				faces.takephoto = true;

			faces.pitch = faceBuffers[i].quality.scores[4];
			faces.yaw = faceBuffers[i].quality.scores[3];
			faces.roll = faceBuffers[i].quality.scores[5];
			faces.has3D = true;

		}

		if (faceBuffers[i].detected == 1|| param != CW_OP_ALL)
		{
			int age = 0, sex = 0;
			cw_errcode_t errCode = cwGetAgeGenderEval(Attribute_object, &faceBuffers[i].faceAligned, &age, &sex);
			if (errCode == CW_SDKLIT_OK)
			{
				faces.age = age;
				faces.sex = sex;
			}
			faces.valid = 1;
			faces.data.setDetectData("yuncong", &faceBuffers[i], sizeof(cw_face_res_t));
		}

		face_postion.push_back(faces);
		facecnt++;

	}

	//printf("cost %d\tface %d \n", common_get_cur_time_stampms() - startt, face_postion.size());
	
	if (facecnt > 0)
	{
		return true;
	}
	return false;


}

void robot_face_detect_yuncong::release_yuncong_handle()
{
	if (detect_object)
	{
		cwReleaseDetHandle(detect_object);
		detect_object = NULL;
	}
	if (Attribute_object)
	{
		cwReleaseAttributeHandle(Attribute_object);
		Attribute_object = NULL;
	}
}

robot_face_detect_yuncong::~robot_face_detect_yuncong()
{
	release_yuncong_handle();
}

#endif // HAS_YUNCONG