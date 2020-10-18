#include "robot_face_features_arcsoft.h"
#include "opencv2\highgui\highgui.hpp"


robot_face_features_arcsoft::robot_face_features_arcsoft()
{
	handle = NULL;
	isInitFailed = false;
	m_algname = "arcsoft";
}


int robot_face_features_arcsoft::init()
{
	if (!handle)
	{
		//激活SDK
		try
		{
			MRESULT res = ASFActivation(ARCSOFT_APPID, ARCSOFT_SDKKey);
			if (MOK == res || MERR_ASF_ALREADY_ACTIVATED == res)
			{
				LOGI("ALActivation feature sucess: %x ", res);

			}
			else
			{
				LOGE("ALActivation feature failed: %x ", res);
				isInitFailed = true;
				return 1;
			}

			//初始化引擎
			MInt32 mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE;
			res = ASFInitEngine(ASF_DETECT_MODE_IMAGE, ASF_OP_0_ONLY, 16, 5, mask, &handle);
			if (res != MOK)
			{
				LOGE("ALInitEngine feature fail: %x ", res);
				isInitFailed = true;
				return 2;
			}
			else
				LOGI("ALInitEngine feature sucess: %x ", res);
			if (!handle)
			{
				return 3;
			}
		}
		catch (std::exception* e)
		{
			LOGE("ALActivation feature exception: %s ", e->what());
			isInitFailed = true;
			return 4;
		}
		catch (...)
		{
			LOGE("ALActivation feature exception");
			isInitFailed = true;
			return 5;
		}

	}
	return 0;
}

bool robot_face_features_arcsoft::getFeatures(FaceDetectData &fdata, FaceFeatureData &facefeature, int *one_index /*= -1*/)
{
	if (isInitFailed)
	{
		LOGE("arcsoft face init failed");
		return false;
	}

	facefeature.face_feature.clear();
	if (fdata.face_img.empty() || fdata.face_img.data == nullptr ||
		fdata.face_img.rows == 0 || fdata.face_img.cols == 0 || fdata.face_img.channels() < 3)
		return  false;

	if (fdata.face_list.size() < 1)
		return false;


	if (!handle )
		return false;

	cv::Rect rect;
	int maxindex = getMainFaceIndex(fdata, one_index);
	if (maxindex < 0)return false;

	int range_add = 50;


	rect.x = fdata.face_list[maxindex].x - range_add < 0 ? 0 : fdata.face_list[maxindex].x - range_add;
	rect.y = fdata.face_list[maxindex].y - range_add < 0 ? 0 : fdata.face_list[maxindex].y - range_add;

	rect.width = ((rect.x + fdata.face_list[maxindex].width) + 2 * range_add) >= fdata.face_img.cols ?
		(fdata.face_img.cols - rect.x - 1) : ((fdata.face_list[maxindex].width) + 2 * range_add);

	rect.height = ((rect.y + fdata.face_list[maxindex].height) + 2 * range_add) >= fdata.face_img.rows ?
		(fdata.face_img.rows - rect.y - 1) : ((fdata.face_list[maxindex].height) + 2 * range_add);

	rect.width = (rect.width / 4) * 4;

	cv::Mat image_roi;
	fdata.face_img(rect).copyTo(image_roi);



	ASF_MultiFaceInfo detectedFaces1 = { 0 };
	ASF_SingleFaceInfo SingleDetectedFaces1 = { 0 };
	ASF_FaceFeature feature1 = { 0 };
	MRESULT res = ASFDetectFaces(handle, image_roi.cols, image_roi.rows, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)image_roi.data, &detectedFaces1);
	if (MOK != res)
	{
		LOGE("ASFDetectFaces 1 fail: %x", res);
		return false;
	}
	if (detectedFaces1.faceNum != 1)
	{
		LOGD("find %d face in image", detectedFaces1.faceNum);
		if (detectedFaces1.faceNum == 0)return false;
	}

	SingleDetectedFaces1.faceRect.left = detectedFaces1.faceRect[0].left;
	SingleDetectedFaces1.faceRect.top = detectedFaces1.faceRect[0].top;
	SingleDetectedFaces1.faceRect.right = detectedFaces1.faceRect[0].right;
	SingleDetectedFaces1.faceRect.bottom = detectedFaces1.faceRect[0].bottom;
	SingleDetectedFaces1.faceOrient = detectedFaces1.faceOrient[0];

	res = ASFFaceFeatureExtract(handle, image_roi.cols, image_roi.rows, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)image_roi.data, &SingleDetectedFaces1, &feature1);
	if (res != MOK)
	{
		LOGE("ASFFaceFeatureExtract 1 fail: %x", res);
		return false;
	}
	//拷贝feature
	facefeature.face_feature.resize(feature1.featureSize);
	memset(facefeature.face_feature.data(), 0, feature1.featureSize);
	memcpy(facefeature.face_feature.data(), feature1.feature, feature1.featureSize);
	facefeature.algtype = m_algname;
	//LOGT("ASFFaceFeatureExtract time %d  %d :%d ", src_img_data.height, src_img_data.width, get_cur_time_stampms() - bt);
	return true;
}

bool robot_face_features_arcsoft::Features2Identify(std::vector<char> feature, std::vector<char>&identify)
{
	identify.clear();
	identify = feature;
	return true;
}

bool robot_face_features_arcsoft::MergeFeatures(std::vector<char>&feature, std::vector<char>&temlpate)
{
	temlpate.clear();
	temlpate = feature;
	return true;
}

bool robot_face_features_arcsoft::getAllIdentifyFeatures(FaceDetectData &fdata, std::vector<std::vector<char>> &features_list)
{
	if (isInitFailed)
	{
		LOGE("arcsoft face init failed");
		return false;
	}

	features_list.clear();
	std::vector<char> feature_one;

	if (fdata.face_list.size() == 0)
	{
		if (fdata.face_img.empty() || fdata.face_img.data == nullptr || fdata.face_img.rows == 0 || fdata.face_img.cols == 0 || fdata.face_img.channels() < 3)
			return  false;

		if (!handle)
			return false;

		cv::Mat image_roi = fdata.face_img;

		ASF_MultiFaceInfo detectedFaces1 = { 0 };
		ASF_SingleFaceInfo SingleDetectedFaces1 = { 0 };
		ASF_FaceFeature feature1 = { 0 };
		MRESULT res = ASFDetectFaces(handle, image_roi.cols, image_roi.rows, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)image_roi.data, &detectedFaces1);

		int facecnt = 0;
		for (unsigned int i = 0; i < detectedFaces1.faceNum; i++) 
		{
			FaceRect faces;
			faces.x = SingleDetectedFaces1.faceRect.left;
			faces.y = SingleDetectedFaces1.faceRect.top;
			faces.width = SingleDetectedFaces1.faceRect.right > image_roi.cols ? (image_roi.cols - SingleDetectedFaces1.faceRect.left - 1) : (SingleDetectedFaces1.faceRect.right - SingleDetectedFaces1.faceRect.left - 1);
			faces.height = SingleDetectedFaces1.faceRect.bottom > image_roi.rows ? (image_roi.rows - SingleDetectedFaces1.faceRect.top - 1) : (SingleDetectedFaces1.faceRect.bottom - SingleDetectedFaces1.faceRect.top - 1);

			if (faces.x<0 || faces.y<0 ||
				faces.width == 0 || faces.height == 0 ||
				faces.x + faces.width>image_roi.cols ||
				faces.y + faces.height>image_roi.rows)
			{
				continue;
			}


			faces.trackId = i + 10000;
			faces.valid = 1;

			fdata.face_list.push_back(faces);
		}

	}

	for (int i = 0; i < fdata.face_list.size(); i++)
	{
		FaceFeatureData feature_one;
		if (getFeatures(fdata, feature_one, &i))
			;
		features_list.push_back(feature_one.face_feature);
	}

	return features_list.size() == 0 ? false : true;
}


double robot_face_features_arcsoft::IdentifyTemplate(std::vector<char>&feature, std::vector<std::vector<char> >&temlpatelist, int &index)
{
	if (isInitFailed)
	{
		LOGE("arcsoft face init failed");
		return 0;
	}

	if (feature.size() <= 0 || temlpatelist.size() <= 0)
		return 0;

	index = 0;
	double max_score = 0;
	for (int i = 0; i < temlpatelist.size(); i++)
	{
		ASF_FaceFeature   feature1 = { 0 }, feature2 = { 0 };
		MFloat confidenceLevel = 0;
		feature1.featureSize = feature.size();
		feature1.feature = (unsigned char*)feature.data();

		feature2.featureSize = temlpatelist[i].size();
		feature2.feature = (unsigned char*)temlpatelist[i].data();
		

		MRESULT res = ASFFaceFeatureCompare(handle, &feature1, &feature2, &confidenceLevel);
		if (res != MOK)
		{
			LOGE("ASFFaceFeatureCompare fail: %d\n", res);
			return 0;
		}


		double s = confidenceLevel;
		if (s > max_score)
		{
			index = i;
			max_score = s;
		}
	}

	return max_score;
}


robot_face_features_arcsoft::~robot_face_features_arcsoft()
{
}
