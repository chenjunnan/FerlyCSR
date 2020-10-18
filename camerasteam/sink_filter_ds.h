
#ifndef MODULES_VIDEO_DS_H_
#define MODULES_VIDEO_DS_H_

#include <streams.h>  
#include "common_camera.h"

class CaptureSinkFilter;

class CaptureInputPin : public CBaseInputPin {
 public:
  VideoCaptureList _requestedCapability;
  VideoCaptureList _resultingCapability;
  HANDLE _threadHandle;

  CaptureInputPin(IN TCHAR* szName,
                  IN CaptureSinkFilter* pFilter,
                  IN CCritSec* pLock,
                  OUT HRESULT* pHr,
                  IN LPCWSTR pszName);
  virtual ~CaptureInputPin();

  HRESULT GetMediaType(IN int iPos, OUT CMediaType* pmt);
  HRESULT CheckMediaType(IN const CMediaType* pmt);
  STDMETHODIMP Receive(IN IMediaSample*);
  HRESULT SetMatchingMediaType(const VideoCaptureList& capability);
};

class CaptureSinkFilter : public CBaseFilter {
 public:
  CaptureSinkFilter(const IN TCHAR* tszName,
                    IN LPUNKNOWN punk,
                    OUT HRESULT* phr,
                    VideoCaptureInterface& captureObserver);
  virtual ~CaptureSinkFilter();

  void ProcessCapturedFrame(unsigned char* pBuffer,
                            size_t length,
                            const VideoCaptureList& frameInfo);
  //  explicit receiver lock aquisition and release
  void LockReceive() { m_crtRecv.Lock(); }
  void UnlockReceive() { m_crtRecv.Unlock(); }
  //  explicit filter lock aquisition and release
  void LockFilter() { m_crtFilter.Lock(); }
  void UnlockFilter() { m_crtFilter.Unlock(); }
  void SetFilterGraph(IGraphBuilder* graph);  // Used if EVR

  //  --------------------------------------------------------------------
  //  COM interfaces
  DECLARE_IUNKNOWN;
  STDMETHODIMP SetMatchingMediaType(const VideoCaptureList& capability);

  //  --------------------------------------------------------------------
  //  CBaseFilter methods
  int GetPinCount();
  CBasePin* GetPin(IN int Index);
  STDMETHODIMP Pause();
  STDMETHODIMP Stop();
  STDMETHODIMP GetClassID(OUT CLSID* pCLSID);
  //  --------------------------------------------------------------------
  //  class factory calls this
  static CUnknown* CreateInstance(IN LPUNKNOWN punk, OUT HRESULT* phr);

 private:
  CCritSec m_crtFilter;  //  filter lock
  CCritSec m_crtRecv;    //  receiver lock; always acquire before filter lock
  CaptureInputPin* m_pInput;
  VideoCaptureInterface& _captureObserver;
};

#endif  
