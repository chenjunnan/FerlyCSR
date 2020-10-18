#pragma once
#include "robot_face_feature_interface.h"
#ifdef HAS_READFACE
#include "RSFaceSDK.h"
#include "RSCommon.h"
class robot_face_features_readface :
	public robot_face_feature_interface
{
public:
	robot_face_features_readface();

	virtual int  init();

	virtual bool getFeatures(FaceDetectData &fdata, FaceFeatureData  &facefeature, int *one_index);

	virtual bool Features2Identify(std::vector<char> feature, std::vector<char>&identify);

	virtual bool MergeFeatures(std::vector<char>&feature, std::vector<char>&temlpate);

	virtual bool getAllIdentifyFeatures(FaceDetectData &fdata, std::vector<std::vector<char>> &features_list);

	virtual double IdentifyTemplate(std::vector<char>&feature, std::vector<std::vector<char> >&temlpatelist, int &index);


	virtual ~robot_face_features_readface();
private:
	// ÈËÁ³Ê¶±ð¾ä±ú
	RSHandle m_PfaceRecognition;
	// ÈËÁ³¼ì²â¾ä±ú
	RSHandle m_pfaceDetect;

	bool isInit;
};
#endif // HAS_READFACE

