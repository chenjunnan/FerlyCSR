#include "audio_stream_capture_direct.h"
#include <mmsystem.h>
#include <dsound.h>
#include "xCommon.h"
#include "..\xCommon\common_helper.h"

#ifndef MAX
#define MAX(a,b)        ( (a) > (b) ? (a) : (b) )
#endif

DEFINE_GUID(_IID_IDirectSoundNotify, 0xb0210783, 0x89cd, 0x11d0, 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16);

#pragma comment(lib, "Dsound.lib") 
audio_stream_capture_direct::audio_stream_capture_direct(void)
{
	m_pCapDev = NULL;
	m_pCapBuf = NULL;
	m_pNotify = NULL;
	// set default wave format PCM
	ZeroMemory(&m_wfxInput, sizeof(m_wfxInput));
	m_wfxInput=DEFAULT_FORMAT;
	m_guidCapDevId = GUID_NULL;
	m_bRecording = FALSE;
	m_hNotifyEvent = NULL;
}

bool audio_stream_capture_direct::init(AUDIO_INFO &config)
{
	m_wfxInput.nSamplesPerSec = config.nSampleRate;
	m_wfxInput.nAvgBytesPerSec = m_wfxInput.nBlockAlign * m_wfxInput.nSamplesPerSec;
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
	{
		LOGE("Audio CoInitialize Failed!\r\n");
		return FALSE;
	}
	return true;
}

bool audio_stream_capture_direct::start()
{
	Open();
	return true;
}

void audio_stream_capture_direct::stop()
{

}

void audio_stream_capture_direct::pushmedia(char *buffer, int len)
{

}

audio_stream_capture_direct::~audio_stream_capture_direct(void)
{
	CoUninitialize();
}

BOOL CALLBACK audio_stream_capture_direct::enum_dev_proc(LPGUID lpGUID, LPCTSTR lpszDesc,
	LPCTSTR lpszDrvName, LPVOID lpContext)
{
	HWND hList = (HWND)lpContext;
	if (!hList) return FALSE;
	LPGUID lpTemp = NULL;
	if (lpGUID != NULL) {
		// NULL only for "Primary Sound Driver".
		if ((lpTemp = (LPGUID)malloc(sizeof(GUID))) == NULL) return(TRUE);
		memcpy(lpTemp, lpGUID, sizeof(GUID));
	}
	::SendMessage(hList, CB_ADDSTRING, 0, (LPARAM)lpszDesc);
	::SendMessage(hList, LB_SETITEMDATA, 0, (LPARAM)lpTemp);
	free(lpTemp);
	return(TRUE);
}

UINT audio_stream_capture_direct::notify_capture_thd(LPVOID data)
{
	audio_stream_capture_direct * pado = static_cast<audio_stream_capture_direct *>(data);
	MSG   msg;
	HRESULT hr;
	DWORD dwResult;
	while (pado->m_bRecording) {
		dwResult = MsgWaitForMultipleObjects(1, &(pado->m_hNotifyEvent), FALSE, INFINITE, QS_ALLEVENTS);
		switch (dwResult) {
		case WAIT_OBJECT_0 + 0:
			// g_hNotificationEvents[0] is signaled
			// This means that DirectSound just finished playing 
			// a piece of the buffer, so we need to fill the circular 
			// buffer with new sound from the wav file
			if (FAILED(hr = pado->RecordCapturedData())) {
				LOGE("Error handling DirectSound notifications.");
				pado->m_bRecording = FALSE;
			}
			break;
		case WAIT_OBJECT_0 + 1:
			// Windows messages are available
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT) pado->m_bRecording = FALSE;
			}
			break;
		}
	}
	return 0;
}

BOOL audio_stream_capture_direct::EnumDevices(HWND hList)
{
	if (FAILED(DirectSoundCaptureEnumerate(
		(LPDSENUMCALLBACK)(audio_stream_capture_direct::enum_dev_proc),
		(VOID*)&hList)))
	{
		return(FALSE);
	}
	return (TRUE);
}

BOOL audio_stream_capture_direct::Open(void)
{
	HRESULT hr;
	if (!m_bRecording) {
		hr = InitDirectSound();
	}
	return (FAILED(hr)) ? FALSE : TRUE;
}
BOOL audio_stream_capture_direct::Close()
{
	HRESULT hr;
	hr = FreeDirectSound();
	CloseHandle(m_hNotifyEvent);
	return (FAILED(hr)) ? FALSE : TRUE;
}
HRESULT audio_stream_capture_direct::InitDirectSound(GUID dev_id)
{
	HRESULT hr;
	m_guidCapDevId = dev_id;
	ZeroMemory(&m_aPosNotify, sizeof(DSBPOSITIONNOTIFY) * (NUM_REC_NOTIFICATIONS + 1));
	m_dwCapBufSize = 0;
	m_dwNotifySize = 0;
	// Create IDirectSoundCapture using the preferred capture device
	hr = DirectSoundCaptureCreate8(&m_guidCapDevId, &m_pCapDev, NULL);
	// init wave format 
	SetWavFormat(&m_wfxInput);
	return (FAILED(hr)) ? S_FALSE : S_OK;
}
HRESULT audio_stream_capture_direct::FreeDirectSound()
{
	// Release DirectSound interfaces
	SAFE_RELEASE(m_pNotify);
	SAFE_RELEASE(m_pCapBuf);
	SAFE_RELEASE(m_pCapDev);
	return S_OK;
}
HRESULT audio_stream_capture_direct::CreateCaptureBuffer(WAVEFORMATEX * wfx)
{
	HRESULT hr;
	DSCBUFFERDESC dscbd;
	SAFE_RELEASE(m_pNotify);
	SAFE_RELEASE(m_pCapBuf);
	// Set the notification size
	m_dwNotifySize = MAX(1024, wfx->nAvgBytesPerSec / 8);
	m_dwNotifySize -= m_dwNotifySize % wfx->nBlockAlign;
	// Set the buffer sizes 
	m_dwCapBufSize = m_dwNotifySize * NUM_REC_NOTIFICATIONS;

	// Create the capture buffer
	ZeroMemory(&dscbd, sizeof(dscbd));
	dscbd.dwSize = sizeof(dscbd);
	dscbd.dwBufferBytes = m_dwCapBufSize;
	dscbd.lpwfxFormat = wfx; // Set the format during creatation
	if (FAILED(hr = m_pCapDev->CreateCaptureBuffer(&dscbd, &m_pCapBuf, NULL)))
		return S_FALSE;
	m_dwNextCapOffset = 0;
	if (FAILED(hr = InitNotifications()))
		return S_FALSE;
	return S_OK;
}
HRESULT audio_stream_capture_direct::InitNotifications()
{
	HRESULT hr;
	int i;
	if (NULL == m_pCapBuf)
		return S_FALSE;
	// create auto notify event 
	m_hNotifyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// Create a notification event, for when the sound stops playing
	if (FAILED(hr = m_pCapBuf->QueryInterface(IID_IDirectSoundNotify, (VOID**)&m_pNotify)))
		return S_FALSE;
	// Setup the notification positions
	for (i = 0; i < NUM_REC_NOTIFICATIONS; i++) {
		m_aPosNotify[i].dwOffset = (m_dwNotifySize * i) + m_dwNotifySize - 1;
		m_aPosNotify[i].hEventNotify = m_hNotifyEvent;
	}
	// Tell DirectSound when to notify us. the notification will come in the from 
	// of signaled events that are handled in WinMain()
	if (FAILED(hr = m_pNotify->SetNotificationPositions(NUM_REC_NOTIFICATIONS, m_aPosNotify)))
		return S_FALSE;
	return S_OK;
}
HRESULT audio_stream_capture_direct::StartOrStopRecord(BOOL bStartRec)
{
	HRESULT hr;
	if (bStartRec) {
		// Create a capture buffer, and tell the capture 
		// buffer to start recording   
		if (FAILED(hr = CreateCaptureBuffer(&m_wfxInput)))
			return S_FALSE;
		if (FAILED(hr = m_pCapBuf->Start(DSCBSTART_LOOPING)))
			return S_FALSE;
		// create notify event recv thread 
		//AfxBeginThread(audio_stream_capture_direct::notify_capture_thd, (LPVOID)(this));
	}
	else {
		// Stop the capture and read any data that 
		// was not caught by a notification
		if (NULL == m_pCapBuf)
			return S_OK;
		// wait until the notify_event_thd thread exit and release the resources.
	/*	Sleep(500);*/
		// Stop the buffer, and read any data that was not 
		// caught by a notification
		if (FAILED(hr = m_pCapBuf->Stop()))
			return S_OK;
		if (FAILED(hr = RecordCapturedData()))
			return S_FALSE;
	}
	return S_OK;
}
HRESULT audio_stream_capture_direct::RecordCapturedData()
{
	HRESULT hr;
	VOID*   pbCaptureData = NULL;
	DWORD   dwCaptureLength;
	VOID*   pbCaptureData2 = NULL;
	DWORD   dwCaptureLength2;
	DWORD   dwReadPos;
	DWORD   dwCapturePos;
	LONG lLockSize;
	if (NULL == m_pCapBuf)
		return S_FALSE;

	if (FAILED(hr = m_pCapBuf->GetCurrentPosition(&dwCapturePos, &dwReadPos)))
		return S_FALSE;
	lLockSize = dwReadPos - m_dwNextCapOffset;
	if (lLockSize < 0)
		lLockSize += m_dwCapBufSize;
	// Block align lock size so that we are always write on a boundary
	lLockSize -= (lLockSize % m_dwNotifySize);
	if (lLockSize == 0)
		return S_FALSE;
	// Lock the capture buffer down
	if (FAILED(hr = m_pCapBuf->Lock(m_dwNextCapOffset, lLockSize,
		&pbCaptureData, &dwCaptureLength,
		&pbCaptureData2, &dwCaptureLength2, 0L)))
		return S_FALSE;
	// call the outer data handler
	if (m_frame_handler) {
		m_frame_handler->AdoFrameData((BYTE*)pbCaptureData, dwCaptureLength);
	}

	// Move the capture offset along
	m_dwNextCapOffset += dwCaptureLength;
	m_dwNextCapOffset %= m_dwCapBufSize; // Circular buffer
	if (pbCaptureData2 != NULL) {
		// call the outer data handler 
		if (m_frame_handler) {
			m_frame_handler->AdoFrameData((BYTE*)pbCaptureData, dwCaptureLength);
		}
		// Move the capture offset along
		m_dwNextCapOffset += dwCaptureLength2;
		m_dwNextCapOffset %= m_dwCapBufSize; // Circular buffer
	}
	// Unlock the capture buffer
	m_pCapBuf->Unlock(pbCaptureData, dwCaptureLength, pbCaptureData2, dwCaptureLength2);
	return S_OK;
}
void audio_stream_capture_direct::SetWavFormat(WAVEFORMATEX * wfx)
{
// 	// get the default capture wave formate 
// 	ZeroMemory(wfx, sizeof(WAVEFORMATEX));
// 	wfx->wFormatTag = WAVE_FORMAT_PCM;
// 	// 8KHz, 16 bits PCM, Mono
// 	//wfx->nSamplesPerSec = 8000;
// 	wfx->wBitsPerSample = 16;
// 	wfx->nChannels = 1;
// 	wfx->nBlockAlign = wfx->nChannels * (wfx->wBitsPerSample / 8);
// 	wfx->nAvgBytesPerSec = wfx->nBlockAlign * wfx->nSamplesPerSec;
}

void audio_stream_capture_direct::GrabAudioFrames(BOOL bGrabAudioFrames, CAdoFrameHandler* frame_handler)
{
	m_frame_handler = frame_handler;
	m_bRecording = bGrabAudioFrames;
	StartOrStopRecord(m_bRecording);
}