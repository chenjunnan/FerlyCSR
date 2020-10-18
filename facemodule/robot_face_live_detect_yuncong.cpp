#include <windows.h>
#include "robot_face_live_detect_yuncong.h"
#ifdef HAS_YUNCONG

#include "CWFaceDetection.h"





robot_face_live_detect_yuncong::robot_face_live_detect_yuncong()
{
	m_w = common_mutex_create_multisignal();
	m_pDetHandle = nullptr;
	m_c = CreateEvent(NULL, FALSE, FALSE, NULL);
}


robot_face_live_detect_yuncong::~robot_face_live_detect_yuncong()
{
	// 程序退出时销毁句柄
	if (m_pDetHandle != nullptr)
	{
		cwReleaseDetHandle(m_pDetHandle);
		m_pDetHandle = nullptr;
	}
	common_mutex_multisignal_release(m_w);
	CloseHandle(m_c);
	if (state())quit();
}

int robot_face_live_detect_yuncong::init()
{
	cw_errcode_t errCode = CW_SDKLIT_OK;
	if (m_pDetHandle == nullptr)
	{
		m_pDetHandle = cwCreateDetHandle(&errCode, DETECT_YUNCONG_MODLE, LICENCE);
		if (CW_SDKLIT_OK != errCode || nullptr == m_pDetHandle)
		{
			LOGE(("create nir handle error：%d"), errCode);
			return 1;
		}

		cw_det_param_t param;
		cwGetFaceParam(m_pDetHandle, &param);
		param.minSize = 30;
		param.maxSize = 600;
		param.pConfigFile = DETECT_YUNCONG_MODLE;    // 设置接口功能参数
		cwSetFaceParam(m_pDetHandle, &param);
	}
	return 0;
}


bool robot_face_live_detect_yuncong::open()
{
	if (nullptr == m_pDetHandle)
		return false;
	if (state())
		return true;;
	return start();
}


void robot_face_live_detect_yuncong::add_task(cv::Mat &image)
{
	if (!state()) return;
	cv::Mat *ptr = new cv::Mat;
	if (!ptr) return;
	common_mutex_multisignal_wait(m_w);
	Message msg;
	msg.msgType = 1;
	image.copyTo(*ptr);
	msg.msgObject = ptr;
	pushq(msg);
	WaitForSingleObject(m_c, -1);
	common_mutex_multisignal_post(m_w);
}

void robot_face_live_detect_yuncong::get_result(std::vector<cw_face_res_t >&list)
{
	common_mutex_multisignal_wait(m_w);
	list = m_res;
	common_mutex_multisignal_post(m_w);
}

int robot_face_live_detect_yuncong::get_faceinfo_bymat(const cv::Mat &matImage, cw_face_res_t* pFaceBuffer)
{
	if (nullptr == m_pDetHandle)
		return 0;
	if (matImage.empty())
	{
		return 0;
	}

	cw_img_t srcImg;
	srcImg.frameId = 0;
	srcImg.data = (char*)matImage.data;
	srcImg.width = matImage.cols;
	srcImg.height = matImage.rows;
	srcImg.angle = CW_IMAGE_ANGLE_0;
	srcImg.mirror = CW_IMAGE_MIRROR_NONE;
	if (matImage.channels() == 1)
	{
		srcImg.format = CW_IMAGE_GRAY8;
	}
	else if (matImage.channels() == 3)
	{
		srcImg.format = CW_IMAGE_BGR888;
	}
	else if (matImage.channels() == 4)
	{
		srcImg.format = CW_IMAGE_BGRA8888;
	}
	else
	{
		return 0;
	}

	int iFaceNum = 0;
	// 检测，关键点与质量分
	cw_errcode_t errCode = cwFaceDetection(m_pDetHandle, &srcImg, pFaceBuffer, MAX_NUM_FACES, &iFaceNum, CW_OP_DET | CW_OP_KEYPT | CW_OP_QUALITY);
	if (errCode != CW_SDKLIT_OK)
	{
		return 0;
	}
	if (iFaceNum < 1)
	{
		return 0;
	}

	return iFaceNum;
}



void robot_face_live_detect_yuncong::run()
{
	bool loop = true;
	LOGT("run live face detect ");
	while (loop)
	{
		Message msg;
		msg = getq(-1);

		switch (msg.msgType)
		{
		case 1:
		{
			SetEvent(m_c);
			common_mutex_multisignal_wait(m_w);
			m_res.clear();
			cv::Mat *faceiamge = (cv::Mat *)msg.msgObject;
			if (faceiamge)
			{
				cw_face_res_t faceinfo[MAX_NUM_FACES];
				int facenum = get_faceinfo_bymat(*faceiamge, faceinfo);
				for (int i = 0; i < facenum;i++)
					m_res.push_back(faceinfo[i]);
			}
			SAFE_DELETE(faceiamge);
			common_mutex_multisignal_post(m_w);
		}
			break;
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
}
#endif // HAS_YUNCONG