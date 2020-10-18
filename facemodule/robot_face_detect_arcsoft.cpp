#include "robot_face_detect_arcsoft.h"
#ifdef HAS_ARCSOFT

#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\highgui\highgui.hpp"




robot_face_detect_arcsoft::robot_face_detect_arcsoft()
{
	isInitFailed = false;
	handle = NULL;
	handle_image = NULL;
}


void robot_face_detect_arcsoft::setMode(std::string name)
{
	

}



bool robot_face_detect_arcsoft::DetectFace(cv::Mat &frame, FaceRect minface, std::vector <FaceRect> &face_postion, std::string ext /*= ""*/)
{
	face_postion.clear();
	int * pResults = NULL;
	int minface_th = (minface.height + minface.width) / 2;
	if (isInitFailed)return false;
	if (!handle)
	{
		//激活SDK
		if (!init_engine(minface_th))
		{
			destory_handle();
			return false;
		}

	}

	unsigned long  startt = common_get_cur_time_stampms();

	if (frame.cols % 4 != 0)
	{
		int width_dst = ((frame.cols+3)/4)*4;
		double rate = ((double)width_dst )/ frame.cols;
		int height_dst = rate*frame.rows;
		cv::resize(frame, frame, cv::Size(width_dst,height_dst), 0, 0, CV_INTER_LINEAR);
	}


	ASF_MultiFaceInfo detectedFaces1 = { 0 }, detectedFaces2 = {0};
	MHandle use_hanlde;
	if (ext.compare("image") == 0 && handle_image!=NULL)
		use_hanlde = handle_image;
	else
		use_hanlde = handle;


	MRESULT res = ASFDetectFaces(use_hanlde, frame.cols, frame.rows, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)frame.data, &detectedFaces1);
	if (MOK != res)
	{
		LOGE("ASFDetectFaces 1 fail: %x ", res);
		return false;
	}

	if (detectedFaces1.faceNum==0)return false;

	std::vector<int> index_list;
	while (!(index_list.size() == detectedFaces1.faceNum || index_list.size()==4))
	{
		int current_max_index = -1;
		int maxface = -1;
		for (int i = 0; i < detectedFaces1.faceNum; i++)
		{
			auto it=std::find(index_list.begin(), index_list.end(), i);
			if (it == index_list.end())
			{
				if ((detectedFaces1.faceRect[i].right - detectedFaces1.faceRect[i].left)*(detectedFaces1.faceRect[i].bottom - detectedFaces1.faceRect[i].top)>maxface)
				{
					maxface = (detectedFaces1.faceRect[i].right - detectedFaces1.faceRect[i].left)*(detectedFaces1.faceRect[i].bottom - detectedFaces1.faceRect[i].top);
					current_max_index = i;;
				}
			}
		}

		if (current_max_index != -1)
			index_list.push_back(current_max_index);
	}

	if (index_list.size() > 0)
	{
		detectedFaces2.faceRect = new MRECT[index_list.size()];
		detectedFaces2.faceOrient = new MInt32[index_list.size()];
		for (int i = 0; i < index_list.size(); i++)
		{
			detectedFaces2.faceRect[i] = detectedFaces1.faceRect[index_list[i]];
			detectedFaces2.faceOrient[i] = detectedFaces1.faceOrient[index_list[i]];
		}
		detectedFaces2.faceNum = index_list.size();
	}


	ASF_AgeInfo ageInfo = { 0 };
	ASF_GenderInfo genderInfo = { 0 };
	ASF_Face3DAngle angleInfo = { 0 };
 	ASF_LivenessInfo liveinfo = { 0 };
	
	//MInt32 processMask = /*ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE |*/ ASF_LIVENESS;
	MInt32 processMask = ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS;
	res = ASFProcess(use_hanlde, frame.cols, frame.rows, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)frame.data, &detectedFaces2, processMask);
	if (MOK == res)
	{
		// 获取年龄		
		res = ASFGetAge(use_hanlde, &ageInfo);
		if (MOK != res || ageInfo.ageArray == NULL /*&& ageInfo.num >= 1 && ageInfo.ageArray != NULL*/)
		{
			ageInfo.num = -1;
			LOGD("ASFGetAge fail: %x ", res);
		}

		// 获取性别

		res = ASFGetGender(use_hanlde, &genderInfo);
		if (MOK != res || genderInfo.genderArray == NULL /*&& genderInfo.num >= 1 && genderInfo.genderArray != NULL*/)
		{
			genderInfo.num = -1;
			LOGD("ASFGetGender fail: %x ", res);
		}
			
		// 获取3D角度

		res = ASFGetFace3DAngle(use_hanlde, &angleInfo);
		if (MOK != res /*&&angleInfo.status != NULL&&*angleInfo.status == 0 && angleInfo.num >= 1 && angleInfo.pitch != NULL && angleInfo.roll != NULL && angleInfo.yaw != NULL*/)
		{
			angleInfo.num = -1;
			LOGD("ASFGetFace3DAngle fail: %x ", res);
		}



		res = ASFGetLivenessScore(handle, &liveinfo);
		if (res != MOK/*&&liveinfo.isLive != NULL&&liveinfo.num >= 1*/)
		{
			liveinfo.num = -1;
			LOGE("ASFGetLivenessScore fail: %x ", res);
		}
		
	}
	else
	{
		LOGD("ASFProcess fail: %x ", res);
	}




	for (int i = 0; i < detectedFaces1.faceNum; i++)
	{
		FaceRect faces;
		faces.x = detectedFaces1.faceRect[i].left<0 ? 0 : detectedFaces1.faceRect[i].left;
		faces.y = detectedFaces1.faceRect[i].top<0 ? 0 : detectedFaces1.faceRect[i].top;
		faces.width = detectedFaces1.faceRect[i].right>frame.cols ? (frame.cols - detectedFaces1.faceRect[i].left - 1) : (detectedFaces1.faceRect[i].right - detectedFaces1.faceRect[i].left - 1);
		faces.height = detectedFaces1.faceRect[i].bottom > frame.rows ? (frame.rows - detectedFaces1.faceRect[i].top - 1) : (detectedFaces1.faceRect[i].bottom - detectedFaces1.faceRect[i].top - 1);

		if (faces.x<0 || faces.y<0 ||
			faces.width == 0 || faces.height == 0 ||
			faces.x + faces.width>frame.cols ||
			faces.y + faces.height>frame.rows)
		{
			continue;
		}


		int image_xcentor = frame.cols / 2;
		int image_ycentor = frame.rows / 2;

		double x_centor = 0, y_centor = 0;
		x_centor = faces.x + faces.width / 2;
		y_centor = faces.y + faces.height / 2;

		x_centor = x_centor / image_xcentor - 1;
		y_centor = y_centor / image_ycentor - 1;

		faces.xoffset = x_centor;
		faces.yoffset = y_centor;

// 		cv::Rect rect;
// 		rect.height = faces.height;
// 		rect.width = (faces.width / 4) * 4;
// 		rect.x = faces.x;
// 		rect.y = faces.y;
		faces.trackId = i;
		faces.score = 1.0;

		auto it = std::find(index_list.begin(), index_list.end(), i);
		if (MOK == res&&it != index_list.end())
		{
			int index_begin = it - index_list.begin();
			if (ageInfo.num >= index_begin && ageInfo.ageArray != NULL)
				faces.age = ageInfo.ageArray[index_begin];

			if (genderInfo.num >= index_begin && genderInfo.genderArray != NULL)
				faces.sex = (genderInfo.genderArray[index_begin] + 1);

			if (angleInfo.num >= index_begin && angleInfo.status != NULL&&*angleInfo.status == 0 && angleInfo.pitch != NULL && angleInfo.roll != NULL && angleInfo.yaw != NULL)
			{
				if (abs(angleInfo.pitch[index_begin]) < 20 &&
					abs(angleInfo.roll[index_begin]) < 20 &&
					abs(angleInfo.yaw[index_begin]) < 20)
					faces.takephoto = true;

				faces.pitch = angleInfo.pitch[index_begin];
				faces.roll = angleInfo.roll[index_begin];
				faces.yaw = angleInfo.yaw[index_begin];
				faces.has3D = true;
				faces.valid = 1;
			}
			if (liveinfo.isLive != NULL&&liveinfo.num >= index_begin)
			{
				if (liveinfo.isLive[index_begin] == 1)
					faces.isLive = true;
				//if (liveinfo.isLive != NULL&&liveinfo.num >= 1)
				//faces.isLive == (*liveinfo.isLive == 1);
			}
			
		}
		face_postion.push_back(faces);

	}
	if (index_list.size() > 0)
	{
		delete []detectedFaces2.faceRect;
		delete detectedFaces2.faceOrient;
	}

	if (face_postion.size() > 0)
		return true;
	return false;
}

bool robot_face_detect_arcsoft::init_engine(int minface_th)
{
	try
	{
		std::string appid;
		if (!common_config_get_field_string("arcsoft", "appid", &appid))
			appid = ARCSOFT_APPID;
		std::string key;
		if (!common_config_get_field_string("arcsoft", "key", &key))
			key = ARCSOFT_SDKKey;

		MRESULT res = MOK;
		ASF_ActiveFileInfo activeFileInfo = { 0 };
		res = ASFGetActiveFileInfo(&activeFileInfo);
		if (res != MOK)
		{
			LOGI("ASFGetActiveFileInfo fail: %d\n", res);
		}
	
		const ASF_VERSION version = ASFGetVersion();
		LOGI("\nVersion:%s\n", version.Version);
		LOGI("BuildDate:%s\n", version.BuildDate);
		LOGI("CopyRight:%s\n", version.CopyRight);


		//激活接口,首次激活需联网

		res = ASFActivation((char*)appid.c_str(), (char*)key.c_str());
		if (MOK == res || MERR_ASF_ALREADY_ACTIVATED == res)
		{
			LOGI("ALActivation sucess: %x ", res);

		}
		else
		{
			LOGE("ALActivation failed: %x ", res);
			isInitFailed = true;
			return false;
		}

		//初始化引擎
		MInt32 mask = ASF_FACE_DETECT | ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS;
		res = ASFInitEngine(ASF_DETECT_MODE_VIDEO, ASF_OP_0_ONLY, minface_th > 16 ? minface_th : 16, MAX_NUM_FACES, mask, &handle);
		if (res != MOK)
		{
			LOGE("ALInitEngine fail: %x ", res);
			isInitFailed = true;;
			return false;
		}
		else
			LOGI("ALInitEngine sucess: %x ", res);
		if (!handle)
		{
			return false;
		}

		res = ASFInitEngine(ASF_DETECT_MODE_IMAGE, ASF_OP_0_ONLY, minface_th > 16 ? minface_th : 16, MAX_NUM_FACES, mask, &handle_image);
		if (res != MOK)
		{
			LOGE("ALInitEngine fail: %x ", res);
			isInitFailed = true;;
			return false;
		}
		else
			LOGI("ALInitEngine sucess: %x ", res);
		if (!handle_image)
		{
			return false;
		}

		//设置活体置信度 SDK内部默认值为 IR：0.7  RGB：0.5（无特殊需要，可以不设置）
		ASF_LivenessThreshold threshold = { 0 };
		threshold.thresholdmodel_BGR = 0.5;
		threshold.thresholdmodel_IR = 0.7;
		res = ASFSetLivenessParam(handle_image, &threshold);
		if (res != MOK)
		{
			LOGE("ASFSetLivenessParam fail: %d\n", res);
		}
		else
			LOGI("RGB Threshold: %f  IR Threshold: %f\n", threshold.thresholdmodel_BGR, threshold.thresholdmodel_IR);
		res = ASFSetLivenessParam(handle, &threshold);
		if (res != MOK)
		{
			LOGE("ASFSetLivenessParam fail: %d\n", res);
		}
		else
			LOGI("RGB Threshold: %f  IR Threshold: %f\n", threshold.thresholdmodel_BGR, threshold.thresholdmodel_IR);



	}
	catch (std::exception* e)
	{
		LOGE("ALActivation exception: %s ", e->what());
		isInitFailed = true;
		return false;
	}
	catch (...)
	{
		LOGE("ALActivation exception");
		isInitFailed = true;
		return false;
	}
	return true;
}

robot_face_detect_arcsoft::~robot_face_detect_arcsoft()
{
	destory_handle();
}

void robot_face_detect_arcsoft::destory_handle()
{
	if (handle)
	{
		MRESULT res = ASFUninitEngine(handle);
		if (res != MOK)
			LOGE("ALUninitEngine fail: %d ", res);
		handle = NULL;
	}

	if (handle_image)
	{
		MRESULT res = ASFUninitEngine(handle_image);
		if (res != MOK)
			LOGE("ALUninitEngine fail: %d ", res);

		handle_image = NULL;
	}
}

#endif // HAS_ARCSOFT