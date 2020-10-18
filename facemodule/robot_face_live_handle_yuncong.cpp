#include <windows.h>
#include "robot_face_live_handle_yuncong.h"

#include "opencv2\core\types_c.h"
#include "camerastreamstruct.h"
#include "opencv2\imgproc\types_c.h"
#include "opencv2\imgproc\imgproc.hpp"
#ifdef HAS_YUNCONG

#include "CWFaceDetection.h"
#include "CWFaceNisLiveness.h"
#include "robot_face_detect_interface.h"
#include "CWFaceAttribute.h"
#include "CWFaceNisLivService.h"
robot_face_live_handle_yuncong::robot_face_live_handle_yuncong()
{
	m_pNisLiveHandle = nullptr;
	m_w=common_mutex_create_multisignal();
	image_buffer.resize(256);
	m_c = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_congcastrate = false;
}



robot_face_live_handle_yuncong::~robot_face_live_handle_yuncong()
{
	close_live();
	// 程序退出时销毁句柄
	if (m_pNisLiveHandle != nullptr)
	{
		cwReleaseNirLivenessHandle(m_pNisLiveHandle);
		m_pNisLiveHandle = nullptr;
	}
	//cwCloseNirLivService();
	if (detecter[0].state())
		detecter[0].quit();
	if (detecter[1].state())
		detecter[1].quit();
	CloseHandle(m_c);
	common_mutex_multisignal_release(m_w);
}

int robot_face_live_handle_yuncong::init()
{
	int fun = 0;
	if (!common_config_get_field_int("face", "castrate", &fun))
		fun = 0;
	if (fun)m_congcastrate = true;
	cw_errcode_t errCode = CW_SDKLIT_OK;
	if (m_pNisLiveHandle == nullptr)
	{
		cw_nirliveness_err_t errLiveCode = CW_NIRLIV_OK;
		m_pNisLiveHandle = cwCreateNirLivenessHandle(&errLiveCode, NISLIVE_YUNCONG_MODLE, MATRIX_YUNCONG_MODLE,
			"./log", 0.5, CW_NIR_LIV_DET_BINOCULAR, CW_NIR_LIV_MODE_MULTI_FACES, "");
		if (CW_NIRLIV_OK != errLiveCode || nullptr == m_pNisLiveHandle)
		{
			LOGE("create nis handle error：%d", errLiveCode);
			return 1;
		}
	}
	if (!m_congcastrate)
	{
		if (detecter[1].init() || detecter[0].init() || !detecter[0].open() || !detecter[1].open())
		{
			LOGE("create yuncong detect handle error.");
			return 1;
		}
	}

	return 0;
}

bool robot_face_live_handle_yuncong::open(std::string streamname)
{
	if (m_pNisLiveHandle == nullptr || streamname.empty())
	{
		return false;
	}
	if (state())
	{
		return true;;
	}
	close_live();

	char name_buffer[1024] = { 0 };
	sprintf_s(name_buffer, 1024, "%s_reader", streamname.c_str());

	if (m_pm == NULL)
	{
		m_pm = Processon_Memory_Open(streamname.c_str(), name_buffer);
		if (!m_pm)
			return false;
		if (!malloc_buffer())
		{
			close_live();
			return false;
		}
	}

	while (1)
	{
		Message msg = getq(0);
		if (msg.msgType == -1)break;
		if (msg.msgType == 1)
		{
			FaceDetectData *faceiamge = (FaceDetectData *)msg.msgObject;
			SAFE_DELETE(faceiamge);
		}
	}
	m_streamname = streamname;

	return start();
}

bool robot_face_live_handle_yuncong::malloc_buffer()
{
	int pm_ret = -1;
	int outt = 60;
	while (pm_ret)
	{
		unsigned int data_len = image_buffer.size();
		pm_ret = Processon_Memory_Read(m_pm, image_buffer.data(), &data_len);
		if (pm_ret < 0)
		{
			LOGE("processon memory occur error %d!!", pm_ret);
			return false;
		}
		if (pm_ret > 0)
		{
			switch (pm_ret)
			{
			case 1001:
				if (outt-- <= 0)return false;
				break;
			case 1002:
				image_buffer.resize(data_len);
				break;
			default:
				break;
			}
			continue;

		}
	}
	return true;
}


void robot_face_live_handle_yuncong::close_live()
{
	if (state())
		quit();
	if (m_pm != NULL)
	{
		Processon_Memory_Close(m_pm);
		m_pm = NULL;
	}
}


void robot_face_live_handle_yuncong::get_stand_frame(unsigned char * buffer, unsigned int data_len, cv::Mat &frame)
{
	int type = CV_8UC3;

	frame.release();

	VideoMemoryHeadFormat * head_info = (VideoMemoryHeadFormat *)buffer;

	frame.create(head_info->height, head_info->width, type);

	memcpy(frame.data, buffer + sizeof(VideoMemoryHeadFormat), data_len - sizeof(VideoMemoryHeadFormat));

	if (head_info->color_type == VF_RGB)
	{
		cvtColor(frame, frame, CV_RGB2BGR);
	}

	if (head_info->color_type == VF_BGR)
	{
		;
	}

	if (head_info->height != CANMERA_HEIGHT || head_info->width != CANMERA_WIDTH)
	{
		cv::Size  des_size;

		double scalar = (double)frame.cols / frame.rows;
		double rate = 0;

		if (frame.cols > frame.rows)
		{
			rate = (double)CANMERA_WIDTH / frame.cols;
			des_size.width = CANMERA_WIDTH;
			des_size.height = (int)(rate*frame.rows);
		}
		else
		{
			rate = (double)CANMERA_WIDTH / frame.rows;
			des_size.height = CANMERA_WIDTH;
			des_size.width = (int)(rate*frame.cols);
		}

		cv::resize(frame, frame, des_size, 0, 0, CV_INTER_LINEAR);
	}

}

bool robot_face_live_handle_yuncong::handle_image(cv::Mat &image, robot_face_detect_interface *detecter)
{
	if (m_pm == NULL)return false;
	cv::Mat nisimage;
	bool newimage = false;
	unsigned int data_len = 0;
	while (!newimage)
	{
		data_len = image_buffer.size();
		int pm_ret = Processon_Memory_Read(m_pm, image_buffer.data(), &data_len);
		if (pm_ret < 0)
		{
			LOGE("processon memory occur error!!");
			return false;
		}

		if (pm_ret == 0)
		{
			VideoMemoryHeadFormat *head_info = (VideoMemoryHeadFormat *)image_buffer.data();

			if ((common_get_longcur_time_stampms() - head_info->stime)>30)
				continue;
			else
				newimage = true;
		}

	}
	
	get_stand_frame(image_buffer.data(), data_len, nisimage);

	if (nisimage.empty())
		return false;

	if (m_congcastrate)
	{
		if (detecter == NULL)return false;
		common_mutex_multisignal_wait(m_w);
		FaceRect faceRect; faceRect.height = 30; faceRect.width = 30;
		std::vector <FaceRect> face_postion;
		int state=detecter->DetectFace(image, faceRect, face_postion, "live");
		std::vector<cw_face_res_t> visvec, nirvec;
		for (int i = 0; i < face_postion.size();i++)
		{
			std::vector<char> data;
			cw_face_res_t face_l;
			if (face_postion[i].data.getDetectData("yuncong", data) && data.size() >= sizeof(cw_face_res_t))
			{
				memcpy(&face_l, data.data(), sizeof(cw_face_res_t));
				visvec.push_back(face_l);
			}


		}
		face_postion.clear();
		state = detecter->DetectFace(nisimage, faceRect, face_postion, "live");
		for (int i = 0; i < face_postion.size(); i++)
		{
			std::vector<char> data;
			cw_face_res_t face_l;
			if (face_postion[i].data.getDetectData("yuncong", data) && data.size() >= sizeof(cw_face_res_t))
			{
				memcpy(&face_l, data.data(), sizeof(cw_face_res_t));
				nirvec.push_back(face_l);
			}
		}
		image.copyTo(face_result.face_img);
		nisimage.copyTo(face_result.nis_img);

		face_result.status = GetNisLiveStateByMat(visvec, nirvec, face_result) == 0 ? false : true;
		common_mutex_multisignal_post(m_w);
	}
	else
	{
		if (!state()) return false;
		FaceDetectData *ptr = new FaceDetectData;
		if (!ptr) return false;
		common_mutex_multisignal_wait(m_w);
		Message msg;
		msg.msgType = 1;
		image.copyTo(ptr->face_img);
		nisimage.copyTo(ptr->nis_img);
		msg.msgObject = ptr;
		pushq(msg);
		WaitForSingleObject(m_c, -1);
		common_mutex_multisignal_post(m_w);
	}

	return true;
}

void robot_face_live_handle_yuncong::get_result(FaceDetectData &flist)
{
	common_mutex_multisignal_wait(m_w);
	flist = face_result;
	face_result.status = false;
	common_mutex_multisignal_post(m_w);
}

int robot_face_live_handle_yuncong::GetNisLiveStateByMat(std::vector<cw_face_res_t>  &visVec, std::vector<cw_face_res_t>  &nirVec, FaceDetectData &facedata)
{
	std::vector<cw_face_res_t> match_visVec,match_nirVec;

	for (int i = 0; i < visVec.size();i++)
	{
		for (int j = 0; j < nirVec.size();j++)
		{
			int xleft = MAX(visVec[i].faceRect.x, nirVec[j].faceRect.x);
			int xright = MIN(visVec[i].faceRect.width + visVec[i].faceRect.x, nirVec[j].faceRect.width + nirVec[j].faceRect.x);
			int ytop = MAX(visVec[i].faceRect.y, nirVec[j].faceRect.y);
			int ybottom = MIN(visVec[i].faceRect.height + visVec[i].faceRect.y, nirVec[j].faceRect.height + nirVec[j].faceRect.y);
			int totle = visVec[i].faceRect.width*visVec[i].faceRect.height + nirVec[j].faceRect.height*nirVec[j].faceRect.width;
			if (xleft < xright&&ytop < ybottom)
			{
				int aero = (xright - xleft)*(ybottom - ytop);
				if (((aero * 200) / totle)>40)
				{
					match_visVec.push_back(visVec[i]);
					match_nirVec.push_back(nirVec[j]);
					break;
				}
			}
		}
	}

	if (match_visVec.size() > 0 && match_nirVec.size() > 0 && match_nirVec.size() == match_visVec.size())
	{
		cv::Mat tempvis, tempnis;
		tempvis = facedata.face_img.clone();
		tempnis = facedata.nis_img.clone();
		cw_nirliv_detinfo_t detInfo;
		detInfo.nLandmarks = match_visVec[0].keypt.nkeypt;

		// 红外图片数据
		detInfo.pNirImg = new cw_nirliv_img_t();
		detInfo.pNirImg->data = (char*)tempnis.data;
		detInfo.pNirImg->width = tempnis.cols;
		detInfo.pNirImg->height = tempnis.rows;
		detInfo.pNirImg->format = CW_IMAGE_BGR888;   // 这里视频获取的都是3通道的bgr图

		// 可见光图片数据
		detInfo.pVisImg = new cw_nirliv_img_t();
		detInfo.pVisImg->data = (char*)tempvis.data;
		detInfo.pVisImg->width = tempvis.cols;
		detInfo.pVisImg->height = tempvis.rows;
		detInfo.pVisImg->format = CW_IMAGE_BGR888;   // 这里视频获取的都是3通道的bgr图

		// 由于创建句柄时选择的单人脸匹配，所以这里只传入第一个人脸数据
		// 如果选择多人脸匹配，这里就需要传入多个人脸数据
		detInfo.visFaceNum = 1;
		detInfo.nirFaceNum = 1;

		detInfo.pVisInfo = new cw_nirliv_face_param_t();  // 可见光人脸信息
		detInfo.pNirInfo = new cw_nirliv_face_param_t();   // 红外人脸信息

		cw_nirliv_res_t *res = new cw_nirliv_res_t[match_nirVec.size()];

		for (int i = 0; i < match_nirVec.size();i++)
		{
			detInfo.pVisInfo->pKeypoints = match_visVec[i].keypt.points;
			detInfo.pVisInfo->fKeyScore = match_visVec[i].keypt.keyptScore;
			detInfo.pVisInfo->fSkinScore = match_visVec[i].quality.scores[8];   // 肤色分

			detInfo.pNirInfo->pKeypoints = match_nirVec[i].keypt.points;
			detInfo.pNirInfo->fKeyScore = match_nirVec[i].keypt.keyptScore;
			detInfo.pNirInfo->fSkinScore = match_nirVec[i].quality.scores[8];   // 肤色分
			cw_nirliveness_err_t errNis = cwFaceNirLivenessDet(m_pNisLiveHandle, &detInfo, &res[i]);
			if (errNis != CW_NIRLIV_OK)
			{
				LOGE("nis detect error %d", errNis);
				res[i].score - 1;
			}
		}
	
		for (int i = 0; i < match_visVec.size(); i++)
		{
			if (res[i].score < 0)continue;
			FaceRect face;

			face.x = match_visVec[i].faceRect.x;
			face.y = match_visVec[i].faceRect.y;
			face.width = match_visVec[i].faceRect.width;
			face.height = match_visVec[i].faceRect.height;
			face.score = match_visVec[i].quality.scores[0];

			if (face.x<0 || face.y<0 ||
				face.width == 0 || face.height == 0 ||
				face.x + face.width>facedata.face_img.cols ||
				face.y + face.height>facedata.face_img.rows)
			{
				LOGE("error face data");
				continue;
			}
			face.isLive = res[i].livRst == CW_NIR_LIV_DET_LIVE;
			if (face.isLive)
				facedata.face_list.push_back(face);
		}

		delete detInfo.pNirImg;
		delete detInfo.pVisImg;
		delete detInfo.pVisInfo;
		delete detInfo.pNirInfo;
		delete res;
	}



	return facedata.face_list.size();

}

void robot_face_live_handle_yuncong::run()
{

	cw_face_res_t faceBuffers[MAX_NUM_FACES];
	bool loop = true;
	int live_count = 0;
	int sum_count = 0;
	int fps = 0;
	Timer_helper fpstime;
	LOGI("run live face detect ");
	while (loop)
	{
		Message msg;
		msg.msgType = 0;
		msg = getq(0);

		switch (msg.msgType)
		{
		case 1:
		{
			SetEvent(m_c);
			common_mutex_multisignal_wait(m_w);		
			m_result.clear();
			FaceDetectData *faceiamge = (FaceDetectData *)msg.msgObject;
			if (faceiamge)
			{
				std::vector<cw_face_res_t> visvec, nirvec;
				detecter[0].add_task(faceiamge->face_img);
				detecter[1].add_task(faceiamge->nis_img);
				detecter[0].get_result(visvec);
				detecter[1].get_result(nirvec);
				faceiamge->status = GetNisLiveStateByMat(visvec, nirvec, *faceiamge) == 0 ? false:true;
				face_result = *faceiamge;
			}
			SAFE_DELETE(faceiamge);
			common_mutex_multisignal_post(m_w);
			break;
		}
		case  2:
		{

		}break;
		case _EXIT_THREAD_:
			loop = false;
		case -1:
		case  0:
			break;
		default:
			LOGE("unknow face detect message type  %d", msg.msgType);
			break;
		}
	}
	LOGW("exit live detect thread");

}
#endif // HAS_YUNCONG