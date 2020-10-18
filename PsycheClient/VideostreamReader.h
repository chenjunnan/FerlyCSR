#pragma once
#include <Windows.h>
#include "common_helper.h"
#include "memorySwaper.h"
#include <list>
#include "camerastreamstruct.h"


struct VIDEO_MEDIADATA
{
	VideoMemoryHeadFormat  video_format;
	std::vector<char> data;
};

class VideostreamReader :public Thread_helper
{
public:
	VideostreamReader();

	void  set_stream(std::string streamname, std::string readername);
	int   startup_receive();
	bool  get_media(VIDEO_MEDIADATA&data);

	virtual ~VideostreamReader();
private:
	bool malloc_buffer(std::vector<unsigned char> &buffer);
	virtual void run();
	std::string m_streamname;
	std::string m_readname;
	HANDLE       m_pm;
	std::list<VIDEO_MEDIADATA> m_media;
	CHANDLE    m_ml;
	HANDLE     m_c;
};

