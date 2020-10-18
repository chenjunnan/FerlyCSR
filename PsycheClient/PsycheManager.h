#pragma once
#include <stdio.h>
#include "PsycheWebsocket.h"
#include "soundplayer.h"
#include "PsycheRTCInterface.h"
#include "response.h"
#include <list>
#include <wtypes.h>

struct REQUEST_CACHE_INFO
{
	std::string resultdata;
	std::string requestdata;
	std::string fromer;
	std::string function;
	unsigned long long time = 0;
	int timeout_due = 10;
	bool remove_flag = false;
};


enum PSYCHEM_STATUS
{
	PSYCHEM_IDE = 0,
	PSYCHEM_WAIT_CALL_WS_RESPONSE = 100,
	PSYCHEM_WAIT_CALL_CAMERA_RESPONSE,
	PSYCHEM_WAIT_CALL_RTC_RESPONSE,
	PSYCHEM_WAIT_CALL_CAMERA_BY_REMOTE_RESPONSE,
	PSYCHEM_WAIT_CALL_CAMERA_BY_REMOTE_RESPONSE_NOACK,

	PSYCHEM_WAIT_LEAVE_RTC_RESPONSE = 200,
	PSYCHEM_WAIT_LEAVE_WS_RESPONSE,
	PSYCHEM_WAIT_LEAVE_CAMERA_RESPONSE

};


class PsycheManager :public SoundStateNotify,
	public PsycheMessageNotify
	,public Thread_helper,
	public PsycheNotify
{
public:
	static PsycheManager *getInstance();
	bool init();
	bool StartManager();
	bool StopManager();
	
	int HandleMessage(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata = NULL, unsigned long  dExtlen = 0);


	virtual void onRtcError(int s, const char *);
	virtual void onUserOffline(unsigned int uid, std::string stream);
	virtual void onUserOnline(unsigned int uid, std::string stream);
	virtual void onResume(){}
	virtual void onPause(){}
	virtual void onFinsih() {}
	virtual void onProcess(double rate) {}
	
	virtual void onTextJsonSingal(std::string &json);


	
	virtual void onDisconnect();
	virtual ~PsycheManager();
private:
	int handle_camera_response(int error);
	void play_startupsound();
	void build_call(std::string &ercode, Json::Value &json_object);
	void live_join(std::string &ercode, Json::Value &json_object);
	void join_monitor(Json::Value &json_object);
	void web_leave(std::string &packtype);
	void command_msg(Json::Value &json_objectr, std::string packtype, std::string fun);

	void open_camera();
	void close_camera();
	std::string package_websocket_context(std::string source, std::string fun, std::string packtype);
	std::string package_websocket_context(std::string source, std::string fun, std::string packtype, Json::Value &data);
	virtual void run();
	PsycheManager();
	PsycheWebsocket  _pws;
	CSoundPlayer *player;
	PsycheRTCInterface *_rtc;
	robot_json_rsp _json;
	CHANDLE m_http;
	std::string _streamname;

	std::string m_robot_id;
	std::string _wsurl;
	std::string _tokenurl;
	std::string _token;
	std::string _channel;
	unsigned int  _uid;

	std::map<std::string, std::list<REQUEST_CACHE_INFO> > m_map_remote;

	PSYCHEM_STATUS psyche_state;
	CHANDLE		m_state_ml;
	CHANDLE		m_map_ml;
};