#include "PsycheRTCAgoraVideoPub.h"
#include "IAgoraMediaEngine.h"
#include "AgoraBase.h"
#include "agora\AgoraObject.h"
#include "opencv2\core\core.hpp"
#include "opencv2\imgproc\imgproc.hpp"


PsycheRTCAgoraVideoPub::PsycheRTCAgoraVideoPub()
{
}


int PsycheRTCAgoraVideoPub::StartUp(std::string streamname)
{
	if (state())return 0;
	m_reader.set_stream(streamname, "AgroaReader");
	if (m_reader.startup_receive() != 0)
		return 92001;
	return start() ? 0 : 92002;
}

PsycheRTCAgoraVideoPub::~PsycheRTCAgoraVideoPub()
{
}

void PsycheRTCAgoraVideoPub::run()
{
	bool loop = true;
	agora::util::AutoPtr<agora::media::IMediaEngine> mediaEngine;
	agora::rtc::IRtcEngine*  lpRtcEngine = CAgoraObject::GetEngine();
	mediaEngine.queryInterface(lpRtcEngine, agora::AGORA_IID_MEDIA_ENGINE);
	VIDEO_MEDIADATA vdata;
	agora::media::ExternalVideoFrame ExVF;
	memset(&ExVF, 0x00, sizeof(agora::media::ExternalVideoFrame));
	cv::Mat temp,dest;
	while (loop)
	{
		Message msg = getq(0);
		if (msg.msgType == _EXIT_THREAD_){loop = false; continue;}
		if (m_reader.get_media(vdata))
		{
			if (vdata.video_format.color_type == VF_BGR)
			{
				if (temp.empty())
				{
					temp.create(vdata.video_format.height, vdata.video_format.width, CV_8UC3);
					dest.create(vdata.video_format.height, vdata.video_format.width, CV_8UC4);
				}
				memcpy(temp.data, vdata.data.data(), vdata.data.size());
				cv::cvtColor(temp, dest, CV_BGR2BGRA);
				ExVF.buffer = dest.data;
				ExVF.format = agora::media::ExternalVideoFrame::VIDEO_PIXEL_FORMAT::VIDEO_PIXEL_BGRA;
				ExVF.height = vdata.video_format.height;
				ExVF.type = agora::media::ExternalVideoFrame::VIDEO_BUFFER_TYPE::VIDEO_BUFFER_RAW_DATA;
				ExVF.stride = vdata.video_format.width;
				ExVF.timestamp = common_get_longcur_time_stampms();
				int ret = mediaEngine->pushVideoFrame(&ExVF);
				if (ret)LOGE("push data error %d", ret);
			}
			else
			{
				LOGE("not support video format ");
			}

		}
		else
		{
			LOGE("Get media failed .");
		}
	}
	m_reader.quit();
}
