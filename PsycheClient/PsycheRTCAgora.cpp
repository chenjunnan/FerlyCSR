#include "PsycheRTCAgora.h"
#include "agora\AGEventDef.h"
#include "IAgoraMediaEngine.h"


PsycheRTCAgora::PsycheRTCAgora()
{
	m_run = false;
	_ntf = NULL;
	_useExtVideo = false;
}


PsycheRTCAgora::~PsycheRTCAgora()
{
	if (state())Thread_helper::quit();
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_QUIT:
	{
		PostQuitMessage(0);
		break;
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}



bool PsycheRTCAgora::Init( std::string windowsname)
{
	if (!common_config_get_field_string("agora", "appid", &_appid))
		return false;
	common_config_get_field_string("psyche", "devicename", &m_devicename);


	if (_appid.empty())return false;
	CAgoraObject::SetMsgHandler(this);
	m_lpAgoraObject = CAgoraObject::GetAgoraObject(_appid.c_str());
	m_lpRtcEngine = CAgoraObject::GetEngine();
	m_lpAgoraObject->EnableVideo(TRUE);
	m_lpAgoraObject->EnableAudio(TRUE);
	m_lpAgoraObject->SetLogFilePath(NULL);
	m_lpAgoraObject->SetLiveState();
	int vt=m_agAudioin.GetVolume();
	m_agAudioin.SetVolume(vt);

	m_agCamera.Create(m_lpRtcEngine);
	m_agPlayout.Create(m_lpRtcEngine);
	
	//m_lpAgoraObject->EnableNetworkTest(TRUE);


	common_config_get_field_string("psyche", "streamname", &_streamname);
	if (!_streamname.empty())
	{
		agora::util::AutoPtr<agora::media::IMediaEngine> mediaEngine;
		mediaEngine.queryInterface(m_lpRtcEngine, agora::AGORA_IID_MEDIA_ENGINE);
		if (mediaEngine)
		{
			if (mediaEngine->setExternalVideoSource(true, false) != 0)
			{
				LOGE("set external video failed");
				_streamname.clear();
			}
		}
	}
	else
	{

		std::string devicename, deviceguid;
		if (m_agPlayout.GetDeviceCount() <= 0)
		{
			LOGE("no playout device");
		}
		else
		{
			if (m_agPlayout.GetDevice(0, devicename, deviceguid))
				m_agPlayout.SetCurDevice(deviceguid.c_str());

		}


	}

	m_windowstitle = "agora";
	common_get_guid(&m_windowsclassname);
	return start();
}

bool PsycheRTCAgora::Join(std::string id, int uid)
{
	return _joinchannel(id,uid);

}

bool PsycheRTCAgora::Call(std::string id, int uid )
{
	return _joinchannel(id,uid);

}

void PsycheRTCAgora::SetNotify(PsycheNotify *p)
{
	_ntf = p;
}

void PsycheRTCAgora::CloseReader()
{
	if (!_streamname.empty() && _cameraReader.state())
		_cameraReader.quit();
}

bool PsycheRTCAgora::_joinchannel(std::string id, int uid)
{
	if (_appid.empty())
	{
		if (!Init(""))return false;
	}
	
	std::string strChannelName = id;
	if (strChannelName.size() == 0)
	{
		LOGE("channel empty");
		return false;
	}

	if (!_streamname.empty())
	{
		int ret = _cameraReader.StartUp(_streamname);
		if (ret)
		{
			LOGE("Startup failed %d", ret);
			return false;
		}
		LOGD("use live camera stream");
	}
	else
	{
		bool found = false;
		if (!m_devicename.empty())
		{
			std::string devicename, deviceguid;

			for (int i = 0; i < m_agCamera.GetDeviceCount(); i++)
			{
				if (m_agCamera.GetDevice(i, devicename, deviceguid))
				{
					if (m_devicename.compare(devicename) == 0)
					{
						bool retSetCap=m_agCamera.SetCurDevice(deviceguid.c_str());
						LOGW("%s %s %s camera", retSetCap?"use":"cant use", devicename.c_str(), deviceguid.c_str());

						std::string currentid = m_agCamera.GetCurDeviceID();
						if (deviceguid.compare(currentid)==0)
							found = true;
						break;
					}
				}
			}
			if (!found)
			{
				LOGE("cant find [%s] device", m_devicename.c_str());
			}
		}
		if (!found)
		{
			LOGD("use default camera");
		}
	}



	VideoCanvas vc;

	vc.uid = 0;
	vc.view = NULL;
	vc.renderMode = RENDER_MODE_TYPE::RENDER_MODE_HIDDEN;

	//cancel setVideoProfile bitrate since version 2.1.0
	m_lpRtcEngine->setVideoProfile(VIDEO_PROFILE_TYPE::VIDEO_PROFILE_LANDSCAPE_480P, false);
	m_lpAgoraObject->EnableVideo(TRUE);
	m_lpAgoraObject->EnableAudio(TRUE);
	//m_lpRtcEngine->setupLocalVideo(vc);
	//m_lpRtcEngine->startPreview();

	bool ret = m_lpAgoraObject->JoinChannel(strChannelName.c_str(), uid);
	if (ret)
		m_run = true;
	return ret;
}

int PsycheRTCAgora::Status()
{
	return m_run;
}

bool PsycheRTCAgora::Leave()
{
	if (_appid.empty())return true;
	IRtcEngine		*lpRtcEngine = CAgoraObject::GetEngine();
	CAgoraObject	*lpAgoraObject = CAgoraObject::GetAgoraObject();
	m_run = false;
	CloseReader();
	bool ret =lpAgoraObject->LeaveCahnnel();;
	LOGE("RTC LeaveCahnnel");
	m_agCamera.Close();
	m_agPlayout.Close();

	_appid.clear();
	return ret;
}

void PsycheRTCAgora::Close()
{
	::SendMessage(m_hWnd, WM_QUIT, 0, 0);
	Thread_helper::wait();

	if (!_appid.empty())
	{	
		LOGE("RTC CloseAgoraObject");
		m_lpRtcEngine->release();
		m_lpAgoraObject->CloseAgoraObject();
	}
}


void PsycheRTCAgora::Shutup()
{
	m_lpAgoraObject->SetPlaybackVolume(0);
}

void PsycheRTCAgora::Speak()
{
	m_lpAgoraObject->SetPlaybackVolume(100);
}
void PsycheRTCAgora::SetLocalVideoMute(bool status)
{
	m_lpAgoraObject->MuteLocalVideo(status);
}
void PsycheRTCAgora::SetLocalAudioMute(bool status)
{
	m_lpAgoraObject->MuteLocalAudio(status);
}
void PsycheRTCAgora::SetRemoteVideoMuteStatus(unsigned int uid, bool status)
{
	m_lpAgoraObject->MutexRemoteVideo(uid, status);
}
void PsycheRTCAgora::SetRemoteAudioMuteStatus(unsigned int uid, bool status)
{
	m_lpAgoraObject->MutexRemoteAudio(uid, status);
}
int PsycheRTCAgora::GetHelperList(std::map<unsigned int, std::string> &mapl)
{
	std::map<unsigned int, VideoServicer> data=_assWriter.getStreamList();
	mapl.clear();
	for (auto it = data.begin(); it != data.end();it++)
		mapl[it->first] = it->second.streamname;
	return 0;
}

void PsycheRTCAgora::onActiveSpeaker(uid_t uid)
{

}

void PsycheRTCAgora::onFirstRemoteVideoDecoded(uid_t uid, int width, int height, int elapsed)
{
	_list[uid].height = height;
	_list[uid].width = width;
// 	VideoCanvas vc;
// 	vc.uid = uid;
// 	vc.view = m_hWnd;
// 	vc.renderMode = RENDER_MODE_TYPE::RENDER_MODE_FIT;
// 	m_lpRtcEngine->setupRemoteVideo(vc);
	printf("FirstRemoteVideoDecoded %u\n", uid);
	std::string name;

	_ntf->onUserOnline(uid, name);
}

void PsycheRTCAgora::onJoinChannelSuccess(const char* channel, uid_t uid, int elapsed)
{
	_list.clear();
	m_uid = uid;
	LOGI("JoinChannelSuccess  channel:%s uid:%u\n", channel, uid);
}

void PsycheRTCAgora::onConnectionInterrupted() 
{
	_ntf->onRtcError(900001,"connect interrupted");
	LOGE("ConnectionInterrupted\n");
}

void PsycheRTCAgora::onUserJoined(uid_t uid, int elapsed)
{
	printf("UserOnline %u\n", uid);
}
void PsycheRTCAgora::onUserOffline(uid_t uid, USER_OFFLINE_REASON_TYPE reason)
{
	if (_list.find(uid) != _list.end())
		_list.erase(uid);
	printf("UserOffline %u\n", uid);
	std::string name;
	_ntf->onUserOffline(uid, name);
}

void PsycheRTCAgora::onRejoinChannelSuccess(const char* channel, uid_t uid, int elapsed)
{
	_list.clear();
	LOGI("JoinChannelSuccess channel:%s uid:%u\n", channel, uid);
}

void PsycheRTCAgora::onError(int err, const char* msg)
{
	LOGE("happen Error %s %d\n", msg, err);
	_ntf->onRtcError(err,msg);
	m_run = false;
	m_uid = 0;
}

void PsycheRTCAgora::onLeaveChannel(const RtcStats& stats)
{
	m_run = false;
	m_uid = 0;
	LOGI("LeaveChannel\n");
}

void PsycheRTCAgora::onRtcStats(const RtcStats& stats)
{
}

void PsycheRTCAgora::onConnectionLost()
{
	_ntf->onRtcError(900001,"lost connect");
	LOGE("ConnectionLost\n");
}

void PsycheRTCAgora::run()
{
	HINSTANCE hInstance = ::GetModuleHandle(NULL);

	WNDCLASSEXA wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, NULL);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = m_windowsclassname.c_str();
	wcex.hIconSm = LoadIcon(wcex.hInstance, NULL);

	DWORD ret = RegisterClassExA(&wcex);
	if (ret == 0)
	{
		LOGE("register windows class failed error :%d.", GetLastError());
		return;
	}

	m_hWnd = CreateWindowA(m_windowsclassname.c_str(), m_windowstitle.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!m_hWnd)
	{
		LOGE("createwindow failed error :%d.", GetLastError());
		return;
	}

	ShowWindow(m_hWnd, SW_HIDE);
	UpdateWindow(m_hWnd);


	MSG msg;
	Message msgth;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		msgth = getq(0);
		if (msgth.msgType == _EXIT_THREAD_)break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	DestroyWindow(m_hWnd);
	UnregisterClass(m_windowsclassname.c_str(), hInstance);
	LOGI("exit agora thread.");
}
