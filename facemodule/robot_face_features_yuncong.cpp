#include "robot_face_features_yuncong.h"
#ifdef HAS_YUNCONG
#include "CWFaceConfig.h"
#include "CWFaceDetection.h"
#include "CWFaceRecognition.h"
#include "common_helper.h"
#include "opencv2\highgui\highgui.hpp"
#include "CWFaceAttribute.h"


static int i_save = 2001;
robot_face_features_yuncong::robot_face_features_yuncong()
{
	detect_object = NULL;
	feature_object = NULL;
	Attribute_object = NULL;
	match_object = NULL;
	m_congcastrate = false;
	m_facesize_th=1000;
	m_score_th=0.3;
	updatetime = 60;
	m_algname = "yuncong";
}


void robot_face_features_yuncong::release_yuncong_handle()
{
	if (detect_object)
	{
		cwReleaseDetHandle(detect_object);
		detect_object = NULL;
	}


	if (feature_object)
	{
		cwReleaseRecogHandle(feature_object);
		feature_object = NULL;
	}

	if (match_object)
	{
		cwReleaseRecogHandle(match_object);
		match_object = NULL;
	}


	if (Attribute_object)
	{
		cwReleaseAttributeHandle(Attribute_object);
		Attribute_object = NULL;
	}


}

int  robot_face_features_yuncong::init()
{
	cw_errcode_t errCode;
	release_yuncong_handle();

	int fun = 0;
	if (!common_config_get_field_int("yuncong", "castrate", &fun))
		fun = 0;

	if (fun)
	{
		if (!common_config_get_field_int("yuncong", "castratesize", &m_facesize_th))
			m_facesize_th = 1000;


		if (!common_config_get_field_int("yuncong", "castratefeatureinv", &updatetime))
			updatetime = 60;

		m_congcastrate = true;
	}
	int mode = 0;
	if (!common_config_get_field_int("yuncong", "identifymode", &mode))
		mode = 0;

	std::string mode_str = FACECONFIG_YUNCONG_MODLE;
	if (mode)mode_str = FACECONFIG_YUNCONG_MODLE_N;

	if (!m_congcastrate)
	{
		detect_object = cwCreateDetHandle(&errCode, DETECT_YUNCONG_MODLE, LICENCE);
		if (detect_object&&errCode == CW_SDKLIT_OK)
		{
			cw_det_param_t param;
			cwGetFaceParam(detect_object, &param);
			param.minSize = 30;
			param.maxSize = 1000;
			param.pConfigFile = DETECT_YUNCONG_MODLE;    // 设置接口功能参数
			cwSetFaceParam(detect_object, &param);
			cwSetFaceBufOrder(detect_object, 1);
		}
		else
		{
			LOGE("create yun cong detect handle failed %d.", errCode);
			return 1;
		}
		match_object = cwCreateRecogHandle(&errCode, mode_str.c_str(), LICENCE, CW_RECOGNITION);
		if (match_object == NULL || errCode != CW_SDKLIT_OK)
		{
			LOGE("create yun cong match handle failed %d.", errCode);
			return 1;
		}
	}

	feature_object = cwCreateRecogHandle(&errCode, mode_str.c_str(), LICENCE, CW_FEATURE_EXTRACT);
	if (feature_object == NULL || errCode != CW_SDKLIT_OK)
	{
		LOGE("create yun cong feature handle failed %d.", errCode);
		return 1;
	}

	return 0;
}

bool robot_face_features_yuncong::getFaceID_Region(int type,int *one_index, FaceDetectData &fdata, cw_img_t &srcImg, cw_face_res_t*faceBuffers)
{
	if (fdata.face_list.size() < 1)
		return false;
		

	cv::Rect rect;
	int maxindex = getMainFaceIndex(fdata, one_index);
	if (maxindex < 0)return false;

	if (m_congcastrate)
	{
		if (type == 0 || fdata.face_list[maxindex].height*fdata.face_list[maxindex].width > m_facesize_th &&
			fdata.face_list[maxindex].takephoto&&fdata.face_list[maxindex].score > 0.65)
		{
			std::vector<char> feature;
			if (fdata.face_list[maxindex].data.getDetectData("yuncong", feature) && feature.size() >= sizeof(cw_face_res_t))
			{
				memcpy(faceBuffers, feature.data(), sizeof(cw_face_res_t));
				return true;
			}
		}
		else
		{
			return false;
		}
			
	}

	if (!detect_object)
		return false;

	cv::Mat image_roi;

	int range_add = 100;

	if (fdata.face_list.size() != 1)
	{
		range_add = 60;

		rect.x = fdata.face_list[maxindex].x - range_add < 0 ? 0 : fdata.face_list[maxindex].x - range_add;
		rect.y = fdata.face_list[maxindex].y - range_add < 0 ? 0 : fdata.face_list[maxindex].y - range_add;

		rect.width = ((rect.x + fdata.face_list[maxindex].width) + 2*range_add) >= fdata.face_img.cols ?
			(fdata.face_img.cols - rect.x - 1) : ((fdata.face_list[maxindex].width) + 2*range_add);

		rect.height = ((rect.y + fdata.face_list[maxindex].height) + 2*range_add) >= fdata.face_img.rows ?
			(fdata.face_img.rows - rect.y - 1) : ((fdata.face_list[maxindex].height) + 2*range_add);

		fdata.face_img(rect).copyTo(image_roi);

	}
	else
	{
		fdata.face_img.copyTo(image_roi);
	}


	srcImg.frameId = 0;
	srcImg.data = (char*)image_roi.data;
	//srcImg.dataLen = image_roi.total();
	srcImg.width = image_roi.cols;
	srcImg.height = image_roi.rows;
	srcImg.angle = CW_IMAGE_ANGLE_0;
	srcImg.format = CW_IMAGE_BGR888;
	srcImg.mirror = CW_IMAGE_MIRROR_NONE;
	if (image_roi.channels() == 1)
	{
		srcImg.format = CW_IMAGE_GRAY8;
	}
	else if (image_roi.channels() == 3)
	{
		srcImg.format = CW_IMAGE_BGR888;
	}
	else if (image_roi.channels() == 4)
	{
		srcImg.format = CW_IMAGE_BGRA8888;
	}
	else
	{
		return false;
	}

	int iFaceNum = 0;


	unsigned long startt = common_get_cur_time_stampms();

	cw_errcode_t errDet = cwFaceDetection(detect_object, &srcImg, faceBuffers, 1, &iFaceNum, CW_OP_DET | CW_OP_KEYPT | CW_OP_ALIGN | CW_OP_QUALITY);

	if (errDet != CW_SDKLIT_OK)
	{
		LOGE("Face feature detect Error, Code:  %d", errDet);
		return false;
	}

	if (iFaceNum < 1)
	{
 		LOGD("Face count zero:  %d", errDet);
		return false;
	}

	if (CW_QUALITY_OK == faceBuffers[0].quality.errcode&&faceBuffers[0].quality.scores[0] < 0.5)
	{
		//LOGE("face image low");
		return false;
	}


	return true;
}

bool robot_face_features_yuncong::getFeatures(FaceDetectData &fdata, FaceFeatureData &facefeature, int *one_index)
{
	facefeature.face_feature.clear();

	if (fdata.face_img.empty() || fdata.face_img.data == nullptr || fdata.face_img.rows == 0 || fdata.face_img.cols == 0 || fdata.face_img.channels() < 3)
		return  false;

	if (!feature_object)
		return false;

	cw_img_t srcImg;
	cw_face_res_t faceBuffers[MAX_NUM_FACES] = { 0 };
	
	if (!getFaceID_Region(0, one_index, fdata, srcImg, faceBuffers))
	{
		LOGT("Get Face id region failed");
		return false;

	}



	int iUsedLength = cwGetFeatureLength(feature_object);

	facefeature.face_feature.resize(iUsedLength);
	
	cw_errcode_t errDet = cwGetFiledFeature(feature_object, &faceBuffers[0].faceAligned, facefeature.face_feature.data());

	if (CW_SDKLIT_OK != errDet)
	{
		facefeature.face_feature.clear();
		LOGE("Get Feature Error: %d",errDet);
		return false;
	}
	facefeature.algtype = m_algname;

	return true;

}

bool robot_face_features_yuncong::MergeFeatures(std::vector<char>&feature, std::vector<char>&temlpate)
{
	std::vector<char> allfeature;
	int iUsedLength = cwGetFeatureLength(feature_object);
	float wid[10] = { 1, 1,1,1,1 };
	allfeature.resize(iUsedLength);

	feature.insert(feature.end(), temlpate.begin(), temlpate.end());

	cw_errcode_t errCode = cwMergeFeature(feature_object, feature.data(), wid, 2, allfeature.data());
	if (CW_SDKLIT_OK != errCode)
	{
		allfeature.clear();
		LOGE("cwMergeFeature Error: %d", errCode);
		return false;
	}
	temlpate.clear();
	temlpate = allfeature;
	return true;
}

bool robot_face_features_yuncong::Features2Identify(std::vector<char> feature, std::vector<char>&identify)
{
	identify.clear();
	int iUsedLength = cwGetFeatureLength(feature_object);

	identify.resize(iUsedLength);

	cw_errcode_t errCode = cwConvertFiledFeatureToProbeFeature(feature_object, feature.data(), iUsedLength, 1, identify.data());
	if (CW_SDKLIT_OK != errCode)
	{
		identify.clear();
		LOGE("cwConvertFiledFeatureToProbeFeature identify Feature Error: %d", errCode);
		return false;
	}

	return true;
}

bool robot_face_features_yuncong::get_identify_features(FaceDetectData &fdata, std::vector<char> &features, int *one_index)
{
	features.clear();

	if (fdata.face_img.empty() || fdata.face_img.data == nullptr || fdata.face_img.rows == 0 || fdata.face_img.cols == 0 || fdata.face_img.channels() < 3)
		return  false;

	if (!feature_object )
		return false;

	cw_img_t srcImg;
	cw_face_res_t faceBuffers[MAX_NUM_FACES] = {0};
	
	if (!getFaceID_Region(1,one_index, fdata, srcImg, faceBuffers))
		return false;

	int iUsedLength = cwGetFeatureLength(feature_object);

	features.resize(iUsedLength );

	cw_errcode_t errCode = cwGetProbeFeature(feature_object, &faceBuffers[0].faceAligned, features.data());
	
	if (CW_SDKLIT_OK != errCode)
	{
		features.clear();
		LOGE("Get identify Feature Error: %d", errCode);
		return false;
	}

	return true;

}

bool robot_face_features_yuncong::getAllIdentifyFeatures(FaceDetectData &fdata, std::vector<std::vector<char>> &features_list)
{
	features_list.clear();

	if (fdata.face_list.size() == 0)
	{
		if (fdata.face_img.empty() || fdata.face_img.data == nullptr || fdata.face_img.rows == 0 || fdata.face_img.cols == 0 || fdata.face_img.channels() < 3)
			return  false;

		if (!detect_object)
			return false;


		cw_img_t srcImg;
		srcImg.frameId = 0;
		srcImg.data = (char*)fdata.face_img.data;
		//srcImg.dataLen = fdata.face_img.total();
		srcImg.width = fdata.face_img.cols;
		srcImg.height = fdata.face_img.rows;
		srcImg.angle = CW_IMAGE_ANGLE_0;
		srcImg.format = CW_IMAGE_BGR888;
		srcImg.mirror = CW_IMAGE_MIRROR_NONE;

		cw_face_res_t faceBuffers[MAX_NUM_FACES];
		int iFaceNum = 0;
		cw_errcode_t errDet = cwFaceDetection(detect_object, &srcImg, faceBuffers, MAX_NUM_FACES, &iFaceNum, CW_OP_DET | CW_OP_KEYPT | CW_OP_ALIGN | CW_OP_QUALITY);

		if (errDet != CW_SDKLIT_OK)
		{
			LOGE("Face detect Error, Code:  %d", errDet);
			return false;
		}
		if (iFaceNum < 1)
		{
			return false;
		}

		for (int i = 0; i < iFaceNum; i++)
		{
			FaceRect faces;

			faces.x = faceBuffers[i].faceRect.x;
			faces.y = faceBuffers[i].faceRect.y;
			faces.width = faceBuffers[i].faceRect.width;
			faces.height = faceBuffers[i].faceRect.height;
			faces.score = faceBuffers[i].quality.scores[0];
			//printf("score:%d", (int)faces.score);

			faces.width = (faces.x + faces.width)>fdata.face_img.cols ? (fdata.face_img.cols - faces.x - 1) : faces.width;
			faces.height = (faces.y + faces.height) > fdata.face_img.rows ? (fdata.face_img.rows - faces.y - 1) : faces.height;

			if (faces.x<0 || faces.y<0 ||
				faces.width == 0 || faces.height == 0 ||
				faces.x + faces.width>fdata.face_img.cols ||
				faces.y + faces.height>fdata.face_img.rows)
			{
				continue;
			}
			int image_xcentor = srcImg.width / 2;
			int image_ycentor = srcImg.height / 2;

			double x_centor = 0, y_centor = 0;
			x_centor = faces.x + faces.width / 2;
			y_centor = faces.y + faces.height / 2;


			x_centor = x_centor / image_xcentor - 1;
			y_centor = y_centor / image_ycentor - 1;


			faces.xoffset = x_centor;
			faces.yoffset = y_centor;
			faces.valid = 1;
			fdata.face_list.push_back(faces);

		}
	}

	for (int i = 0; i < fdata.face_list.size(); i++)
	{
		std::vector<char> feature_one;
		int id = i;
		if (m_congcastrate)
		{
			if (feature_cache.find(fdata.face_list[i].trackId) != feature_cache.end())
			{
				feature_one = feature_cache[fdata.face_list[i].trackId].feature;
				//feature_cache[fdata.face_list[i].trackId].time = now.get_cur_time_stampms();
			}
			else
			{
				if (get_identify_features(fdata, feature_one, &id))
				{
					if (fdata.face_list[i].trackId != -1)
					{
						feature_cache[fdata.face_list[i].trackId].feature = feature_one;
						feature_cache[fdata.face_list[i].trackId].time = common_get_cur_time_stamps();
					}
					else
					{
						;//printf("id -1\n");
					}

				}
				else
					;//printf("get feature failed\n");
			}
		}
		else
		{
			if (!get_identify_features(fdata, feature_one, &id))
			{
				;
			}
		}


		features_list.push_back(feature_one);

	}
	for (auto it = feature_cache.begin(); it != feature_cache.end(); )
	{
		if ((common_get_cur_time_stamps() - it->second.time) > (updatetime))
			it = feature_cache.erase(it);
		else
			it++;

	}
	
	return features_list.size() == 0 ? false : true;

}

double robot_face_features_yuncong::IdentifyTemplate(std::vector<char>&feature, std::vector<std::vector<char> >&temlpatelist, int &index)
{
	void *handle = NULL;

	if (m_congcastrate)
		handle = feature_object;
	else
		handle = match_object;

	if (!handle)
		return 0;

	float maxscore = 0;

	int iUsedLength = cwGetFeatureLength(handle);

	if (feature.size() < iUsedLength || temlpatelist.size() < 1)
		return 0;

	char *template_buffer = new char[temlpatelist.size()*iUsedLength];

	float *scores = new float[temlpatelist.size()];

	memset(template_buffer, 0x00, temlpatelist.size()*iUsedLength);
	memset(scores, 0x00, temlpatelist.size()*sizeof(float));

	for (int i = 0; i < temlpatelist.size(); i++)
		memcpy(template_buffer + i*iUsedLength, temlpatelist[i].data(), temlpatelist[i].size());

	cw_errcode_t errCode = cwComputeMatchScore(handle, feature.data(), feature.size(), 1, template_buffer, iUsedLength, temlpatelist.size(), scores);
	delete template_buffer;

	if (errCode != CW_SDKLIT_OK)
	{
		LOGE("yuncong face identify error:%d.", errCode);
		delete scores;
		return 0;
	}
	for (int i = 0; i < temlpatelist.size(); i++)
	{
		if (maxscore < scores[i])
		{
			maxscore = scores[i];
			index = i;
		}
	}
	delete scores;

	return maxscore;
}

robot_face_features_yuncong::~robot_face_features_yuncong()
{
	release_yuncong_handle();
}
#endif // HAS_YUNCONG