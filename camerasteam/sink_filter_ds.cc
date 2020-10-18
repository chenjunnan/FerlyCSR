
#include <C:\Program Files\Windows Kits\8.0\Include\um\strmif.h>
#include "sink_filter_ds.h"

#undef  __IReferenceClockTimerControl_FWD_DEFINED__
#include <dvdmedia.h>  // VIDEOINFOHEADER2
#include <initguid.h>
#include <stdio.h>



#define DELETE_RESET(p) \
  {                     \
    delete (p);         \
    (p) = NULL;         \
  }

DEFINE_GUID(CLSID_SINKFILTER,
            0x88cdbbdc,
            0xa73b,
            0x4afa,
            0xac,
            0xbf,
            0x15,
            0xd5,
            0xe2,
            0xce,
            0x12,
            0xc3);


typedef struct tagTHREADNAME_INFO {
  DWORD dwType;      // must be 0x1000
  LPCSTR szName;     // pointer to name (in user addr space)
  DWORD dwThreadID;  // thread ID (-1=caller thread)
  DWORD dwFlags;     // reserved for future use, must be zero
} THREADNAME_INFO;

CaptureInputPin::CaptureInputPin(IN TCHAR* szName,
                                 IN CaptureSinkFilter* pFilter,
                                 IN CCritSec* pLock,
                                 OUT HRESULT* pHr,
                                 IN LPCWSTR pszName)
    : CBaseInputPin(szName, pFilter, pLock, pHr, pszName),
      _requestedCapability(),
      _resultingCapability() {
  _threadHandle = NULL;
}

CaptureInputPin::~CaptureInputPin() {}

HRESULT
CaptureInputPin::GetMediaType(IN int iPosition, OUT CMediaType* pmt) {
  // reset the thread handle
  _threadHandle = NULL;

  if (iPosition < 0)
    return E_INVALIDARG;

  VIDEOINFOHEADER* pvi =
      (VIDEOINFOHEADER*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
  if (NULL == pvi) {
    return (E_OUTOFMEMORY);
  }

  ZeroMemory(pvi, sizeof(VIDEOINFOHEADER));
  pvi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  pvi->bmiHeader.biPlanes = 1;
  pvi->bmiHeader.biClrImportant = 0;
  pvi->bmiHeader.biClrUsed = 0;
  if (_requestedCapability.maxFPS != 0) {
    pvi->AvgTimePerFrame = 10000000 / _requestedCapability.maxFPS;
  }

  SetRectEmpty(&(pvi->rcSource));  // we want the whole image area rendered.
  SetRectEmpty(&(pvi->rcTarget));  // no particular destination rectangle

  pmt->SetType(&MEDIATYPE_Video);
  pmt->SetFormatType(&FORMAT_VideoInfo);
  pmt->SetTemporalCompression(FALSE);

  int positionOffset = 1;
  switch (iPosition + positionOffset) {
    case 0: {
		printf("error !!!!!\n");
		return (E_HANDLE);
    } break;
    case 1: {
      pvi->bmiHeader.biCompression = MAKEFOURCC('Y', 'U', 'Y', '2');
      ;
      pvi->bmiHeader.biBitCount = 16;  // bit per pixel
      pvi->bmiHeader.biWidth = _requestedCapability.width;
      pvi->bmiHeader.biHeight = _requestedCapability.height;
      pvi->bmiHeader.biSizeImage =
          2 * _requestedCapability.width * _requestedCapability.height;
      pmt->SetSubtype(&MEDIASUBTYPE_YUY2);
    } break;
    case 2: {
      pvi->bmiHeader.biCompression = BI_RGB;
      pvi->bmiHeader.biBitCount = 24;  // bit per pixel
      pvi->bmiHeader.biWidth = _requestedCapability.width;
      pvi->bmiHeader.biHeight = _requestedCapability.height;
      pvi->bmiHeader.biSizeImage =
          3 * _requestedCapability.height * _requestedCapability.width;
      pmt->SetSubtype(&MEDIASUBTYPE_RGB24);
    } break;
    case 3: {
      pvi->bmiHeader.biCompression = MAKEFOURCC('U', 'Y', 'V', 'Y');
      pvi->bmiHeader.biBitCount = 16;  // bit per pixel
      pvi->bmiHeader.biWidth = _requestedCapability.width;
      pvi->bmiHeader.biHeight = _requestedCapability.height;
      pvi->bmiHeader.biSizeImage =
          2 * _requestedCapability.height * _requestedCapability.width;
      pmt->SetSubtype(&MEDIASUBTYPE_UYVY);
    } break;
    case 4: {
      pvi->bmiHeader.biCompression = MAKEFOURCC('M', 'J', 'P', 'G');
      pvi->bmiHeader.biBitCount = 12;  // bit per pixel
      pvi->bmiHeader.biWidth = _requestedCapability.width;
      pvi->bmiHeader.biHeight = _requestedCapability.height;
      pvi->bmiHeader.biSizeImage =
          3 * _requestedCapability.height * _requestedCapability.width / 2;
      pmt->SetSubtype(&MEDIASUBTYPE_MJPG);
    } break;
    default:
      return VFW_S_NO_MORE_ITEMS;
  }
  pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);
  return NOERROR;
}

HRESULT
CaptureInputPin::CheckMediaType(IN const CMediaType* pMediaType) {

  _threadHandle = NULL;

  const GUID* type = pMediaType->Type();
  if (*type != MEDIATYPE_Video)
    return E_INVALIDARG;

  const GUID* formatType = pMediaType->FormatType();

 
  const GUID* SubType = pMediaType->Subtype();
  if (SubType == NULL) {
    return E_INVALIDARG;
  }

  if (*formatType == FORMAT_VideoInfo) {
    VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*)pMediaType->Format();
    if (pvi == NULL) {
      return E_INVALIDARG;
    }


    _resultingCapability.width = pvi->bmiHeader.biWidth;


    if (*SubType == MEDIASUBTYPE_RGB24 && pvi->bmiHeader.biHeight > 0) {
      _resultingCapability.height = -(pvi->bmiHeader.biHeight);
    } else {
      _resultingCapability.height = abs(pvi->bmiHeader.biHeight);
    }



    if (*SubType == MEDIASUBTYPE_MJPG &&
        pvi->bmiHeader.biCompression == MAKEFOURCC('M', 'J', 'P', 'G')) {
      _resultingCapability.videoType = VideoType::kMJPEG;
      return S_OK;  // This format is acceptable.
    }
    if (*SubType == MEDIASUBTYPE_I420 &&
        pvi->bmiHeader.biCompression == MAKEFOURCC('I', '4', '2', '0')) {
      _resultingCapability.videoType = VideoType::kI420;
      return S_OK;  // This format is acceptable.
    }
    if (*SubType == MEDIASUBTYPE_YUY2 &&
        pvi->bmiHeader.biCompression == MAKEFOURCC('Y', 'U', 'Y', '2')) {
      _resultingCapability.videoType = VideoType::kYUY2;
      ::Sleep(60);  // workaround for bad driver
      return S_OK;  // This format is acceptable.
    }
    if (*SubType == MEDIASUBTYPE_UYVY &&
        pvi->bmiHeader.biCompression == MAKEFOURCC('U', 'Y', 'V', 'Y')) {
      _resultingCapability.videoType = VideoType::kUYVY;
      return S_OK;  // This format is acceptable.
    }

    if (*SubType == MEDIASUBTYPE_RGB24 &&
        pvi->bmiHeader.biCompression == BI_RGB) {
      _resultingCapability.videoType = VideoType::kRGB24;
      return S_OK;  // This format is acceptable.
    }
  }
  if (*formatType == FORMAT_VideoInfo2) {
    // VIDEOINFOHEADER2 that has dwInterlaceFlags
    VIDEOINFOHEADER2* pvi = (VIDEOINFOHEADER2*)pMediaType->Format();

    if (pvi == NULL) {
      return E_INVALIDARG;
    }


    _resultingCapability.width = pvi->bmiHeader.biWidth;

    // Store the incoming height,
    // for RGB24 we assume the frame to be upside down
    if (*SubType == MEDIASUBTYPE_RGB24 && pvi->bmiHeader.biHeight > 0) {
      _resultingCapability.height = -(pvi->bmiHeader.biHeight);
    } else {
      _resultingCapability.height = abs(pvi->bmiHeader.biHeight);
    }

    if (*SubType == MEDIASUBTYPE_MJPG &&
        pvi->bmiHeader.biCompression == MAKEFOURCC('M', 'J', 'P', 'G')) {
      _resultingCapability.videoType = VideoType::kMJPEG;
      return S_OK;  // This format is acceptable.
    }
    if (*SubType == MEDIASUBTYPE_I420 &&
        pvi->bmiHeader.biCompression == MAKEFOURCC('I', '4', '2', '0')) {
      _resultingCapability.videoType = VideoType::kI420;
      return S_OK;  // This format is acceptable.
    }
    if (*SubType == MEDIASUBTYPE_YUY2 &&
        pvi->bmiHeader.biCompression == MAKEFOURCC('Y', 'U', 'Y', '2')) {
      _resultingCapability.videoType = VideoType::kYUY2;
      return S_OK;  // This format is acceptable.
    }
    if (*SubType == MEDIASUBTYPE_UYVY &&
        pvi->bmiHeader.biCompression == MAKEFOURCC('U', 'Y', 'V', 'Y')) {
      _resultingCapability.videoType = VideoType::kUYVY;
      return S_OK;  // This format is acceptable.
    }

    if (*SubType == MEDIASUBTYPE_RGB24 &&
        pvi->bmiHeader.biCompression == BI_RGB) {
      _resultingCapability.videoType = VideoType::kRGB24;
      return S_OK;  // This format is acceptable.
    }
  }
  return E_INVALIDARG;
}

HRESULT
CaptureInputPin::Receive(IN IMediaSample* pIMediaSample) {
  HRESULT hr = S_OK;

  if (m_pFilter == NULL) return S_FALSE;
  if (pIMediaSample == NULL)return S_FALSE;


  reinterpret_cast<CaptureSinkFilter*>(m_pFilter)->LockReceive();
  hr = CBaseInputPin::Receive(pIMediaSample);

  if (SUCCEEDED(hr)) {
    const LONG length = pIMediaSample->GetActualDataLength();
    if(length < 0)return S_FALSE;

    unsigned char* pBuffer = NULL;
    if (S_OK != pIMediaSample->GetPointer(&pBuffer)) {
      reinterpret_cast<CaptureSinkFilter*>(m_pFilter)->UnlockReceive();
      return S_FALSE;
    }

    // NOTE: filter unlocked within Send call
    reinterpret_cast<CaptureSinkFilter*>(m_pFilter)->ProcessCapturedFrame(
        pBuffer, static_cast<size_t>(length), _resultingCapability);
  } else {
    reinterpret_cast<CaptureSinkFilter*>(m_pFilter)->UnlockReceive();
  }

  return hr;
}


HRESULT CaptureInputPin::SetMatchingMediaType(
    const VideoCaptureList& capability) {
  _requestedCapability = capability;
  _resultingCapability = VideoCaptureList();
  return S_OK;
}
//  ----------------------------------------------------------------------------
CaptureSinkFilter::CaptureSinkFilter(const IN TCHAR* tszName,
                                     IN LPUNKNOWN punk,
                                     OUT HRESULT* phr,
                                     VideoCaptureInterface& captureObserver)
    : CBaseFilter(tszName, punk, &m_crtFilter, CLSID_SINKFILTER),
      m_pInput(NULL),
      _captureObserver(captureObserver) {
  (*phr) = S_OK;
  TCHAR inputPinName[] = "VideoCaptureInputPin";
  m_pInput = new CaptureInputPin(inputPinName, this, &m_crtFilter, phr,
                                 L"VideoCapture");
  if (m_pInput == NULL || FAILED(*phr)) {
    (*phr) = FAILED(*phr) ? (*phr) : E_OUTOFMEMORY;
    goto cleanup;
  }
cleanup:
  return;
}

CaptureSinkFilter::~CaptureSinkFilter() {
  delete m_pInput;
}

int CaptureSinkFilter::GetPinCount() {
  return 1;
}

CBasePin* CaptureSinkFilter::GetPin(IN int Index) {
  CBasePin* pPin;
  LockFilter();
  if (Index == 0) {
    pPin = m_pInput;
  } else {
    pPin = NULL;
  }
  UnlockFilter();
  return pPin;
}

STDMETHODIMP CaptureSinkFilter::Pause() {
  LockReceive();
  LockFilter();
  if (m_State == State_Stopped) {
    //  change the state, THEN activate the input pin
    m_State = State_Paused;
    if (m_pInput && m_pInput->IsConnected()) {
      m_pInput->Active();
    }
    if (m_pInput && !m_pInput->IsConnected()) {
      m_State = State_Running;
    }
  } else if (m_State == State_Running) {
    m_State = State_Paused;
  }
  UnlockFilter();
  UnlockReceive();
  return S_OK;
}

STDMETHODIMP CaptureSinkFilter::Stop() {
  LockReceive();
  LockFilter();

  //  set the state
  m_State = State_Stopped;

  //  inactivate the pins
  if (m_pInput)
    m_pInput->Inactive();

  UnlockFilter();
  UnlockReceive();
  return S_OK;
}

void CaptureSinkFilter::SetFilterGraph(IGraphBuilder* graph) {
  LockFilter();
  m_pGraph = graph;
  UnlockFilter();
}

void CaptureSinkFilter::ProcessCapturedFrame(
    unsigned char* pBuffer,
    size_t length,
    const VideoCaptureList& frameInfo) {
  if (m_State == State_Running) {
    _captureObserver.onFrame(pBuffer, length, frameInfo);

    UnlockReceive();
    return;
  }
  UnlockReceive();
  return;
}

STDMETHODIMP CaptureSinkFilter::SetMatchingMediaType(
    const VideoCaptureList& capability) {
  LockReceive();
  LockFilter();
  HRESULT hr;
  if (m_pInput) {
    hr = m_pInput->SetMatchingMediaType(capability);
  } else {
    hr = E_UNEXPECTED;
  }
  UnlockFilter();
  UnlockReceive();
  return hr;
}

STDMETHODIMP CaptureSinkFilter::GetClassID(OUT CLSID* pCLSID) {
  (*pCLSID) = CLSID_SINKFILTER;
  return S_OK;
}
