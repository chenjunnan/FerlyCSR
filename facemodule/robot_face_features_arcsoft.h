#pragma once
#include "robot_face_feature_interface.h"
#ifdef HAS_ARCSOFT

#include "arcsoft_face_sdk.h"
#include "merror.h"

class robot_face_features_arcsoft :public robot_face_feature_interface
{
public:
	robot_face_features_arcsoft();
	virtual int  init();

	virtual bool getFeatures(FaceDetectData &fdata, FaceFeatureData  &facefeature, int *one_index);

	virtual bool Features2Identify(std::vector<char> feature, std::vector<char>&identify) ;

	virtual bool MergeFeatures(std::vector<char>&feature, std::vector<char>&temlpate);

	virtual bool getAllIdentifyFeatures(FaceDetectData &fdata, std::vector<std::vector<char>> &features_list);

	virtual double IdentifyTemplate(std::vector<char>&feature, std::vector<std::vector<char> >&temlpatelist, int &index) ;


	virtual ~robot_face_features_arcsoft();
private:	
	MHandle handle;
		bool  isInitFailed;
};


#endif // HAS_ARCSOFT
