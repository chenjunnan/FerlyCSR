#pragma once
#include <windows.h>

#include "uuids.h"


// DEFINE_GUID(MEDIASUBTYPE_I420,
// 	0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
const GUID MEDIASUBTYPE_I420 = { 0x30323449, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };

enum VideoType
{
	kUnknown,
	kIYUV,
	kMJPEG,
	kI420,
	kYUY2,
	kUYVY,
	kRGB24,
	kRGB565
};


struct VideoCaptureList {
	int width;
	int height;
	int maxFPS;
	VideoType videoType;
	bool interlaced;

	VideoCaptureList() {
		width = 0;
		height = 0;
		maxFPS = 0;
		videoType = VideoType::kUnknown;
		interlaced = false;
	};
	bool operator!=(const VideoCaptureList& other) const {
		if (width != other.width)
			return true;
		if (height != other.height)
			return true;
		if (maxFPS != other.maxFPS)
			return true;
		if (videoType != other.videoType)
			return true;
		if (interlaced != other.interlaced)
			return true;
		return false;
	}
	bool operator==(const VideoCaptureList& other) const {
		return !operator!=(other);
	}
};
class VideoCaptureInterface {
public:

	virtual int onFrame(unsigned char* videoFrame,
		size_t videoFrameLength,
		const VideoCaptureList& frameInfo,
		long long captureTime = 0){
		return 1;
	}

protected:
	~VideoCaptureInterface() {}
};
