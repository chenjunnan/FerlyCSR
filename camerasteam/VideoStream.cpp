#include "VideoStream.h"
#include "memorySwaper.h"
#include "camerastreamstruct.h"





VideoStream::VideoStream(VideoConfig vc)
{
	m_config = vc;
	m_video = NULL;
}

bool VideoStream::init_memory()
{

	if (!Processon_Memory_Create(m_config.stream_name.c_str(), 
		m_config.width * m_config.height * m_config.channels + sizeof(VideoMemoryHeadFormat), m_config.fps))
	{
		LOGE("cant create %s process memory!", m_config.stream_name.c_str());
		return false;
	}
	m_pm = Processon_Memory_Open(m_config.stream_name.c_str(), "writer");
	if (!m_pm)
	{
		LOGE("cant open %s process memory!", m_config.stream_name.c_str());
		return false;
	}
	
	return true;
}


bool VideoStream::open()
{
	if (!m_pm)
		return false;
	if (state())
		return true;
	close_device();
	if (m_config.mode == 0)
	{
		if (open_byopencv())
			return start();
	}
	else
	{
		if (m_cd.Open(m_config))
			return start();
	}
	return false;
}

bool VideoStream::set(int t, int v, bool a)
{
	if (m_config.mode == 0)
		return false;//todo opencv

	return m_cd.SetParameter((CameraControlProperty)t, v, a);
}

bool VideoStream::get(int t, int& v, bool& a)
{
	if (m_config.mode == 0)
		return false;//todo opencv

	return m_cd.GetParameter((CameraControlProperty)t, v, a);
}

void VideoStream::getConfig(VideoConfig& vc)
{
	vc = m_config;
}

bool VideoStream::open_byopencv()
{
	m_video = cvCaptureFromCAM(m_config.video_logicID);

	if (!m_video)
	{
		LOGE("cant open %d camera!", m_config.video_logicID);
		return false;
	}


	int ret1 = cvSetCaptureProperty(m_video, CV_CAP_PROP_FRAME_HEIGHT, m_config.height);
	int ret2 = cvSetCaptureProperty(m_video, CV_CAP_PROP_FRAME_WIDTH, m_config.width);
	int ret3 = cvSetCaptureProperty(m_video, CV_CAP_PROP_FPS, m_config.fps);
	//int ret4 = cvSetCaptureProperty(m_video, CV_CAP_PROP_FORMAT, m_config.width);
	if (ret1 != 1 || ret2 != 1 || ret3 != 1)
	{
		LOGE("camera %d not support %dx%d  .FPS:%d!", m_config.video_logicID, m_config.width, m_config.height, m_config.fps);
		cvReleaseCapture(&m_video);
		m_video = NULL;
		return false;
	}
	return true;
}

VideoStream::~VideoStream()
{
	if (state())
		quit();

	close_device();

	Processon_Memory_Close(m_pm);
	Processon_Memory_Destory(m_config.stream_name.c_str());
}

void VideoStream::run()
{
	if (!m_pm)return ;

	if (!m_video &&!m_cd.isOpen())
		return;
	cv::Mat p;
	int image_size = m_config.width*m_config.height*m_config.channels;
	unsigned char *image_buffer = new unsigned char[image_size + sizeof(VideoMemoryHeadFormat)];
	VideoMemoryHeadFormat head_info;
	bool loop = true;
	while (loop)
	{
		Message msg = getq(0);
		switch (msg.msgType)
		{
		case _EXIT_THREAD_:
			loop=false;
			break;
		default:;
		}
		IplImage *temp;
		IplImage *img;
		if (m_config.mode == 0)
		{
			//m_videocap >> p;
			img = cvQueryFrame(m_video);

			if (!img)
			{
				close_device();
				common_thread_sleep(1000);
				open_byopencv();
				continue;
			}
			
			head_info.width = img->width; head_info.height = img->height;
			head_info.channels = img->nChannels;
			head_info.angle = m_config.angle;
			head_info.color_type = m_config.format;
			head_info.id = m_config.video_logicID;
			head_info.fps = m_config.fps;
			if (m_config.format != VF_BGR)
			{
				IplImage *dst_image = cvCreateImage(cvGetSize(img), 8, img->nChannels);
				if (!dst_image)continue;
				int codec = CV_BGR2RGB;
				if (m_config.format == VF_RGB)
					codec = CV_BGR2RGB;
				if (m_config.format == VF_RGBA)
					codec = CV_BGR2RGBA;
				if (m_config.format == VF_YUV420P)
					codec = CV_BGR2YUV_I420;

				cvCvtColor(img, dst_image, codec);
				temp = dst_image;
			}
			else
				temp = img;
			head_info.stime = common_get_longcur_time_stampms();
			memcpy(image_buffer, &head_info, sizeof(VideoMemoryHeadFormat));
			memcpy(image_buffer + sizeof(VideoMemoryHeadFormat), temp->imageData, temp->imageSize);
			if (!Processon_Memory_Write(m_pm, image_buffer, image_size + sizeof(VideoMemoryHeadFormat)))
			{
				LOGE("Write processon memory error!!!");
			}
			if (m_config.format != VF_BGR)
				cvReleaseImage(&temp);
		}
		else
		{
			if (!m_cd.QueryFrame(p, VF_BGR) || p.empty())
			{
				close_device();
				common_thread_sleep(1000);
				m_cd.Open(m_config);
				continue;
			}
			head_info.width = p.cols; head_info.height = p.rows;
			head_info.channels = p.channels();
			head_info.angle = m_config.angle;
			head_info.color_type = m_config.format;
			head_info.id = m_config.video_logicID;
			head_info.fps = m_config.fps;
			head_info.stime = common_get_longcur_time_stampms();
			memcpy(image_buffer, &head_info, sizeof(VideoMemoryHeadFormat));
			memcpy(image_buffer + sizeof(VideoMemoryHeadFormat), p.data, p.cols*p.rows * p.channels());
			if (!Processon_Memory_Write(m_pm, image_buffer, image_size + sizeof(VideoMemoryHeadFormat)))
			{
				LOGE("Write processon memory error!!!");
			}
		}
	

	}
	SAFE_DELETE(image_buffer);
	close_device();
	LOGW("ID: %d camera  exit",m_config.video_logicID);
}

void VideoStream::close_device()
{
	if (m_video)
	{
		cvReleaseCapture(&m_video);
		m_video = NULL;
	}
	if (m_cd.isOpen())
		m_cd.Close();
}
