#pragma once
#include "common_helper.h"
#include <map>

class PsycheNotify
{
public:
	virtual ~PsycheNotify(){}
	virtual void onRtcError(int e,const char *msge){}
	virtual void onUserOffline(unsigned int uid, std::string stream){}
	virtual void onUserOnline(unsigned int uid, std::string stream){}
};

class PsycheRTCInterface:
	protected Thread_helper
{
public:
	PsycheRTCInterface();
	virtual void SetNotify(PsycheNotify *)=0;
	virtual bool Init(std::string arg="" )=0;
	virtual bool Join(std::string id,int uid=0) = 0;
	virtual bool Call(std::string id, int uid = 0) = 0;
	virtual int  Status() = 0;
	virtual bool Leave() = 0;
	virtual void Close() = 0;
	virtual void Shutup()=0;
	virtual void Speak()=0;
	virtual void SetLocalVideoMute(bool status){}
	virtual void SetLocalAudioMute(bool status){}
	virtual void SetRemoteVideoMuteStatus(unsigned int uid,bool status){}
	virtual void SetRemoteAudioMuteStatus(unsigned int uid, bool status){}

	virtual ~PsycheRTCInterface();
};

