#pragma  once

enum VIDEO_FORMAT
{
	VF_BGR = 0,
	VF_RGB,
	VF_RGBA,
	VF_YUV420P,
	VF_YUY2,
	VF_MJPEG
};
struct VideoMemoryHeadFormat
{
	int id;
	int width;
	int height;
	int channels;
	int angle;
	int color_type;
	unsigned long long stime;
	int fps;
	VideoMemoryHeadFormat()
	{
		id = 0; width = 0; height = 0; channels = 0; angle = 0; color_type = 0; stime = 0; fps = 30;
	}
};
