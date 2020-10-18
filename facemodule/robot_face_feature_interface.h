#pragma once
#include "robot_face_detect_interface.h"
#include "opencv2\core\core.hpp"

class robot_face_feature_interface
{
public:
	robot_face_feature_interface(){};

	virtual int  init()=0;

	virtual bool getFeatures(FaceDetectData &fdata, FaceFeatureData  &facefeature, int *one_index) = 0;

	virtual bool Features2Identify(std::vector<char> feature, std::vector<char>&identify) = 0;

	virtual bool MergeFeatures(std::vector<char>&feature, std::vector<char>&temlpate) = 0;

	virtual bool getAllIdentifyFeatures(FaceDetectData &fdata, std::vector<std::vector<char>> &features_list) = 0;

	virtual double IdentifyTemplate(std::vector<char>&feature, std::vector<std::vector<char> >&temlpatelist, int &index) = 0;

	virtual ~robot_face_feature_interface(){};

	std::string  m_algname;


	int getMainFaceIndex(FaceDetectData &fdata,int *&one_index)
	{
		int maxindex = 0;
		if (*one_index == -1)
		{
			int discenter = CANMERA_HEIGHT;
			for (int i = 0; i < fdata.face_list.size(); i++)
			{

				int _x = (fdata.face_list[i].width / 2 + fdata.face_list[i].x);
				int _y = (fdata.face_list[i].height / 2 + fdata.face_list[i].y);

				_x = abs((CANMERA_WIDTH / 2) - _x);
				_y = abs((CANMERA_HEIGHT / 2) - _y);

				int _dis = sqrt(_x*_x + _y*_y);

				if (discenter > _dis &&
					((fdata.face_list[i].height*fdata.face_list[i].width) >
					((3 * fdata.face_list[maxindex].width* fdata.face_list[maxindex].height) / 4))
					/*&&faces[i].score > (faces[maxindex].score - 3)*/)
				{
					maxindex = i;
					discenter = _dis;
				}

			}
			if (fdata.face_list.size() == 0)return -1;
			*one_index = maxindex;
		}
		else
		{
			if (*one_index >= 10000)
			{
				int findid = -1;
				for (int i = 0; i < fdata.face_list.size(); i++)
				{
					if (fdata.face_list[i].trackId == *one_index)
						findid = i;
				}
				if (findid < 0)
					return  -1;
				maxindex = findid;
				*one_index = maxindex;
			}
			else
			{
				if (fdata.face_list.size() <= *one_index || *one_index < 0)
				{
					LOGE("face index error!");
					return  -1;
				}

				maxindex = *one_index;

			}

		}
		if (maxindex < 0 || maxindex >= fdata.face_list.size())return  -1;

		if (fdata.face_list[maxindex].x<0 || fdata.face_list[maxindex].x + fdata.face_list[maxindex].width > fdata.face_img.cols ||
			fdata.face_list[maxindex].y + fdata.face_list[maxindex].height > fdata.face_img.rows || fdata.face_list[maxindex].y < 0 ||
			fdata.face_list[maxindex].width <= 0 || fdata.face_list[maxindex].height <= 0)
			return  -1;

		return maxindex;
	}

};

