#pragma once

#include <string>
struct AUDIO_INFO
{
	int nDeviceID;
	int nSampleRate;
	std::string streamsname;

};


#define DEFAULT_FORMAT		\
{\
	WAVE_FORMAT_PCM,	\
	1,					\
	16000,				\
	32000,				\
	2,					\
	16,					\
	sizeof(WAVEFORMATEX)	\
}
