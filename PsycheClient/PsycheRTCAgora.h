#pragma once
#include "PsycheRTCInterface.h"
#include "AgoraBase.h"
#include "agora\AgoraObject.h"
#include "IAgoraRtcEngine.h"
#include "agora\AgoraPlayoutManager.h"
#include "agora\AgoraAudInputManager.h"
#include "agora\AgoraCameraManager.h"
#include "common_helper.h"
#include <map>
#include "PsycheRTCAgoraVideoPub.h"
using namespace agora::rtc;

struct USER_LIST_INFO
{
	int   width;
	int height;
};

class PsycheRTCAgora :
	public PsycheRTCInterface,
	public IRtcEngineEventHandler
{
public:
	PsycheRTCAgora();
	virtual ~PsycheRTCAgora();
	virtual bool Init(std::string arg);
	virtual bool Join(std::string id, int uid = 0);
	virtual bool Call(std::string id, int uid = 0);
	virtual void SetNotify(PsycheNotify *);
	virtual void CloseReader();
	virtual int  Status();
	virtual bool Leave() ;
	virtual void Close();
	virtual void Shutup();
	virtual void Speak();

	virtual void SetLocalVideoMute(bool status);
	virtual void SetLocalAudioMute(bool status);
	virtual void SetRemoteVideoMuteStatus(unsigned int uid, bool status);
	virtual void SetRemoteAudioMuteStatus(unsigned int uid, bool status);
	virtual void onActiveSpeaker(uid_t uid);
	virtual void onFirstRemoteVideoDecoded(uid_t uid, int width, int height, int elapsed);
	virtual void onJoinChannelSuccess(const char* channel, uid_t uid, int elapsed);
	virtual void onConnectionInterrupted();
	virtual void onUserJoined(uid_t uid, int elapsed);
	virtual void onUserOffline(uid_t uid, USER_OFFLINE_REASON_TYPE reason);
	virtual void onRejoinChannelSuccess(const char* channel, uid_t uid, int elapsed);
	virtual void onError(int err, const char* msg);
	virtual void onLeaveChannel(const RtcStats& stats);
	virtual void onRtcStats(const RtcStats& stats);
	virtual void onConnectionLost();

private:
	bool _joinchannel(std::string id, int uid = 0);

	virtual void run();
	CAgoraObject	*m_lpAgoraObject;
	IRtcEngine		*m_lpRtcEngine;
	CAgoraPlayoutManager	m_agPlayout;
	CAgoraAudInputManager	m_agAudioin;
	CAgoraCameraManager		m_agCamera;
	std::string m_windowstitle;
	std::string m_windowsclassname;
	HDEVNOTIFY m_hDevNotify = 0;
	PsycheNotify *   _ntf;
	std::string _appid;
	HWND m_hWnd;
	GUID   m_guid;
	bool m_run;
	uid_t  m_uid;
	std::string m_devicename;
	std::map<unsigned int, USER_LIST_INFO>  _list;
	PsycheRTCAgoraVideoPub     _cameraReader;
	bool						_useExtVideo;
	std::string _streamname;
};

