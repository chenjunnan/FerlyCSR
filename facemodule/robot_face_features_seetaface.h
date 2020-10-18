#pragma once
#include "opencv2\core\core.hpp"
#include "robot_face_detect_interface.h"
#include "SeetaFace\FaceDetection\include\face_detection.h"
#include "SeetaFace\FaceAlignment\include\face_alignment.h"
#include "SeetaFace\FaceIdentification\include\face_identification.h"
#include "robot_face_feature_interface.h"
#include "robot_face_detect_seetaface.h"
class robot_face_features_seetaface :public robot_face_feature_interface
{
public:
	robot_face_features_seetaface();


	virtual int  init();

	virtual bool getFeatures(FaceDetectData &fdata, FaceFeatureData &facefeature, int *one_index);

	virtual bool Features2Identify(std::vector<char> feature, std::vector<char>&identify) ;

	virtual bool MergeFeatures(std::vector<char>&feature, std::vector<char>&temlpate) ;

	virtual bool getAllIdentifyFeatures(FaceDetectData &fdata, std::vector<std::vector<char>> &features_list);

	virtual double IdentifyTemplate(std::vector<char>&feature, std::vector<std::vector<char> >&temlpatelist, int &index) ;


	virtual ~robot_face_features_seetaface();
private:
	seeta::FaceDetection *m_detector;
	seeta::FaceAlignment *m_alignment_detector;
	seeta::FaceIdentification m_face_recognizer;
	int    m_init_ok;
};

