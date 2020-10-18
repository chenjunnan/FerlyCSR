#include "robot_face_features_seetaface.h"
#include "common_helper.h"
#include "SeetaFace\FaceDetection\include\common.h"
#include "opencv2\imgproc\imgproc.hpp"
#include "robot_face_detect_interface.h"
#include "opencv2\highgui\highgui.hpp"



robot_face_features_seetaface::robot_face_features_seetaface()
{
	m_alignment_detector = NULL;
	m_detector = NULL;
	m_init_ok = 0;
	m_algname = "seetaface";
}


int robot_face_features_seetaface::init()
{
	if (!common_check_file_exist(SEETAFACE_DETECT_MODEL_PATH) ||
		!common_check_file_exist(SEETAFACE_IDENTIFY_MODEL_PATH) ||
		!common_check_file_exist(SEETAFACE_ALIGNMENT_MODEL_PATH)
		)
	{
		LOGF("face model file miss!!!!");
		return -1;
	}
	m_detector = new seeta::FaceDetection(SEETAFACE_DETECT_MODEL_PATH);
	m_alignment_detector = new seeta::FaceAlignment(SEETAFACE_ALIGNMENT_MODEL_PATH);
	m_face_recognizer.LoadModel(SEETAFACE_IDENTIFY_MODEL_PATH);
	m_init_ok = 1;
	m_detector->SetMinFaceSize(40);
	m_detector->SetScoreThresh(2.f);
	m_detector->SetImagePyramidScaleFactor(0.8f);
	m_detector->SetWindowStep(4, 4);
	return 0;
}

static int i_save = 1001;

bool robot_face_features_seetaface::getFeatures(FaceDetectData &fdata, FaceFeatureData &facefeature, int *one_index /*= -1*/)
{
	if (!m_init_ok)
	{
		LOGE("seetafacfe face init failed");
		return false;
	}

	facefeature.face_feature.clear();
	if (fdata.face_img.empty() || fdata.face_img.data == nullptr ||
		fdata.face_img.rows == 0 || fdata.face_img.cols == 0 || fdata.face_img.channels()<3)
		return  false;

	if (fdata.face_list.size() < 1)
		return false;


	if (!m_detector || !m_alignment_detector)
		return false;

	int maxindex = getMainFaceIndex(fdata, one_index);
	if (maxindex < 0)return false;

	int range_add = 20;

	cv::Rect rect;
	rect.x = fdata.face_list[maxindex].x - range_add < 0 ? 0 : fdata.face_list[maxindex].x - range_add;
	rect.y = fdata.face_list[maxindex].y - range_add < 0 ? 0 : fdata.face_list[maxindex].y - range_add;

	rect.width = ((rect.x + fdata.face_list[maxindex].width) + 2 * range_add) >= fdata.face_img.cols ?
		(fdata.face_img.cols - rect.x - 1) : ((fdata.face_list[maxindex].width) + 2 * range_add);

	rect.height = ((rect.y + fdata.face_list[maxindex].height) + 2 * range_add) >= fdata.face_img.rows ?
		(fdata.face_img.rows - rect.y - 1) : ((fdata.face_list[maxindex].height) + 2 * range_add);



	cv::Mat image_roi;
	fdata.face_img(rect).copyTo(image_roi);


	cv::Mat gray;
	cvtColor(image_roi, gray, CV_BGR2GRAY);

	seeta::ImageData src_img_data(gray.cols, gray.rows, gray.channels());
	src_img_data.data = gray.data;


	seeta::ImageData src_img_data_color(image_roi.cols, image_roi.rows, image_roi.channels());
	src_img_data_color.data = image_roi.data;

	int bt = common_get_cur_time_stampms();

	std::vector<seeta::FaceInfo> faces = m_detector->Detect(src_img_data);

	unsigned int face_num = faces.size();


	if (face_num != 1)
	{
		LOGD("find %d face in image", face_num);
		if (face_num == 0)return false;
	}

	seeta::FacialLandmark points[5];
	bt = common_get_cur_time_stampms();
	if (!m_alignment_detector->PointDetectLandmarks(src_img_data, faces[0], points))
	{
		LOGE("PointDetectLandmarks failed");
		return  false;
	}


	float *feature_buffer = new float[m_face_recognizer.feature_size()];
	memset(feature_buffer, 0x00, sizeof(float)*m_face_recognizer.feature_size());

	m_face_recognizer.ExtractFeatureWithCrop(src_img_data_color, points, feature_buffer);

	char *temp_ptr = (char*)feature_buffer;
	for (int i = 0; i < m_face_recognizer.feature_size()*sizeof(float);i++)
		facefeature.face_feature.push_back(temp_ptr[i]);
	delete feature_buffer;
	facefeature.algtype = m_algname;
	
	return true;
}

bool robot_face_features_seetaface::Features2Identify(std::vector<char> feature, std::vector<char>&identify)
{
	identify.clear();
	identify = feature;
	return true;
}

bool robot_face_features_seetaface::MergeFeatures(std::vector<char>&feature, std::vector<char>&temlpate)
{
	temlpate.clear();
	temlpate =feature;
	return true;
}

bool robot_face_features_seetaface::getAllIdentifyFeatures(FaceDetectData &fdata, std::vector<std::vector<char>> &features_list)
{
	if (!m_init_ok)
	{
		LOGE("seetafacfe face init failed");
		return false;
	}

	features_list.clear();
	std::vector<char> feature_one;

	if (fdata.face_list.size() == 0)
	{
		if (fdata.face_img.empty() || fdata.face_img.data == nullptr || fdata.face_img.rows == 0 || fdata.face_img.cols == 0 || fdata.face_img.channels() < 3)
			return  false;

		if (!m_detector || !m_alignment_detector)
			return false;

		cv::Mat image_roi = fdata.face_img;

		cv::Mat gray;
		cvtColor(image_roi, gray, CV_BGR2GRAY);

		seeta::ImageData src_img_data(gray.cols, gray.rows, gray.channels());
		src_img_data.data = gray.data;


		seeta::ImageData src_img_data_color(image_roi.cols, image_roi.rows, image_roi.channels());
		src_img_data_color.data = image_roi.data;

		std::vector<seeta::FaceInfo> faces = m_detector->Detect(src_img_data);

		int facecnt = 0;
		for (unsigned int i = 0; i < faces.size(); i++) {
			FaceRect face;
			face.x = faces[i].bbox.x;
			face.y = faces[i].bbox.y;
			face.width = faces[i].bbox.width;
			face.height = faces[i].bbox.height;

			face.width = (face.x + face.width)>image_roi.cols ? (image_roi.cols - face.x - 1) : face.width;
			face.height = (face.y + face.height) > image_roi.rows ? (image_roi.rows - face.y - 1) : face.height;

			face.score = ((double)faces[i].score / 15.0);
			if (face.score > 1)face.score = 1.0;

			if (face.x<0 || face.y<0 ||
				face.width == 0 || face.height == 0 ||
				face.x + face.width>image_roi.cols ||
				face.y + face.height>image_roi.rows)
				continue;

			face.trackId = i+10000;
			face.valid = 1;

			fdata.face_list.push_back(face);
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


double robot_face_features_seetaface::IdentifyTemplate(std::vector<char>&feature, std::vector<std::vector<char> >&temlpatelist, int &index)
{
	if (!m_init_ok)
	{
		LOGE("seetafacfe face init failed");
		return 0;
	}
	if (feature.size() <= 0 || temlpatelist.size() <=0)
		return 0;

	index = 0;
	double max_score = 0;
	for (int i = 0; i < temlpatelist.size(); i++)
	{
		double s = m_face_recognizer.CalcSimilarity((float*)feature.data(), (float*)temlpatelist[i].data());
		if (s > max_score)
		{
			index = i;
			max_score = s;
		}
	}

	return max_score;
}

robot_face_features_seetaface::~robot_face_features_seetaface()
{
}
