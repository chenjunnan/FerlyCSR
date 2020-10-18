#pragma once
#include <string>
#include "winrec.h"
#include "xCommon.h"
#include "memorySwaper.h"
#include "audiohead.h"
#include "common_helper.h"
#include "acommon.h"




class audio_stream_capture
{
public:
	audio_stream_capture();
	bool init(AUDIO_INFO &config);
	bool start();
	void stop();
	void pushmedia(char *buffer,int len);
	~audio_stream_capture();
	struct	recorder *m_recorder;
private:
	AUDIO_INFO m_info;
	HANDLE m_pm;
	unsigned char *m_buffer;
	int m_buffersize;
	WAVEFORMATEX wavfmt;
	bool m_run;
};

