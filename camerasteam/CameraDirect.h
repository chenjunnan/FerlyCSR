#ifndef CCAMERA_H
#define CCAMERA_H
#define __IDxtCompositor_INTERFACE_DEFINED__  
#define __IDxtAlphaSetter_INTERFACE_DEFINED__  
#define __IDxtJpeg_INTERFACE_DEFINED__  
#define __IDxtKey_INTERFACE_DEFINED__ 
#define WIN32_LEAN_AND_MEAN
#include <C:\Program Files\Windows Kits\8.0\Include\um\strmif.h>
/*#include "strmif.h"*/
#include <atlbase.h>

#include <qedit.h>
#include "dshow.h"

#include <windows.h>
#include "opencv2\core\types_c.h"
#include "opencv2\core\core_c.h"
#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "camerastreamstruct.h"
#include "common_helper.h"
#include "common_camera.h"



#define MYFREEMEDIATYPE(mt)	{if ((mt).cbFormat != 0)		\
					{CoTaskMemFree((PVOID)(mt).pbFormat);	\
					(mt).cbFormat = 0;						\
					(mt).pbFormat = NULL;					\
				}											\
				if ((mt).pUnk != NULL)						\
				{											\
					(mt).pUnk->Release();					\
					(mt).pUnk = NULL;						\
				}}		

struct CameraCapablity
{
	int min_width;
	int min_height;
	int min_fps;
	int max_width;
	int max_height;
	int max_fps;
	DWORD biCompression;
	WORD biBitCount;
	GUID vcode;
	GUID formattype;
	CameraCapablity()
	{
		min_width = 0;
		min_height = 0;
		min_fps = 0;
		max_width = 0;
		max_height = 0;
		max_fps = 0;
		biCompression = 0;
		biBitCount = 0;
	}
};
struct VideoConfig
{
	int height;
	int width;
	int channels;
	int fps;
	VIDEO_FORMAT format;
	int video_logicID;
	std::string stream_name;
	std::string device_name;
	int device_name_index;
	int angle;
	int mode;
	CameraCapablity cameracap;
	VideoConfig()
	{
		height = 0; width = 0; channels = 3; fps = 0; video_logicID = 0; angle = 0; format = VF_RGB; device_name_index = 0; mode = 0;
	}
};


struct Camera
{
	std::vector<CameraCapablity> capablity;
	std::string name;
	int index;
	Camera()
	{
		index = 0;
	}
};

struct VideoCaptureCapabilityWindows : public VideoCaptureList {
	unsigned int directShowCapabilityIndex;
	bool supportFrameRateControl;
	VideoCaptureCapabilityWindows() {
		directShowCapabilityIndex = 0;
		supportFrameRateControl = false;
	}
};



class CaptureSinkFilter;
class CSampleGrabberCB : public ISampleGrabberCB
{
public:
	std::vector<unsigned char> cache_image;
	CHANDLE m_ml;
	HANDLE hEvent;
	Timer_helper now;
	int i = 0, oldi = 0;
	CSampleGrabberCB()
	{
		hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_ml=common_mutex_create_threadlock();
	}
	~CSampleGrabberCB()
	{
		CloseHandle(hEvent);
		common_mutex_threadlock_release(m_ml);
	}
	STDMETHODIMP_(ULONG) AddRef()
	{
		return 2;
	}
	STDMETHODIMP_(ULONG) Release()
	{
		return 1;
	}
	STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
	{
		if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown)
		{
			*ppv = (void *) static_cast<ISampleGrabberCB*> (this);
			return NOERROR;
		}
		return E_NOINTERFACE;
	}
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample * pSample)
	{
		return 0;
	}
	STDMETHODIMP BufferCB(double dblSampleTime, BYTE * pBuffer, long lBufferSize)
	{
		LOCK_HELPER_AUTO_HANDLER(m_ml);
		if (cache_image.size() != lBufferSize)
			cache_image.resize(lBufferSize);
		memcpy(cache_image.data(), pBuffer, lBufferSize);
		SetEvent(hEvent);

		return 0;
	}

}; 



class CameraDirect :public VideoCaptureInterface
{
public:
	CameraDirect();

	virtual ~CameraDirect();

	bool Open(VideoConfig &vp, bool bDisplayProperties = false);

	bool SetParameter(CameraControlProperty key, int value,bool auto_or_manual);

	bool GetParameter(CameraControlProperty key, int &value, bool &auto_or_manual);

	BOOL isOpen(){ return m_bConnected; }



	void Close();

	static int CameraList(std::vector<Camera> &cclist);

	static int GetOptimization(Camera input, int setwidth=0, int setheight=0, int fps=0);

	int GetWidth(){return m_nWidth;} 

	int GetHeight(){return m_nHeight;}

	bool QueryFrame(cv::Mat& img, VIDEO_FORMAT type );

	virtual int onFrame(unsigned char* videoFrame,
		size_t videoFrameLength,
		const VideoCaptureList& frameInfo,
		long long captureTime = 0);

	CSampleGrabberCB m_cb;
private:
	int GetBestMatchedCapability(const VideoCaptureList& requested);
	int GetWindowsCapability(const int capabilityIndex, VideoCaptureCapabilityWindows& windowsCapability);

	bool _open(VideoConfig &vp, bool bDisplayProperties);

	static void PinList(IAMStreamConfig * config, std::vector<Camera> &cclist);

	bool BindFilter(std::string name,int camindex, IBaseFilter **pFilter);

	int CreateCapabilityMap();
	bool SetCameraOutput(const VideoConfig& requestedCapability);

	bool m_bConnected, m_bLock, m_bChanged;

	int m_nWidth, m_nHeight;

	long m_nBufferSize;

	cv::Mat m_tempcache;

	VIDEO_FORMAT m_media_format;

	CComPtr<IGraphBuilder> m_pGraph;

	CComPtr<ISampleGrabber> m_pSampleGrabber;

	CComPtr<IMediaControl> m_pMediaControl;

	CComPtr<IMediaEvent> m_pMediaEvent;

	CComPtr<IBaseFilter> m_pSampleGrabberFilter;
	CComPtr<IBaseFilter> m_pDeviceFilter;
	CComPtr<IBaseFilter> m_pNullFilter;

	CComPtr<IPin> m_pGrabberInput;
	CComPtr<IPin> m_pGrabberOutput;
	CComPtr<IPin> m_pCameraOutput;
	CComPtr<IPin> m_pNullInputPin;

// 	IPin *m_pGrabberOutput;
// 	IPin * m_pGrabberInput;
	CaptureSinkFilter* _sinkFilter;
	std::vector<VideoCaptureCapabilityWindows> _captureCapabilitiesWindows;
};

#endif 
