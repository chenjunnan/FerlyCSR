#pragma once
#include "robot_face_feature_interface.h"
#ifdef HAS_YUNCONG
#include "CWFaceConfig.h"
struct idfeature_info
{
	unsigned long long time;
	std::vector<char> feature;
	idfeature_info()
	{
		time = 0;
	}

};


class robot_face_features_yuncong :public robot_face_feature_interface
{
public:
	robot_face_features_yuncong();


	virtual int  init();

	virtual bool getFeatures(FaceDetectData &fdata, FaceFeatureData &facefeature, int *one_index);

	virtual bool Features2Identify(std::vector<char> feature, std::vector<char>&identify);

	virtual bool MergeFeatures(std::vector<char>&feature, std::vector<char>&temlpate);
	
	virtual bool getAllIdentifyFeatures(FaceDetectData &fdata, std::vector<std::vector<char>> &features_list);

	virtual double IdentifyTemplate(std::vector<char>&feature, std::vector<std::vector<char> >&temlpatelist, int &index);

	virtual ~robot_face_features_yuncong();
private:
	void release_yuncong_handle();

	bool get_identify_features(FaceDetectData &fdata, std::vector<char> &features, int *one_index);
	bool getFaceID_Region(int type,int *one_index, FaceDetectData &fdata, cw_img_t &srcImg, cw_face_res_t*faceBuffers);

	void *detect_object;
	void *feature_object;
	void *Attribute_object;
	void *match_object;
	bool  m_congcastrate;
	int   m_facesize_th;
	double m_score_th;
	int updatetime;
	std::map<int, idfeature_info> feature_cache;
};

#endif // HAS_YUNCONG