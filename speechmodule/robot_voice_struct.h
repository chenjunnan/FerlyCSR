#pragma once
#include <list>
#include "common_helper.h"

#define  CCB_OFFLINEIFLY//建行版本

extern const char *MODULE_NAME;

#define BOOLTOINT(x) x?0:1
enum ISRSTATE
{
	ISRINIT = 0,
	ISRIDE,
	ISRRUNING,
	ISRDONE
};


struct  VoiceInStruct
{
	std::string 	request_result_type;
	bool			loop;
	int				mode;
	std::string		text;//utf-8
	VoiceInStruct()
	{
		loop = false;
		mode=0;
	}
};

struct  VoiceOutStruct
{
	std::string isr_result;
	std::string nlp_result;
};


struct  VoiceMessage
{
	int			appid;
	std::string fromguid;
	std::string content;
	VoiceInStruct in;
	VoiceOutStruct out;
	int			error;

	VoiceMessage()
	{
		appid = 0;
		error = 0;
	}
};

struct slot_context
{
	int id;
	int sc;
	std::string context;
	slot_context()
	{
		id = 0;
		sc = 0;
	}
};

struct  slot_id
{
	bool indata;
	std::string slot_name;
	std::vector<slot_context> slot_context;
	slot_id()
	{
		indata = false;
	}
};

enum PLAY_STATUS
{
	PLAY_STATUS_WAI_FINIFSH = 0,
	PLAY_STATUS_WAIT_DATA,
	PLAY_STATUS_STOP,
	PLAY_STATUS_RESET
};

class robot_sound_buffer_interface
{
public:
	robot_sound_buffer_interface(){ buffer_ml = common_mutex_create_threadlock(); }
	virtual void setText(std::string text){ LOCK_HELPER_AUTO_HANDLER(buffer_ml); _text = text; }
	virtual int  push_data(unsigned char *ptr, int len) = 0;
	virtual void done() = 0;
	virtual void discard() = 0;
	virtual void clear() = 0;
	virtual bool isStop()=0;
	virtual int  status()=0;
	virtual int  work() = 0;
	virtual ~robot_sound_buffer_interface(){ common_mutex_threadlock_release(buffer_ml); };
	std::string _text;
	CHANDLE		 buffer_ml;
};


struct tts_message
{
	int channels;
	std::string context;
	bool recoder;
	bool active;
	int id_cache;
	std::list<robot_sound_buffer_interface *> handler;
	tts_message()
	{
		id_cache = -1;
		channels = 0;
		recoder = false;
		active = true;
	}
};

#define MAX_PARAMS_LEN      (1024)
#define MAX_GRAMMARID_LEN   (32)

typedef struct _GrammarData {
	unsigned long ts;
	int     build_fini;  //标识语法构建是否完成
	int     update_fini; //标识更新词典是否完成
	int     errcode; //记录语法构建或更新词典回调错误码
	_GrammarData()
	{
		build_fini = 0; update_fini = 0; errcode = 0;
	}
}GrammarData;
