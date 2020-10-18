
#include <C:\Program Files\Windows Kits\8.0\Include\um\strmif.h>
#include "CameraDirect.h"

#include <windows.h>
#include "xCommon.h"
#include <opencv2\imgproc\imgproc.hpp>
#include <Dvdmedia.h>
#include <initguid.h>
#include <mtype.h>

#include "sink_filter_ds.h"
#pragma comment(lib,"Strmiids.lib") 
#pragma comment(lib,"winmm.lib") 


CameraDirect::CameraDirect()
{
	m_bConnected = m_bLock = m_bChanged = false;
	m_nWidth = m_nHeight = 0;
	m_nBufferSize = 0;

	m_pNullFilter = NULL;
	m_pMediaEvent = NULL;
	m_pSampleGrabberFilter = NULL;
	m_pGraph = NULL;
	m_media_format = VF_BGR;
	_sinkFilter = NULL;
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
}

CameraDirect::~CameraDirect()
{
	Close();
	CoUninitialize();
}

void CameraDirect::Close()
{
	if(m_bConnected)
	{
		m_pMediaControl->Stop();
	}
	if (m_pGraph) {
		if (_sinkFilter)
			m_pGraph->RemoveFilter(_sinkFilter);
		if (m_pDeviceFilter)
			m_pGraph->RemoveFilter(m_pDeviceFilter);
		HRESULT hr = m_pGraph->Disconnect(m_pGrabberOutput);
		hr += m_pGraph->Disconnect(m_pGrabberInput);


	}
	if (_sinkFilter)
	{
		_sinkFilter->Release();
		_sinkFilter = NULL;
	}

	_sinkFilter = NULL;
	m_pGraph = NULL;
	m_pDeviceFilter = NULL;
	m_pMediaControl = NULL;
	m_pSampleGrabberFilter = NULL;
	m_pGrabberInput = NULL;
	m_pGrabberOutput = NULL;
	m_pCameraOutput = NULL;
	m_pMediaEvent = NULL;
	m_pNullFilter = NULL;
	m_pNullInputPin = NULL;

	m_bConnected = m_bLock = m_bChanged = false;
	m_nWidth = m_nHeight = 0;
	m_nBufferSize = 0;
}

LONGLONG GetMaxOfFrameArray(LONGLONG* maxFps, long size) {
	LONGLONG maxFPS = maxFps[0];
	for (int i = 0; i < size; i++) {
		if (maxFPS > maxFps[i])
			maxFPS = maxFps[i];
	}
	return maxFPS;
}

int GuidToString(const GUID &guid, char* buffer){
	int buf_len = 64;
	sprintf_s(
		buffer,
		buf_len,
		"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	return 0;
}

int GetMajorType(GUID guid, char* buffer) {
	memset(buffer, 0, 256);
	if (guid == MEDIATYPE_Video) {
		sprintf_s(buffer, 256, "MEDIATYPE_Video");
		return 0;
	}
	if (guid == MEDIATYPE_Audio) {
		sprintf_s(buffer, 256, "MEDIATYPE_Audio");
		return 0;
	}
	if (guid == MEDIASUBTYPE_RGB24) {
		sprintf_s(buffer, 256, "MEDIATYPE_Stream");
		return 0;
	}
	return -1;
}

int GetSubType(GUID guid, char* buffer) {
	memset(buffer, 0, 256);
	if (guid == MEDIASUBTYPE_YUY2){
		sprintf_s(buffer, 256, "MEDIASUBTYPE_YUY2");
		return 0;
	}
	if (guid == MEDIASUBTYPE_MJPG) {
		sprintf_s(buffer, 256, "MEDIASUBTYPE_MJPG");
		return 0;
	}
	if (guid == MEDIASUBTYPE_RGB24) {
		sprintf_s(buffer, 256, "MEDIASUBTYPE_RGB24");
		return 0;
	}
	return -1;
}

int GetFormatType(GUID guid, char* buffer) {
	memset(buffer, 0, 256);
	if (guid == FORMAT_VideoInfo) {
		sprintf_s(buffer, 256, "FORMAT_VideoInfo");
		return 0;
	}
	if (guid == FORMAT_VideoInfo2) {
		sprintf_s(buffer, 256, "FORMAT_VideoInfo2");
		return 0;
	}
	return -1;
}


BOOL PinMatchesCategory(IPin* pPin, REFGUID Category) {
	BOOL bFound = FALSE;
	IKsPropertySet* pKs = NULL;
	HRESULT hr = pPin->QueryInterface(IID_PPV_ARGS(&pKs));
	if (SUCCEEDED(hr)) {
		GUID PinCategory;
		DWORD cbReturned;
		hr = pKs->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0,
			&PinCategory, sizeof(GUID), &cbReturned);
		if (SUCCEEDED(hr) && (cbReturned == sizeof(GUID))) {
			bFound = (PinCategory == Category);
		}
		pKs->Release();
	}
	return bFound;
}

CComPtr<IPin> GetInputPin(IBaseFilter* filter) {
	HRESULT hr;
	CComPtr<IPin> pin = NULL;
	CComPtr<IEnumPins> pPinEnum = NULL;
	filter->EnumPins(&pPinEnum);
	if (pPinEnum == NULL) {
		return NULL;
	}

	// get first unconnected pin
	hr = pPinEnum->Reset();  // set to first pin

	while (S_OK == pPinEnum->Next(1, &pin, NULL)) {
		PIN_DIRECTION pPinDir;
		pin->QueryDirection(&pPinDir);
		if (PINDIR_INPUT == pPinDir)  // This is an input pin
		{
			IPin* tempPin = NULL;
			if (S_OK != pin->ConnectedTo(&tempPin))  // The pint is not connected
			{
				//pPinEnum->Release();
				return pin;
			}
		}
/*		pin->Release();*/
	}
	//pPinEnum->Release();
	return NULL;
}

CComPtr<IPin> GetOutputPin(IBaseFilter* filter, REFGUID Category) {
	HRESULT hr;
	CComPtr<IPin> pin = NULL;
	CComPtr<IEnumPins> pPinEnum = NULL;
	filter->EnumPins(&pPinEnum);
	if (pPinEnum == NULL) {
		return NULL;
	}
	// get first unconnected pin
	hr = pPinEnum->Reset();  // set to first pin
	while (S_OK == pPinEnum->Next(1, &pin, NULL)) {
		PIN_DIRECTION pPinDir;
		pin->QueryDirection(&pPinDir);
		if (PINDIR_OUTPUT == pPinDir)  // This is an output pin
		{
			if (Category == GUID_NULL || PinMatchesCategory(pin, Category)) {
				//pPinEnum->Release();
				return pin;
			}
		}
		//pin->Release();
		pin = NULL;
	}
	//pPinEnum->Release();
	return NULL;
}


int CameraDirect::GetBestMatchedCapability(
	const VideoCaptureList& requested) {

	int bestformatIndex = -1;
	int bestWidth = 0;
	int bestHeight = 0;
	int bestFrameRate = 0;
	VideoType bestVideoType = VideoType::kUnknown;

	const int numberOfCapabilies =
		static_cast<int>(_captureCapabilitiesWindows.size());

	for (int tmp = 0; tmp < numberOfCapabilies;
		++tmp)  // Loop through all capabilities
	{
		VideoCaptureList& capability = _captureCapabilitiesWindows[tmp];

		const int diffWidth = capability.width - requested.width;
		const int diffHeight = capability.height - requested.height;
		const int diffFrameRate = capability.maxFPS - requested.maxFPS;

		const int currentbestDiffWith = bestWidth - requested.width;
		const int currentbestDiffHeight = bestHeight - requested.height;
		const int currentbestDiffFrameRate = bestFrameRate - requested.maxFPS;
		
		if (requested.videoType != VideoType::kUnknown &&
			(capability.videoType != requested.videoType &&
			capability.videoType != VideoType::kYUY2 &&
			capability.videoType != VideoType::kRGB24&&
			capability.videoType != VideoType::kMJPEG
			))//now only deal with yuy2 mjpeg ,and  bgr
			continue;

		if ((diffHeight >= 0 &&diffHeight <= abs(currentbestDiffHeight))  // Height better or equalt that previouse.
			|| (currentbestDiffHeight < 0 && diffHeight >= currentbestDiffHeight)) 
		{
			if (diffHeight ==currentbestDiffHeight)  // Found best height. Care about the width)
			{
				if ((diffWidth >= 0 &&diffWidth <= abs(currentbestDiffWith))  // Width better or equal
					|| (currentbestDiffWith < 0 && diffWidth >= currentbestDiffWith)) 
				{
					if (diffWidth == currentbestDiffWith &&
						diffHeight == currentbestDiffHeight)  // Same size as previously
					{
						// Also check the best frame rate if the diff is the same as
						// previouse
						if (((diffFrameRate >= 0 &&diffFrameRate <=currentbestDiffFrameRate)  // Frame rate to high but
							// better match than previouse
							// and we have not selected IUV
							|| (currentbestDiffFrameRate < 0 &&diffFrameRate >=currentbestDiffFrameRate))  // Current frame rate is
							// lower than requested.
							// This is better.
							) 
						{
							if ((currentbestDiffFrameRate ==diffFrameRate)  // Same frame rate as previous  or frame rate
								// allready good enough
								|| (currentbestDiffFrameRate >= 0)) 
							{
								if (bestVideoType != VideoType::kYUY2 ||
									bestVideoType != VideoType::kRGB24 ||
									bestVideoType != requested.videoType)
								{
									if (requested.videoType != VideoType::kUnknown &&
										(capability.videoType == requested.videoType ||
										capability.videoType == VideoType::kYUY2 ||
										capability.videoType == VideoType::kRGB24)) //first yuy2 rgb
									{
										bestVideoType = capability.videoType;
										bestformatIndex = tmp;
									}
								}

								// If width height and frame rate is full filled we can use the
								// camera for encoding if it is supported.
								if (bestVideoType == VideoType::kUnknown &&
									capability.height == requested.height &&
									capability.width == requested.width &&
									capability.maxFPS >= requested.maxFPS) {
									bestformatIndex = tmp;
								}
							}
							else  // Better frame rate
							{
								bestWidth = capability.width;
								bestHeight = capability.height;
								bestFrameRate = capability.maxFPS;
								bestVideoType = capability.videoType;
								bestformatIndex = tmp;
							}
						}
					}
					else  // Better width than previously
					{
						bestWidth = capability.width;
						bestHeight = capability.height;
						bestFrameRate = capability.maxFPS;
						bestVideoType = capability.videoType;
						bestformatIndex = tmp;
					}
				}     // else width no good
			}
			else  // Better height
			{
				bestWidth = capability.width;
				bestHeight = capability.height;
				bestFrameRate = capability.maxFPS;
				bestVideoType = capability.videoType;
				bestformatIndex = tmp;
			}
		}  // else height not good
	}    // end for

	// Copy the capability
	if (bestformatIndex < 0)
		return -1;

	return bestformatIndex;
}


int CameraDirect::GetWindowsCapability(
	const int capabilityIndex,
	VideoCaptureCapabilityWindows& windowsCapability) {

	if (capabilityIndex < 0 || static_cast<size_t>(capabilityIndex) >=
		_captureCapabilitiesWindows.size()) {
		return -1;
	}

	windowsCapability = _captureCapabilitiesWindows[capabilityIndex];
	return 0;
}


char *getTypeName(VideoType t)
{
	if (t == VideoType::kI420)return "i420";
	if (t == VideoType::kMJPEG)return "mjpeg";
	if (t == VideoType::kYUY2)return "yuy2";
	if (t == VideoType::kUYVY)return "uyvy";
	if (t == VideoType::kRGB24)return "rgb24";
	if (t == VideoType::kRGB565)return "rgb555";
	if (t == VideoType::kIYUV)return "iyuv";
	return "unknown";
}


int CameraDirect::CreateCapabilityMap()

{
	
	char productId[1024];
	_captureCapabilitiesWindows.clear();
	CComPtr<IPin> outputCapturePin = GetOutputPin(m_pDeviceFilter, GUID_NULL);
	if (!outputCapturePin) {
		return -1;
	}

	CComPtr<IAMExtDevice> extDevice = NULL;
	HRESULT hr =m_pDeviceFilter->QueryInterface(IID_IAMExtDevice, (void**)&extDevice);
	if (SUCCEEDED(hr) && extDevice) {
	}
	
	CComPtr<IAMStreamConfig> streamConfig = NULL;
	hr = outputCapturePin->QueryInterface(IID_IAMStreamConfig,
		(void**)&streamConfig);
	if (FAILED(hr)) {
		return -1;
	}

	// this  gets the FPS
	CComPtr<IAMVideoControl> videoControlConfig = NULL;
	HRESULT hrVC = m_pDeviceFilter->QueryInterface(IID_IAMVideoControl,
		(void**)&videoControlConfig);
	if (FAILED(hrVC)) {
		;
	}

	AM_MEDIA_TYPE* pmt = NULL;
	VIDEO_STREAM_CONFIG_CAPS caps;
	int count, size;

	hr = streamConfig->GetNumberOfCapabilities(&count, &size);
	if (FAILED(hr)) {
		return -1;
	}

	bool supportFORMAT_VideoInfo2 = false;
	bool supportFORMAT_VideoInfo = false;
	bool foundInterlacedFormat = false;
	GUID preferedVideoFormat = FORMAT_VideoInfo;
	for (int tmp = 0; tmp < count; ++tmp) {
		hr = streamConfig->GetStreamCaps(tmp, &pmt, reinterpret_cast<BYTE*>(&caps));
		if (!FAILED(hr)) {
			if (pmt->majortype == MEDIATYPE_Video &&
				pmt->formattype == FORMAT_VideoInfo2) {
				supportFORMAT_VideoInfo2 = true;
				VIDEOINFOHEADER2* h =
					reinterpret_cast<VIDEOINFOHEADER2*>(pmt->pbFormat);
				if (h == NULL) return false;

				foundInterlacedFormat |=
					h->dwInterlaceFlags &
					(AMINTERLACE_IsInterlaced | AMINTERLACE_DisplayModeBobOnly);
			}
			if (pmt->majortype == MEDIATYPE_Video &&
				pmt->formattype == FORMAT_VideoInfo) {
				supportFORMAT_VideoInfo = true;
			}
			if (pmt->pbFormat)
				CoTaskMemFree(pmt->pbFormat);
			CoTaskMemFree(pmt);

		}
		
	}

	if (supportFORMAT_VideoInfo2) {
		if (supportFORMAT_VideoInfo && !foundInterlacedFormat) {
			preferedVideoFormat = FORMAT_VideoInfo;
		}
		else {
			preferedVideoFormat = FORMAT_VideoInfo2;
		}
	}

	for (int tmp = 0; tmp < count; ++tmp) {
		hr = streamConfig->GetStreamCaps(tmp, &pmt, reinterpret_cast<BYTE*>(&caps));
		if (FAILED(hr)) {
			return -1;
		}

		if (pmt->majortype == MEDIATYPE_Video &&
			pmt->formattype == preferedVideoFormat) {
			VideoCaptureCapabilityWindows capability;
			long long avgTimePerFrame = 0;

			if (pmt->formattype == FORMAT_VideoInfo) {
				VIDEOINFOHEADER* h = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
				if (h == NULL) return false;

				capability.directShowCapabilityIndex = tmp;
				capability.width = h->bmiHeader.biWidth;
				capability.height = h->bmiHeader.biHeight;
				avgTimePerFrame = h->AvgTimePerFrame;
			}
			if (pmt->formattype == FORMAT_VideoInfo2) {
				VIDEOINFOHEADER2* h =
					reinterpret_cast<VIDEOINFOHEADER2*>(pmt->pbFormat);
				if (h == NULL) return false;

				capability.directShowCapabilityIndex = tmp;
				capability.width = h->bmiHeader.biWidth;
				capability.height = h->bmiHeader.biHeight;
				capability.interlaced =
					h->dwInterlaceFlags &
					(AMINTERLACE_IsInterlaced | AMINTERLACE_DisplayModeBobOnly);
				avgTimePerFrame = h->AvgTimePerFrame;
			}

			if (hrVC == S_OK) {
				LONGLONG* frameDurationList=NULL;
				LONGLONG maxFPS;
				long listSize;
				SIZE size;
				size.cx = capability.width;
				size.cy = capability.height;

	

				hrVC = videoControlConfig->GetFrameRateList(
					outputCapturePin, tmp, size, &listSize, &frameDurationList);

				if (hrVC == S_OK && listSize > 0 &&
					0 != (maxFPS = GetMaxOfFrameArray(frameDurationList, listSize))) {
					capability.maxFPS = static_cast<int>(10000000 / maxFPS);
					capability.supportFrameRateControl = true;
					CoTaskMemFree(frameDurationList);
				}
				else  // use existing method
				{
					if (avgTimePerFrame > 0)
						capability.maxFPS = static_cast<int>(10000000 / avgTimePerFrame);
					else
						capability.maxFPS = 0;
				}
			}
			else  // use existing method in case IAMVideoControl is not supported
			{
				if (avgTimePerFrame > 0)
					capability.maxFPS = static_cast<int>(10000000 / avgTimePerFrame);
				else
					capability.maxFPS = 0;
			}

			// can't switch MEDIATYPE :~(
			if (pmt->subtype == MEDIASUBTYPE_I420) {
				capability.videoType = VideoType::kI420;
			}
			else if (pmt->subtype == MEDIASUBTYPE_IYUV) {
				capability.videoType = VideoType::kIYUV;
			}
			else if (pmt->subtype == MEDIASUBTYPE_RGB24) {
				capability.videoType = VideoType::kRGB24;
			}
			else if (pmt->subtype == MEDIASUBTYPE_YUY2) {
				capability.videoType = VideoType::kYUY2;
			}
			else if (pmt->subtype == MEDIASUBTYPE_RGB565) {
				capability.videoType = VideoType::kRGB565;
			}
			else if (pmt->subtype == MEDIASUBTYPE_MJPG) {
				capability.videoType = VideoType::kMJPEG;
			}
			else if (pmt->subtype == MEDIASUBTYPE_dvsl ||
				pmt->subtype == MEDIASUBTYPE_dvsd ||
				pmt->subtype ==
				MEDIASUBTYPE_dvhd)  // If this is an external DV camera
			{
				capability.videoType =
					VideoType::kYUY2;  // MS DV filter seems to create this type
			}
			else if (pmt->subtype ==
				MEDIASUBTYPE_UYVY)  // Seen used by Declink capture cards
			{
				capability.videoType = VideoType::kUYVY;
			}

			else {
				WCHAR strGuid[39];
				StringFromGUID2(pmt->subtype, strGuid, 39);
				continue;
			}

			LOGI("%d x %d fps: %d  %s", capability.width, capability.height, capability.maxFPS, getTypeName(capability.videoType));
			_captureCapabilitiesWindows.push_back(capability);
		}
		DeleteMediaType(pmt);
		pmt = NULL;
	}
	
	return static_cast<int>(_captureCapabilitiesWindows.size());
}



bool CameraDirect::SetCameraOutput(
	const VideoConfig& vp) {

	VideoCaptureList capability;
	int capabilityIndex;

	// Store the new requested size
	VideoCaptureList _requestedCapability;

	_requestedCapability.height = vp.height;
	_requestedCapability.width = vp.width;
	_requestedCapability.maxFPS = vp.fps;
	if (vp.format == VF_RGB || vp.format == VF_BGR)
		_requestedCapability.videoType = VideoType::kRGB24;
	if (vp.format == VF_YUY2)
		_requestedCapability.videoType = VideoType::kYUY2;
	if (vp.format == VF_MJPEG)
		_requestedCapability.videoType = VideoType::kMJPEG;

	// Match the requested capability with the supported.
	if ((capabilityIndex = GetBestMatchedCapability(
		 _requestedCapability)) < 0) {
		return false;
	}

	
	VideoCaptureCapabilityWindows windowsCapability;
	if (GetWindowsCapability(capabilityIndex, windowsCapability) != 0) {
		return false;
	}
	
	
	VIDEO_STREAM_CONFIG_CAPS caps;


	CComPtr<IAMStreamConfig > streamConfig;
	HRESULT hr = m_pGrabberOutput->QueryInterface(IID_IAMStreamConfig, (void**)&streamConfig);
	AM_MEDIA_TYPE* pmt = NULL;
	if (hr) {
		return false;
	}

	// Get the windows capability from the capture device
	bool isDVCamera = false;
	hr = streamConfig->GetStreamCaps(windowsCapability.directShowCapabilityIndex,
		&pmt, reinterpret_cast<BYTE*>(&caps));
	if (!FAILED(hr)) {
		if (pmt->formattype == FORMAT_VideoInfo2) {
			VIDEOINFOHEADER2* h = reinterpret_cast<VIDEOINFOHEADER2*>(pmt->pbFormat);
			if (capability.maxFPS > 0 && windowsCapability.supportFrameRateControl) {
				h->AvgTimePerFrame = REFERENCE_TIME(10000000.0 / capability.maxFPS);
			}
		}
		else {
			VIDEOINFOHEADER* h = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
			if (capability.maxFPS > 0 && windowsCapability.supportFrameRateControl) {
				h->AvgTimePerFrame = REFERENCE_TIME(10000000.0 / capability.maxFPS);
			}
		}


		_sinkFilter->SetMatchingMediaType(windowsCapability);

		hr += streamConfig->SetFormat(pmt);

		VIDEOINFOHEADER *videoHeader;
		videoHeader = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
		m_nWidth = videoHeader->bmiHeader.biWidth;
		m_nHeight = videoHeader->bmiHeader.biHeight;

		if (!m_tempcache.empty())m_tempcache.release();
		

		if (pmt->subtype == MEDIASUBTYPE_YUY2){
			m_media_format = VF_YUY2;
			m_tempcache.create(m_nHeight, m_nWidth, CV_8UC2);
		}
		else if (pmt->subtype == MEDIASUBTYPE_RGB24)
		{
			m_media_format = VF_BGR;
			m_tempcache.create(m_nHeight, m_nWidth, CV_8UC3);
		}
		else if (pmt->subtype == MEDIASUBTYPE_MJPG)
		{
			m_media_format = VF_BGR;
			m_tempcache.create(m_nHeight, m_nWidth, CV_8UC3);
		}
		else
		{
			LOGE("not support type !!");
			return false;
		}//todo other subtype



		
		if (pmt->subtype == MEDIASUBTYPE_dvsl ||
			pmt->subtype == MEDIASUBTYPE_dvsd || pmt->subtype == MEDIASUBTYPE_dvhd)
			isDVCamera = true;  

		if (pmt->pbFormat)
			CoTaskMemFree(pmt->pbFormat);
		CoTaskMemFree(pmt);
	}

	if (FAILED(hr)) {
		
		return false;
	}


	hr = m_pGraph->ConnectDirect(m_pGrabberOutput, m_pGrabberInput, NULL);
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

bool CameraDirect::Open(VideoConfig &vp, bool bDisplayProperties /*= true*/)
{
	if (_open(vp, bDisplayProperties))return true;
	Close();
	LOGE("open camera %s error.",vp.device_name.c_str());
	return false;
}

bool CameraDirect::_open(VideoConfig &vp, bool bDisplayProperties)
{
	
	HRESULT hr = S_OK;
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if (!BindFilter(vp.device_name, vp.device_name_index, &m_pDeviceFilter))
		return false;

	if (CreateCapabilityMap() <= 0)return false;

	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&m_pGraph);
	if (hr != S_OK)return false;

	hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pMediaControl);
	if (hr != S_OK)return false;

	hr = m_pGraph->AddFilter(m_pDeviceFilter, L"VideoCaptureFilter");
	if (hr != S_OK)return false;

	
	m_pGrabberOutput = GetOutputPin(m_pDeviceFilter, PIN_CATEGORY_CAPTURE);
	if (m_pGrabberOutput == NULL)
		return false;

	
	_sinkFilter = new CaptureSinkFilter("SinkFilter", NULL, &hr, *this);
	if (hr != S_OK)return false;
	_sinkFilter->AddRef();

	hr = m_pGraph->AddFilter(_sinkFilter, L"SinkFilter");
	if (FAILED(hr)) {
		return false;
	}
	m_pGrabberInput = GetInputPin(_sinkFilter);
	if (m_pGrabberInput == NULL)
		return false;


	if (bDisplayProperties)
	{
		CComPtr<ISpecifyPropertyPages> pPages;

		CAUUID uuid;
		HRESULT hr = S_OK;

		hr = m_pDeviceFilter->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pPages);

		if (SUCCEEDED(hr))
		{
			CAUUID caGUID;
			pPages->GetPages(&caGUID);

			OleCreatePropertyFrame(NULL, 0, 0,
				L"Property Sheet", 1,
				(IUnknown **)&m_pDeviceFilter,
				caGUID.cElems, caGUID.pElems,
				LOCALE_USER_DEFAULT, 0, NULL);
			if (caGUID.pElems)
				CoTaskMemFree(caGUID.pElems);
		}
		else
			return false;
		pPages = NULL;
	}
	else
	{
		if (!SetCameraOutput(vp)) {
			return false;
		}
	}

	hr = m_pMediaControl->Run();
	if (FAILED(hr)) {
		return false;
	}
	m_bConnected = true;
	return true;
}

bool CameraDirect::SetParameter(CameraControlProperty key, int value, bool auto_or_manual)
{
	if (key<CameraControl_Pan || key>CameraControl_Focus)return false;
	CComPtr<IAMCameraControl> m_pCtrl;
	HRESULT  hr = m_pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&m_pCtrl);
	if (hr != S_OK)return false;

	long min = 0, max = 0, step = 1, defaultvalue = 0, flag;
	if (m_pCtrl->GetRange(key, &min, &max, &step, &defaultvalue, &flag) != S_OK)
	{
		return false;
	}
	if (auto_or_manual)
	{
		HRESULT  hr=m_pCtrl->Set(key, defaultvalue, CameraControl_Flags_Auto);
		if (hr != S_OK)return false;
		return true;
	}
	else
	{
		if (value != defaultvalue)
		{
			if (min <value&&max > value)
			{
				HRESULT  hr = m_pCtrl->Set(key, defaultvalue, CameraControl_Flags_Manual);
				if (hr != S_OK)return false;
				return true;
			}
			else
				return false;
		}
		return true;
	}
	return false;
}

bool CameraDirect::GetParameter(CameraControlProperty key, int &value, bool &auto_or_manual)
{
	if (key<CameraControl_Pan || key>CameraControl_Focus)return false;
	CComPtr<IAMCameraControl> m_pCtrl;
	HRESULT  hr = m_pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&m_pCtrl);
	if (hr != S_OK)return false;

	AM_MEDIA_TYPE *pmt;
	long min = 0, max = 0, step = 1, defaultvalue = 0, flag;
	hr = m_pCtrl->Get(key, &defaultvalue, &flag);
	if (hr != S_OK)return false;
	value = defaultvalue;
	auto_or_manual = (flag == CameraControl_Flags_Auto);
	return true;
}

bool CameraDirect::BindFilter(std::string camera_name, int camindex, IBaseFilter **pFilter)
{
	if (camindex < 0 || camera_name.empty())
	{
		return false;
	}

	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (hr != NOERROR)
	{
		return false;
	}

	CComPtr<IEnumMoniker> pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR)
	{
		return false;
	}

	

	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	int index = 0;
	bool found = false;
	while (!found&&(hr = pEm->Next(1, &pM, &cFetched)) == S_OK)
	{
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if (SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR)
			{
				char sName[4096] = { 0 };
				int nBufferSize = 4096;
				WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, sName, nBufferSize, "", NULL);
				//printf("BindToStorage:%s\n", sName);
				if (camera_name.compare(sName) == 0)
				{
					if (index == camindex)
					{
						hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
						if (SUCCEEDED(hr))
							found = true;
						else
							LOGE("cant select %s camera device", sName);
					}
					index++;
				}
				SysFreeString(var.bstrVal);
			//	VariantClear(&var);
			}
			pBag->Release();
		}
		pM->Release();
	}

	pCreateDevEnum = NULL;
	return found;
}


bool CameraDirect::QueryFrame(cv::Mat& img, VIDEO_FORMAT type)
{

	if (!m_bConnected)return false;

	if (img.cols != m_nWidth ||
		img.rows != m_nHeight)
	{
		img.create(m_nHeight, m_nWidth, CV_8UC3);
	}
	
	DWORD ret=WaitForSingleObject(m_cb.hEvent,5000);
	if (ret != WAIT_OBJECT_0)
		return false;

	{
		LOCK_HELPER_AUTO_HANDLER(m_cb.m_ml);
		if (type != m_media_format)
			memcpy(m_tempcache.data, m_cb.cache_image.data(), m_cb.cache_image.size());
		else
			memcpy(img.data, m_cb.cache_image.data(), m_cb.cache_image.size());
	}
	if (type != m_media_format)
	{
		if (m_media_format == VF_MJPEG&&type == VF_BGR)
		{
			img = cv::imdecode(m_tempcache, -1);
			if (img.empty())
				return false;
		}
		int codec = CV_BGR2RGB;
		if (m_media_format == VF_YUY2&&type == VF_BGR)
		{
			codec = CV_YUV2BGR_YUY2;
			cvtColor(m_tempcache, img, codec);
		}
		
		
	}
	if (m_media_format == VF_BGR)
		cv::flip(img, img, 0);
	//TODO
	return true;

}

int CameraDirect::onFrame(unsigned char* videoFrame, size_t videoFrameLength, const VideoCaptureList& frameInfo, long long captureTime /*= 0*/)
{
	LOCK_HELPER_AUTO_HANDLER(m_cb.m_ml);
	if (frameInfo.videoType == kYUY2)
		m_media_format = VF_YUY2;
	else if (frameInfo.videoType==kRGB24)
		m_media_format = VF_BGR;
	else if (frameInfo.videoType == kMJPEG)
		m_media_format = VF_MJPEG;
	else
	{
		printf("data not support!!\n");
		return 1; 
	}
	return m_cb.BufferCB(0, videoFrame, videoFrameLength);
}

#define CONVERTCAMERAPARAMETERFORMAT(videotype)  {																															\
		videotype *temp=(videotype*)type->pbFormat;										\
		BITMAPINFOHEADER *bih;															\
		_int64 *fr;																		\
		fr = &temp->AvgTimePerFrame;													\
		bih = &temp->bmiHeader;															\
		CameraCapablity infoccable;														\
		infoccable.max_width = vcaps->MaxOutputSize.cx;									\
		infoccable.max_height = vcaps->MaxOutputSize.cy;								\
		infoccable.min_width = vcaps->MinOutputSize.cx;									\
		infoccable.min_height = vcaps->MinOutputSize.cy;								\
		infoccable.min_fps = 1e7 / vcaps->MinFrameInterval;								\
		infoccable.max_fps = 1e7 / vcaps->MaxFrameInterval;								\
		infoccable.biCompression = bih->biCompression;									\
		infoccable.biBitCount = bih->biBitCount;										\
		infoccable.vcode = type->subtype;												\
		infoccable.formattype = type->formattype;										\
		cclist.back().capablity.push_back(infoccable);									\
}

void CameraDirect::PinList(IAMStreamConfig * config, std::vector<Camera> &cclist)
{
	int i, n, size, r;
	if (config->GetNumberOfCapabilities(&n, &size) == S_OK)
	{
		AM_MEDIA_TYPE *type = NULL;		
		void *caps = malloc(size);
		for (int i = 0; i < n; i++)
		{
			r = config->GetStreamCaps(i, &type, (BYTE *)caps);
			if (r != S_OK)
				continue;
			VIDEO_STREAM_CONFIG_CAPS *vcaps = (VIDEO_STREAM_CONFIG_CAPS *)caps;
			if (type->formattype == FORMAT_VideoInfo)
				CONVERTCAMERAPARAMETERFORMAT(VIDEOINFOHEADER)
			else if (type->formattype == FORMAT_VideoInfo2)
				CONVERTCAMERAPARAMETERFORMAT(VIDEOINFOHEADER2)
			if (type->pbFormat)
				CoTaskMemFree(type->pbFormat);
			CoTaskMemFree(type);
		}
		free(caps);
	}
}

int CameraDirect::CameraList(std::vector<Camera> &cclist)
{
	int count = 0;
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);

	CComPtr<IEnumMoniker> pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR) return 0;
	cclist.clear();
	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	int camera_index = 0;
	while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK)
	{
		CComPtr<IPropertyBag >pBag = 0;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if (SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL); //还有其他属性,像描述信息等等...
/*			hr = pBag->Read(L"DevicePath", &var, NULL); //TODO 目前先这样感觉顺序不会改变
														，未来添加根据设备路径找到物理路径PCIROOT(0)#PCI(1400)#USBROOT(0)#USB(1)#USB(1)
														 具体可以参考usb设备遍历过程SetupDiGetDeviceRegistryProperty，再绑定逻辑id*/
			if (hr == NOERROR)
			{
				char sName[4096] = { 0 };
				int nBufferSize = 4096;
				WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, sName, nBufferSize, "", NULL);
				SysFreeString(var.bstrVal);
			//	VariantClear(&var);
				CComPtr<IBaseFilter>pVCap;
				if (SUCCEEDED(pM->BindToObject(0, 0, IID_IBaseFilter, (void**)&pVCap)) && pVCap != NULL)
				{
					CComPtr<IEnumPins >pins = 0;
					if (SUCCEEDED(pVCap->EnumPins(&pins)))
					{
						bool Found = false;
						IPin *pP = 0;
						PIN_INFO     pinInfo;
						ULONG        n;
						pins->Reset();
						while (!Found && (S_OK == pins->Next(1, &pP, &n)))
						{
							if (S_OK == pP->QueryPinInfo(&pinInfo))
							{
								if (pinInfo.dir == PINDIR_OUTPUT)
								{
									CComPtr<IKsPropertySet>pKs = 0;
									if (pP->QueryInterface(IID_IKsPropertySet,
										(void **)&pKs) == S_OK)
									{
										GUID guid;
										DWORD dw;
										if (pKs->Get(AMPROPSETID_Pin,
											AMPROPERTY_PIN_CATEGORY, NULL, 0,
											&guid, sizeof(GUID), &dw) == S_OK)
										{
											if (guid == PIN_CATEGORY_CAPTURE)
											{
												CComPtr<IAMStreamConfig >config = NULL;
												if (pP->QueryInterface(IID_IAMStreamConfig, (void **)&config) == S_OK)
												{
													Camera  onec;
													onec.index = camera_index++;
													onec.name = sName;
													cclist.push_back(onec);
													PinList(config, cclist);
												}
													
											}
										}
									}
								}
								pinInfo.pFilter->Release();
							}
							pP->Release();
						}
					}
				}

			}
		}
		pM->Release();
	}

	pCreateDevEnum = NULL;
	pEm = NULL;

	return 1;
}

int CameraDirect::GetOptimization(Camera input, int setwidth, int setheight, int fps)
{
	int optindex = -1;
	setwidth = abs(setwidth);
	setheight = abs(setheight);
	fps = abs(fps);
	std::vector<int> firFilterIndex;
	int area = setwidth*setheight;
	int min_a=1920*1080, max_a=0, max_f=0, min_f=1024;
	for (int i = 0; i < input.capablity.size(); i++)
	{
		if ((input.capablity[i].min_height*input.capablity[i].min_width)>max_a)
			max_a = input.capablity[i].min_height*input.capablity[i].min_width;
		if ((input.capablity[i].min_height*input.capablity[i].min_width) < min_a)
			min_a = input.capablity[i].min_height*input.capablity[i].min_width;
		if ((input.capablity[i].min_fps) < min_f)
			min_f = input.capablity[i].min_fps;
		if ((input.capablity[i].min_fps) > max_f)
			max_f = input.capablity[i].min_fps;
	}
	if (area != 0 && area < min_a)area = min_a;
	if (area != 0 && area > max_a)area = max_a;
	if (fps != 0 && fps < min_f)fps = min_f;
	if (fps != 0 && fps > max_f)fps = max_f;
	if (area == 0&&fps==0)
	{
		for (int i = 0; i < input.capablity.size(); i++)
		{
			CameraCapablity tmp = input.capablity[i];
			if (tmp.min_fps>24)
				firFilterIndex.push_back(i);
		}
		if (firFilterIndex.size())
			optindex = firFilterIndex[0];
		for (int i = 0; i < firFilterIndex.size();i++)
		{
			CameraCapablity tmp = input.capablity[firFilterIndex[i]];
			if ((tmp.min_width*tmp.min_height)>(input.capablity[optindex].min_width*input.capablity[optindex].min_height)||
				((tmp.min_width*tmp.min_height) == (input.capablity[optindex].min_width*input.capablity[optindex].min_height) && 
				tmp.min_fps>input.capablity[optindex].min_fps))
			{
				optindex = firFilterIndex[i];
			}
		}
	}

	if (((area != 0 && fps != 0)) || (area != 0 && fps == 0))
	{
		unsigned int maxvalue = ~0;
		for (int i = 0; i < input.capablity.size(); i++)
		{
			CameraCapablity tmp = input.capablity[i];
			if ((tmp.min_width*tmp.min_height - area)>=0 && ((tmp.min_width*tmp.min_height - area) <= (maxvalue - area)))
				maxvalue = tmp.min_width*tmp.min_height;
		}
		for (int i = 0; i < input.capablity.size(); i++)
		{
			CameraCapablity tmp = input.capablity[i];
			if ((tmp.min_width*tmp.min_height)==(maxvalue))
				firFilterIndex.push_back(i);
		}
		if (firFilterIndex.size())
			optindex = firFilterIndex[0];;

		for (int i = 0; i < firFilterIndex.size(); i++)
		{
			CameraCapablity tmp = input.capablity[firFilterIndex[i]];
			if (fps != 0)
			{
				if ((tmp.min_fps - fps)>=0 && (tmp.min_fps - fps) <= (input.capablity[optindex].min_fps - fps))
					optindex = firFilterIndex[i];

			}
			else
			{
				if (tmp.min_fps > input.capablity[optindex].min_fps)
					optindex = firFilterIndex[i];
			}
		}
	}
	if (area == 0 && fps != 0)
	{
		unsigned int maxvalue = ~0;
		for (int i = 0; i < input.capablity.size(); i++)
		{
			CameraCapablity tmp = input.capablity[i];
			if ((tmp.min_fps - fps)>=0 && ((tmp.min_fps - fps) <= (maxvalue - fps)))
				maxvalue = tmp.min_fps;
		}
		for (int i = 0; i < input.capablity.size(); i++)
		{
			CameraCapablity tmp = input.capablity[i];
			if ((tmp.min_fps) == (maxvalue))
				firFilterIndex.push_back(i);
		}
		if (firFilterIndex.size())
			optindex = firFilterIndex[0];;

		for (int i = 0; i < firFilterIndex.size(); i++)
		{
			CameraCapablity tmp = input.capablity[firFilterIndex[i]];
			if ((tmp.min_width*tmp.min_height)>(input.capablity[optindex].min_width*input.capablity[optindex].min_height))
				optindex = firFilterIndex[i];
		}
	}

	for (int i = 0; i < input.capablity.size(); i++)
	{
		if (i == optindex)continue;
		CameraCapablity tmp = input.capablity[i];
		if (tmp.max_fps == input.capablity[optindex].max_fps&&
			tmp.max_height == input.capablity[optindex].max_height&&
			tmp.max_width == input.capablity[optindex].max_width&&
			tmp.min_fps == input.capablity[optindex].min_fps&&
			tmp.min_height == input.capablity[optindex].min_height&&
			tmp.min_width == input.capablity[optindex].min_width&&
			tmp.vcode != MEDIASUBTYPE_YUY2)
		{
			optindex = i;
			break;
		}
	}
	return optindex;
}

