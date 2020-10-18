#include "robot_face_features_readface.h"

#ifdef HAS_READFACE

#include "robot_face_readface_singleton.h"


robot_face_features_readface::robot_face_features_readface()
{
	m_PfaceRecognition = NULL;
	m_pfaceDetect = NULL;
	isInit = false;
	m_algname = "readface";
}


int robot_face_features_readface::init()
{
	if (isInit)return 0;
	if (!robot_face_readface_singleton::getInstance()->isLicense())
		return -1;
	RSHandle m_Plicense = robot_face_readface_singleton::getInstance()->getRSHandle();

	std::string filepath;
	common_get_current_dir(&filepath);
	common_fix_path(&filepath);

	std::string path = filepath + READFACE_CFGPATH;
	common_fix_path(&path);
	if (!common_isDirectory(path.c_str()))
		common_create_dir(path.c_str());

	std::string dbpath = filepath;
	dbpath.append(READFACE_DB_FILE_PATH);

	rsInitFaceRecognition(&m_PfaceRecognition, m_Plicense, dbpath.c_str());
	if (m_PfaceRecognition == NULL)
	{
		LOGE( "rsInitFaceRecognition failed.");
		return -1;
	}
	int m_face_th = 750;
	if (!common_config_get_field_int("readface", "threshold", &m_face_th))
		m_face_th = 750;


	rsRecognitionSetConfidence(m_PfaceRecognition, m_face_th/10);
	rsInitFaceDetect(&m_pfaceDetect, m_Plicense);
	if (m_pfaceDetect == NULL)
	{
		LOGE("rsInitFaceDetect failed.");
		return -1;
	}
	isInit = true;
	return 0;
}

bool robot_face_features_readface::getFeatures(FaceDetectData &fdata, FaceFeatureData &facefeature, int *one_index)
{
	if (!isInit)
	{
		LOGE("readface face init failed");
		return false;
	}

	facefeature.face_feature.clear();
	if (fdata.face_img.empty() || fdata.face_img.data == nullptr ||
		fdata.face_img.rows == 0 || fdata.face_img.cols == 0 || fdata.face_img.channels() < 3)
		return  false;

	if (fdata.face_list.size() < 1)
		return false;


	if (!m_pfaceDetect || !m_PfaceRecognition)
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

	rs_face *pFaceArray = NULL;
	int faceCount = 0;
	int result = rsRunFaceDetect(m_pfaceDetect, image_roi.data,
		PIX_FORMAT_BGR888,
		image_roi.cols,
		image_roi.rows,
		image_roi.step,
		RS_IMG_CLOCKWISE_ROTATE_0,
		&pFaceArray,
		&faceCount);
	if (result != 0 )
	{
		//LOGE("rsRunFaceDetect  fail: %d", result);
		return false;
	}

	if (faceCount <= 0)
		return false;

	if (faceCount != 1)
	{
		LOGD("find %d face in image", faceCount);
	}
	rs_face mFace = pFaceArray[0];
	rs_face_feature faceFeature;

	int regFlag = rsRecognitionGetFaceFeature(m_PfaceRecognition, image_roi.data,
		PIX_FORMAT_BGR888,
		image_roi.cols,
		image_roi.rows,
		image_roi.step,
		&mFace,
		&faceFeature
		);


	releaseFaceDetectResult(pFaceArray, faceCount);

	facefeature.face_feature.resize(sizeof(rs_face_feature));
	memset(facefeature.face_feature.data(), 0, sizeof(rs_face_feature));
	memcpy(facefeature.face_feature.data(), &faceFeature, sizeof(rs_face_feature));
	facefeature.algtype = m_algname;
	//LOGT("ASFFaceFeatureExtract time %d  %d :%d ", src_img_data.height, src_img_data.width, get_cur_time_stampms() - bt);
	return true;
}

bool robot_face_features_readface::Features2Identify(std::vector<char> feature, std::vector<char>&identify)
{
	identify.clear();
	identify = feature;
	return true;
}

bool robot_face_features_readface::MergeFeatures(std::vector<char>&feature, std::vector<char>&temlpate)
{
	temlpate.clear();
	temlpate = feature;
	return true;
}

bool robot_face_features_readface::getAllIdentifyFeatures(FaceDetectData &fdata, std::vector<std::vector<char>> &features_list)
{
	if (!isInit)
	{
		LOGE("readface face init failed");
		return false;
	}
	if (!m_pfaceDetect || !m_PfaceRecognition)
		return false;

	features_list.clear();
	std::vector<char> feature_one;

	if (fdata.face_list.size() == 0)
	{
		if (fdata.face_img.empty() || fdata.face_img.data == nullptr || fdata.face_img.rows == 0 || fdata.face_img.cols == 0 || fdata.face_img.channels() < 3)
			return  false;

		cv::Mat image_roi = fdata.face_img;
		rs_face *pFaceArray = NULL;
		int faceCount = 0;
		int result = rsRunFaceDetect(m_pfaceDetect, image_roi.data,
			PIX_FORMAT_BGR888,
			image_roi.cols,
			image_roi.rows,
			image_roi.step,
			RS_IMG_CLOCKWISE_ROTATE_0,
			&pFaceArray,
			&faceCount);
		if (result !=0|| faceCount <= 0)
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

			faces.width = (faces.x + faces.width) > image_roi.cols ? (image_roi.cols - faces.x - 1) : faces.width;
			faces.height = (faces.y + faces.height) > image_roi.rows ? (image_roi.rows - faces.y - 1) : faces.height;


			if (faces.x<0 || faces.y<0 ||
				faces.width == 0 || faces.height == 0 ||
				faces.x + faces.width>image_roi.cols ||
				faces.y + faces.height>image_roi.rows)
			{
				continue;
			}


			faces.trackId = mFace.trackId + 10000;


			if (faces.score > 0.25)
				faces.valid = 1;
		
			fdata.face_list.push_back(faces);

		}
		releaseFaceDetectResult(pFaceArray, faceCount);

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

double robot_face_features_readface::IdentifyTemplate(std::vector<char>&feature, std::vector<std::vector<char> >&temlpatelist, int &index)
{

	if (!isInit)
	{
		LOGE("readface face init failed");
		return 0;
	}

	if (feature.size() <= 0 || temlpatelist.size() <= 0)
		return 0;

	index = 0;
	double max_score = 0;
	for (int i = 0; i < temlpatelist.size(); i++)
	{
		rs_face_feature *faceFeature1=NULL;
		rs_face_feature *faceFeature2 = NULL;
		faceFeature1 = (rs_face_feature*)feature.data();
		faceFeature2 = (rs_face_feature*)temlpatelist[i].data();
		float s=rsRecognitionFaceVerification(m_PfaceRecognition, faceFeature1, faceFeature2);

		if (s > max_score)
		{
			index = i;
			max_score = s;
		}
	}

	return max_score*0.01;
}

robot_face_features_readface::~robot_face_features_readface()
{
	if (m_pfaceDetect != NULL)
	{
		rsUnInitFaceDetect(&m_pfaceDetect);
		m_pfaceDetect = NULL;
	}
	if (m_PfaceRecognition != NULL)
	{
		rsUnInitFaceRecognition(&m_PfaceRecognition);
		m_PfaceRecognition = NULL;
	}
}
#endif // HAS_READFACE

