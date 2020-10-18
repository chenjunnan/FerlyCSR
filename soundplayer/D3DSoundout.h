#pragma once
#define _WIN32_DCOM
#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include <xaudio2.h>
#include <strsafe.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <conio.h>
#include <Xact3wb.h>
#include "soundplayer.h"
#include "common_helper.h"
#include <thread>

#define TTS_IFLY_MEDIA_DATA 101
#define TTS_IFLY_MEDIA_DATA_OVER 102
#define TTS_PLAY_OVER			103
#define TTS_PLAY_STOP			104
#define TTS_PLAY_PAUSE			105
#define TTS_PLAY_RESUME			106
//--------------------------------------------------------------------------------------
// Callback structure
//--------------------------------------------------------------------------------------
#define STREAMING_BUFFER_SIZE (128*1024)
//(64*1024)
#define MAX_BUFFER_COUNT 8
int sample_rate = 16000;	//PCM sample rate
int channels = 1;			//PCM channel number
int bits_per_sample = 16;	//bits per sample

struct StreamingVoiceContext : public IXAudio2VoiceCallback
{
	STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32 pos)
	{
		//printf("OnVoiceProcessingPassStart %d\n",pos);
	}
	STDMETHOD_(void, OnVoiceProcessingPassEnd)()
	{
		//printf("OnVoiceProcessingPassEnd \n");
	}
	STDMETHOD_(void, OnStreamEnd)()
	{
		printf("OnStreamEnd\n");
	}
	STDMETHOD_(void, OnBufferStart)(void*)
	{
		//printf("OnBufferStart \n");
	}
	STDMETHOD_(void, OnBufferEnd)(void*)
	{
		SetEvent(hBufferEndEvent);
		notify_thread->pushq(TTS_PLAY_OVER);
	}
	STDMETHOD_(void, OnLoopEnd)(void*)
	{
		printf("OnLoopEnd\n");
	}
	STDMETHOD_(void, OnVoiceError)(void*, HRESULT)
	{
		LOGE("OnVoiceError");
	}

	HANDLE hBufferEndEvent;
	Thread_helper *notify_thread;

	StreamingVoiceContext(Thread_helper * th) : hBufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL))
	{
		notify_thread = th;
	}
	virtual ~StreamingVoiceContext()
	{
		CloseHandle(hBufferEndEvent);
	}
};
enum PLAYER_STATE
{
	PLAYIDE=0,
	PLAYING,
	PLAYREADY,
	PLAYPAUSE,
};

class SoundBuffer
{
public:
	SoundBuffer();
	~SoundBuffer();
	void Insert(unsigned char *buffer, int len);
	int  RemainLength();
	bool GetRemove(unsigned char *buffer, int len);
	void Clean();
	double Rate();
private:
	CHANDLE m_bfml;
	std::vector<unsigned char> m_buffer;
	unsigned long long      m_totle_datalen;
	unsigned long long      m_post_datalen;
};

class SubmitBufferManager
{
public:
	SubmitBufferManager();
	~SubmitBufferManager();
	void Init();
	int PlayData(SoundBuffer *sbufer, BYTE * &ptr);
private:
	BYTE(*m_buffers)[STREAMING_BUFFER_SIZE];
	DWORD currentDiskReadBuffer;
};

class D3DSoundout :
	public CSoundPlayer, 
	public Thread_helper
{
public:
	D3DSoundout();
	virtual void set_notify(SoundStateNotify*_notify) ;
	virtual int init(short nChannels,
		int       nSamplesPerSec,    /* sample rate */
		int       nAvgBytesPerSec,   /* for buffer estimation */
		short        nBlockAlign,       /* block size of data */
		short        wBitsPerSample /* number of bits per sample of mono data */) ;
	virtual bool reset();
	virtual bool stop();
	virtual bool pause();
	virtual bool resume();
	virtual bool isfinsh();
	virtual void brccache() ;
	virtual void quit();
	virtual void sound(unsigned char *data, int len) ;
	virtual void endsound();
	virtual ~D3DSoundout();

private:
	virtual void run();

	void cleanup();

	void call_process();

	bool isPlayDone();

	void waitEnd();

	int  TrySubmitAudioData();

	bool isFull();

	void Init_once();

	//bool    m_fast_stop;
	StreamingVoiceContext m_voiceContext;
	WAVEFORMATEX    wfx;
	HRESULT hr;
	IXAudio2MasteringVoice* pMasteringVoice;
	IXAudio2* pXAudio2;
	IXAudio2SourceVoice* pSourceVoice;
	SoundStateNotify	*m_notify;

	SubmitBufferManager     m_submit_manager;
	SoundBuffer				m_media_cache;
	CHANDLE			       m_statelock;
	HANDLE				  m_waitpauseevent;
	HANDLE				  m_waitstopevent;
	PLAYER_STATE           m_cur_state;
	bool					m_all_finish;
	HMODULE mXAudioDLL;
};

