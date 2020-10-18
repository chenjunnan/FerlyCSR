#include "PsycheWebsocket.h"
#include "json\json.h"
#include "robot_PostMessage.h"
extern const char* MODULE_NAME;

PsycheWebsocket::PsycheWebsocket(PsycheMessageNotify *ntf) :_imp(NULL), _ntf(ntf)
{
	link = false;
	_imp = CreateWSClient(this);
}

bool PsycheWebsocket::isConnect()
{
	return link;//nosyn
}

void PsycheWebsocket::setUrl(const char *uri)
{
	_uri = uri;
}

PsycheWebsocket::~PsycheWebsocket()
{
	_imp = NULL;
}

int PsycheWebsocket::PostCommand(OPCODE code, const char *data, int datalen)
{
	if (!_imp)return -1;
	if (!state())return -101;
	if (!link)return -103;
	return _imp->WSSend(code, data, datalen);
}

void PsycheWebsocket::quit()
{
	if (!_imp)return;
	if (state())
	{
		_imp->WSClose();
		wait();
	}
}

void PsycheWebsocket::run()
{
	if (!_imp || _uri.size()==0)
	{
		LOGE("create ws failed.");
		return;
	}
	bool loop = true;
	int sleeps = 0;
	link = false;

	_imp->WSReset();
	_imp->WSURL(_uri.c_str());
	_imp->WSRun();

	LOGW("exit psyche command thread .");
}

void PsycheWebsocket::onMessage(const char * data, int datalen, OPCODE code)
{
	if (code == OPCODE::OPCODETEXT)
	{
		std::string text;
		text.assign(data, data + datalen);
		if (_ntf)
			_ntf->onTextJsonSingal(text);
	}
}

void PsycheWebsocket::onFailed(int code, const char * errmsg)
{
	LOGE("ws failed %d %s",code,errmsg);
	link = false;
	if (_ntf)
		_ntf->onDisconnect();
}

void PsycheWebsocket::onOpen()
{
	LOGI("%s open success",_uri.c_str());
	link = true;
}

void PsycheWebsocket::onClose()
{
	LOGE("%s close", _uri.c_str());
	if (link)
		_imp->WSClose();
	link = false;
	if (_ntf)
		_ntf->onDisconnect();
}

PsycheWebsocketService::PsycheWebsocketService(PsycheMessageNotify *ntf) :_imp(NULL), _ntf(ntf)
{
	_imp = CreateWSServer(this);
}

int PsycheWebsocketService::PostCommand(OPCODE code, const char *data, int datalen)
{
	if (!state())return -1;
	return _imp->WSBroadcast(code, data, datalen);
}

void PsycheWebsocketService::QuitService()
{
	if (state())
	{
		_imp->WSStop();
		wait(3000);
	}

}

void PsycheWebsocketService::run()
{
	_imp->WSReset();
	_imp->WSRun(9081);
	LOGW("exit psyche service thread .");
}

void PsycheWebsocketService::onMessage(const char *name, const char * data, int datalen, OPCODE code)
{
	Json::Value json_object;
	Json::FastWriter  json_writer;
	Json::Reader  json_reader(Json::Features::strictMode());
	bool b_json_read = json_reader.parse(data, json_object);
	if (code == OPCODE::OPCODETEXT)
	{
		std::string msgcontect;
		msgcontect.assign(data, data + datalen);
		robot_PostMessage::getPostManager()->post(MODULE_NAME, msgcontect.c_str(), msgcontect.length(), "unname_dispath");
	}

}
