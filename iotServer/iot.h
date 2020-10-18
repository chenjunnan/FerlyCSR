#pragma once
#include <list>
#include "json\json.h"
#include "common_helper.h"
extern const char * MODULE_NAME;

#ifdef WIN32
#define LOGBOX_FORMAT(name, level, file,function,line, logformat, ...) \
{do{ \
	common_log(name,level,file,function,line,logformat, ##__VA_ARGS__);\
} while (0);}
#else
#endif

#define LOGBT( fmt, ...) LOGBOX_FORMAT(MODULE_NAME, LOG_LEVEL_TRACE, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGBD( fmt, ...) LOGBOX_FORMAT(MODULE_NAME, LOG_LEVEL_DEBUG, __FILE__,__FUNCTION__,  __LINE__, fmt, ##__VA_ARGS__)
#define LOGBI( fmt, ...) LOGBOX_FORMAT(MODULE_NAME, LOG_LEVEL_INFO, __FILE__,__FUNCTION__,  __LINE__, fmt, ##__VA_ARGS__)
#define LOGBW( fmt, ...) LOGBOX_FORMAT(MODULE_NAME,LOG_LEVEL_WARN, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGBE( fmt, ...) LOGBOX_FORMAT(MODULE_NAME, LOG_LEVEL_ERROR, __FILE__,__FUNCTION__,  __LINE__, fmt, ##__VA_ARGS__)
#define LOGBF( fmt, ...) LOGBOX_FORMAT(MODULE_NAME, LOG_LEVEL_FATAL, __FILE__,__FUNCTION__,  __LINE__, fmt, ##__VA_ARGS__)


enum FLDEVICE_TYPE
{
	DBOX = 0,
	DFRAME
};

enum MESSAGE_TYPE
{
	GETLIST = 99,
	OPENDOOR,
	CLOSEDOOR,
	GETDOORSTATUS,
	GETPAPERSTATUS,
	GETBOXBATTERY,
	SETBOXAP,
	SETFRAMEADSORB_OPEN,
	SETFRAMEADSORB_CLOSE,
};


class msg_context
{
public:
	msg_context(){}
	~msg_context(){}
	std::string id;
	std::string mainid;
	std::string guid;
	int mode = 0;
	int timeout = 10 * 1000;
	std::list<std::pair<int, int> > door_list;
};


class MessageRequest
{
public:
	MessageRequest(int t, unsigned long long time)
	{
		type = t; send_time = time;
	}
	~MessageRequest(){}
	int type;
	std::string resultjson;
	unsigned long long send_time;
	msg_context  context;
};


class SubmoduleInterface :public Thread_helper
{
public:
	SubmoduleInterface(){}
	virtual ~SubmoduleInterface(){}
	virtual void AppendPortList(Json::Value &json_object_base) = 0;
};


#define BOX_MAX_LAYER 2

void appendmessagetype(Json::Value &json_object_post, int type);
void appenderrormessage(Json::Value &json_object_post, int error);
std::string JsonPackage(std::string id, int type, int error);
std::string JsonPackage(std::string id, int type, int value, int error);
std::string JsonPackage(std::string id, int type, std::vector<std::string > infoarray, int error);
std::vector<std::string> split(std::string strtem, char a);