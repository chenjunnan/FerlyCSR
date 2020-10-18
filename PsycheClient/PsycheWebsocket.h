#pragma once
#include "common_helper.h"
#include "websockets.h"
class PsycheMessageNotify
{
public:
	PsycheMessageNotify(){}
	~PsycheMessageNotify(){}
	virtual void onTextJsonSingal(std::string &)=0;
	virtual void onDisconnect()=0;
};
class PsycheWebsocket :
	public Thread_helper,
	public ClientMessageInterface
{
public:
	PsycheWebsocket(PsycheMessageNotify *);
	bool isConnect();
	void setUrl(const char *uri);
	virtual ~PsycheWebsocket();
	virtual int PostCommand(OPCODE code, const char *data, int datalen);
	virtual void quit();
protected:
	virtual void run();
	virtual void onMessage(const char * data, int datalen, OPCODE code);
	virtual void onFailed(int code, const char * errmsg);
	virtual void onOpen();
	virtual void onClose();
	WebsocketClientInterface *_imp;
	PsycheMessageNotify *_ntf;
	std::string _uri;
	bool		_exit;
	bool		link;
};

class PsycheWebsocketService :
	public Thread_helper,
	public ServerMessageInterface
{
public:
	PsycheWebsocketService(PsycheMessageNotify *);
	virtual ~PsycheWebsocketService(){}
	virtual int PostCommand(OPCODE code, const char *data, int datalen);
	virtual void QuitService();
protected:
	virtual void run();
	virtual void onMessage(const char *name, const char * data, int datalen, OPCODE code);
	virtual void onHttp(HttpInterface *httprquest){}
	virtual void onOpen(const char *name){}
	virtual void onClose(const char *name){}
	WebsocketServerInterface *_imp;
	PsycheMessageNotify *_ntf;
	std::string _uri;
	bool		_exit;
	bool		link;
};


