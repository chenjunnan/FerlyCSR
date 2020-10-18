#pragma once

#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif // DLLEXPORT

enum class OPCODE
{
	OPCODECONTINUATION = 0x0,
	OPCODETEXT = 0x1,
	OPCODEBINARY = 0x2,
	OPCODERSV3 = 0x3,
	OPCODERSV4 = 0x4,
	OPCODERSV5 = 0x5,
	OPCODERSV6 = 0x6,
	OPCODERSV7 = 0x7,
	OPCODECLOSE = 0x8,
	OPCODEPING = 0x9,
	OPCODEPONG = 0xA,
};


class ClientMessageInterface
{
public:
	virtual ~ClientMessageInterface() {}
	virtual void onMessage(const char * data,int datalen, OPCODE code)=0;
	virtual void onFailed(int code,const char * errmsg) {}
	virtual void onOpen() {}
	virtual void onClose() {}
	virtual bool onPing(const char *payload) { return false; }
	virtual void onPong(const char *payload) {}
	virtual void onPongTimeout(const char *payload) {}

};

class WebsocketClientInterface
{
public:
	virtual ~WebsocketClientInterface() {}
	virtual void WSURL(const char * uri)=0;
	virtual void WSReset()=0;
	virtual int WSRun() = 0;
	virtual int WSPoll()=0;
	virtual int WSClose() = 0;
	virtual bool WSisStoped() = 0;
	virtual int WSSend(OPCODE code,const char *data,int datalen) = 0;
};

class HttpInterface
{
public:
	virtual ~HttpInterface() {}
	virtual int getport()=0;
	virtual const char* gethost()=0;
	virtual const char* getresource() = 0;
	virtual const char* get_uri() = 0;
	virtual const char* get_head(const char *key) = 0;
	virtual	int add_head(const char *key,const char *value) = 0;
	virtual	int remove_head(const char *key) = 0;
	virtual const char *getbody()=0;
	virtual void setstatus(int state)=0;
	virtual void setbody(const char *body) = 0;
};


class ServerMessageInterface
{
public:
	virtual ~ServerMessageInterface() {}
	virtual void onMessage(const char *name,const char * data, int datalen, OPCODE code) = 0;
	virtual void onHttp(HttpInterface *httprquest) {}
	virtual void onOpen(const char *name) {}
	virtual void onClose(const char *name) {}
	virtual bool onPing(const char *name,const char *payload) { return false; }
	virtual void onPong(const char *name,const char *payload) {}
	virtual void onPongTimeout(const char *name,const char *payload) {}
};

class  WebsocketServerInterface
{
public:
	virtual ~WebsocketServerInterface() {}
	virtual void WSReset() = 0;
	virtual int WSRun(int port) = 0;
	virtual int WSStop() = 0;
	virtual bool WSisStoped() = 0;
	virtual int WSSend(const char *name, OPCODE code, const char *data, int datalen) = 0;
	virtual int WSBroadcast(OPCODE code, const char *data, int datalen) = 0;
};


extern "C"
{
	DLLEXPORT WebsocketClientInterface* CreateWSClient(ClientMessageInterface *);
	DLLEXPORT void  DestoryWSClient(WebsocketClientInterface *);

	DLLEXPORT WebsocketServerInterface* CreateWSServer(ServerMessageInterface *);
	DLLEXPORT void  DestoryWSServer(WebsocketServerInterface *);
}