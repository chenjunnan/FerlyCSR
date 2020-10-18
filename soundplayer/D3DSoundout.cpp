
#include "D3DSoundout.h"
#include "common_helper.h"




D3DSoundout::D3DSoundout() :m_voiceContext(this)
{
	pMasteringVoice = NULL;
	pXAudio2 = NULL;
	mXAudioDLL = NULL;
	m_cur_state = PLAYIDE;
	m_statelock = common_mutex_create_threadlock();
	m_waitstopevent =  CreateEvent(NULL, FALSE, FALSE, NULL);
	m_waitpauseevent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_notify = NULL;
	m_all_finish = false;
	wfx.wFormatTag = WAVE_FORMAT_PCM;//设置波形声音的格式
	wfx.nChannels = 1;//设置音频文件的通道数量
	wfx.nSamplesPerSec = 16000;//设置每个声道播放和记录时的样本频率
	wfx.nAvgBytesPerSec = 32000;//设置请求的平均数据传输率,单位byte/s。这个值对于创建缓冲大小是很有用的
	wfx.nBlockAlign = 2;//以字节为单位设置块对齐
	wfx.wBitsPerSample = 16;
	wfx.cbSize = 0;//额外信息的大小
}


void D3DSoundout::set_notify(SoundStateNotify*_notify)
{
	m_notify = _notify;
}
int D3DSoundout::init(short nChannels,
	int       nSamplesPerSec,    /* sample rate */
	int       nAvgBytesPerSec,   /* for buffer estimation */
	short        nBlockAlign,       /* block size of data */
	short        wBitsPerSample /* number of bits per sample of mono data */)
{
	if (state())
		return 0;
	wfx.nChannels = nChannels;
	wfx.nSamplesPerSec = nSamplesPerSec;
	wfx.nAvgBytesPerSec = nAvgBytesPerSec;
	wfx.nBlockAlign = nBlockAlign;
	wfx.wBitsPerSample = wBitsPerSample;
	HRESULT hres;
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);

	if (FAILED(hres))
	{
		LOGE("CoInitializeEx error %#x", hres);
		return 1;
	}

	if (!mXAudioDLL)
#ifdef _DEBUG
	mXAudioDLL = LoadLibraryExW(L"XAudioD2_7.DLL", nullptr, 0x00000800 /* LOAD_LIBRARY_SEARCH_SYSTEM32 */);
#else
	mXAudioDLL = LoadLibraryExW(L"XAudio2_7.DLL", nullptr, 0x00000800 /* LOAD_LIBRARY_SEARCH_SYSTEM32 */);
#endif	
	if (!mXAudioDLL)
	{
		LOGE("Failed to find XAudio 2.7 DLL");
		CoUninitialize();
		return 1;
	}

	UINT32 flags = 0;

	if (FAILED(hr = XAudio2Create(&pXAudio2, flags)))
	{
		LOGE("Failed to init XAudio2 engine: %#X\n", hr);
		CoUninitialize();
		return 1;
	}
	SAFE_RELEASE(pXAudio2);
	CoUninitialize();
	brccache();

	return start() ? 0 : 1;
}

bool D3DSoundout::reset()
{
	LOCK_HELPER_AUTO_HANDLER(m_statelock)
	if (m_cur_state != PLAYIDE)return false;
	m_cur_state = PLAYREADY;
	return true;
}

bool D3DSoundout::stop()
{
	if (!state())return false;
	pushq(TTS_PLAY_STOP);
	WaitForSingleObject(m_waitstopevent, -1);
	return true;
}

bool D3DSoundout::pause()
{
	if (!state())return false;
	pushq(TTS_PLAY_PAUSE);
	WaitForSingleObject(m_waitpauseevent, -1);
	return true;
}

bool D3DSoundout::resume()
{
	if (!state())return false;
	pushq(TTS_PLAY_RESUME);
	WaitForSingleObject(m_waitpauseevent, -1);
	return true;
}

bool D3DSoundout::isfinsh()
{
	LOCK_HELPER_AUTO_HANDLER(m_statelock)
	return m_cur_state == PLAYIDE;
}


void D3DSoundout::brccache()
{	
	do
	{
		Message xMSG = getq(0);
		if (xMSG.msgType == -1)
			break;
	} while (1);
	m_media_cache.Clean();
}

void D3DSoundout::quit()
{

	Thread_helper::quit();
}


void D3DSoundout::sound(unsigned char *data, int len)
{
	if (!state())return ;
	m_media_cache.Insert(data, len);
	pushq(TTS_IFLY_MEDIA_DATA);
}

void D3DSoundout::endsound()
{
	pushq(TTS_IFLY_MEDIA_DATA_OVER);
}

D3DSoundout::~D3DSoundout()
{
	if (state())quit();
	common_mutex_threadlock_release(m_statelock);
	CloseHandle(m_waitstopevent);
	CloseHandle(m_waitpauseevent);

}

void D3DSoundout::run()
{
	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
	{
		LOGE("SetPriorityClass failed:%d\n", GetLastError());
		return ;
	}


	

	HRESULT hres;
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);

	if (FAILED(hres))
	{
		LOGE("CoInitializeEx error %#x", hres);
		return ;
	}
	UINT32 flags = 0;

	if (FAILED(hr = XAudio2Create(&pXAudio2, flags)))
	{
		LOGE("Failed to init XAudio2 engine: %#X\n", hr);
		pXAudio2 = NULL;
		cleanup();
		return;
	}


	if (FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasteringVoice)))
	{
		LOGE("Failed creating mastering voice: %#X\n", hr);
		cleanup();
		return ;
	}


	if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, &wfx, 0, 1.0f, &m_voiceContext)))
	{
		LOGE("Error %#X creating source voice", hr);
		pMasteringVoice->DestroyVoice();
		cleanup();
		return ;
	}
	bool exit = false;
	bool isendsound = false;
	while (!exit)
	{
		Message xMSG = getq();

		switch (xMSG.msgType)
		{
		case TTS_IFLY_MEDIA_DATA:
		{
			LOCK_HELPER_AUTO_HANDLER(m_statelock)
			Init_once();
			if (m_cur_state != PLAYIDE)
			{
				isendsound = false;
				TrySubmitAudioData();
			}
		}
		break;
		case TTS_PLAY_OVER:
		{
			LOCK_HELPER_AUTO_HANDLER(m_statelock)
			if (m_cur_state != PLAYIDE)
			{
				if (TrySubmitAudioData() == 0)
				{
					if (m_all_finish)
						isendsound = isPlayDone();
				}
				call_process();
			}
		}
		break;
		case TTS_IFLY_MEDIA_DATA_OVER:
		{
			//wait_end();
			//isendsound = true;
			m_all_finish = true;
		}
		break;
		case TTS_PLAY_STOP:
		{
			
			isendsound = true;
		}
		break;
		case TTS_PLAY_PAUSE:
		{
			{
				LOCK_HELPER_AUTO_HANDLER(m_statelock)
				if (m_cur_state == PLAYING)
				{
					pSourceVoice->Stop();
					m_cur_state = PLAYPAUSE;

				}
			}
			SetEvent(m_waitpauseevent);
			if (m_notify)m_notify->onPause();
		}
		break;
		case TTS_PLAY_RESUME:
		{
			{
				LOCK_HELPER_AUTO_HANDLER(m_statelock)
				if (m_cur_state == PLAYPAUSE)
				{
					m_cur_state = PLAYING;
					pSourceVoice->Start(0, 0);
				}
			}
			SetEvent(m_waitpauseevent);
			if (m_notify)m_notify->onResume();
		}
		break;	
		case _EXIT_THREAD_:
			
			isendsound = true;
			exit = true;
			break;
		default:
			;
		}
		if (isendsound)
		{
			{
				LOCK_HELPER_AUTO_HANDLER(m_statelock)
					//cleancache();
				if (m_cur_state != PLAYPAUSE)
					pSourceVoice->Stop(XAUDIO2_PLAY_TAILS);
				pSourceVoice->FlushSourceBuffers();
				m_cur_state = PLAYIDE;
				m_media_cache.Clean();
				isendsound = false;
				m_all_finish = false;
			}
			if (xMSG.msgType == TTS_PLAY_STOP)
				SetEvent(m_waitstopevent);
			if (xMSG.msgType != TTS_PLAY_STOP&&!exit&&m_notify)m_notify->onFinsih();
		}
	}

	brccache();
	pSourceVoice->DestroyVoice();

	// Cleanup XAudio2
	//

	// All XAudio2 interfaces are released when the engine is destroyed, but being tidy
	pMasteringVoice->DestroyVoice();
	cleanup();
	LOGW("exit sound  play");
}


void D3DSoundout::cleanup()
{
	SAFE_RELEASE(pXAudio2);
	CoUninitialize();
}

void D3DSoundout::call_process()
{
	if (m_all_finish)
	{
		double r = m_media_cache.Rate();
		if (m_notify)
			m_notify->onProcess(r);
	}
}

bool D3DSoundout::isPlayDone()
{
	XAUDIO2_VOICE_STATE state;
	pSourceVoice->GetState(&state);
	if (!state.BuffersQueued)
		return true;
	return false;
}


void D3DSoundout::waitEnd()
{
	XAUDIO2_VOICE_STATE state;
	for (;;)
	{
		pSourceVoice->GetState(&state);
		if (!state.BuffersQueued)
			break;
		WaitForSingleObject(m_voiceContext.hBufferEndEvent, 100);

	}
}

int  D3DSoundout::TrySubmitAudioData()
{
	while (1)
	{
		if (isFull())
			return -2;

		XAUDIO2_BUFFER buf = { 0 };
		BYTE *AudioB = NULL;
		int len = m_submit_manager.PlayData(&m_media_cache, AudioB);
		if (len > 0)
		{
			buf.AudioBytes = len;
			buf.pAudioData = AudioB;
			pSourceVoice->SubmitSourceBuffer(&buf);
		}
		else
			return len;
	}
}

bool D3DSoundout::isFull()
{
	XAUDIO2_VOICE_STATE state;
	pSourceVoice->GetState(&state);
	if (state.BuffersQueued == MAX_BUFFER_COUNT - 1)
		return true;
	return false;
}

void D3DSoundout::Init_once()
{
	if (m_cur_state == PLAYREADY)
	{
		m_cur_state = PLAYING;
		pSourceVoice->Start(0, 0);
		m_submit_manager.Init();
		m_all_finish = false;
	}
}

SOUNDPLAYER_API CSoundPlayer * Getplayer()
{
	return new D3DSoundout();
}

SOUNDPLAYER_API void Destoryplayer(CSoundPlayer *p)
{
	if (p)delete (p);
}

SoundBuffer::SoundBuffer()
{
	m_bfml = common_mutex_create_threadlock();
	m_totle_datalen = 0;
	m_post_datalen = 0;
}

SoundBuffer::~SoundBuffer()
{
	common_mutex_threadlock_release(m_bfml);
}

void SoundBuffer::Insert(unsigned char *buffer, int len)
{
	LOCK_HELPER_AUTO_HANDLER(m_bfml);
	m_buffer.insert(m_buffer.end(), buffer, buffer + len);
	m_totle_datalen += len;
}

int SoundBuffer::RemainLength()
{
	LOCK_HELPER_AUTO_HANDLER(m_bfml);
	return m_buffer.size();
}

bool SoundBuffer::GetRemove(unsigned char *buffer, int len)
{
	LOCK_HELPER_AUTO_HANDLER(m_bfml);
	if (len <= m_buffer.size())
	{
		memcpy(buffer, m_buffer.data(), len);
		m_post_datalen += len;
		m_buffer.erase(m_buffer.begin(), m_buffer.begin() + len);
		return true;
	}
	return false;
}

void SoundBuffer::Clean()
{
	LOCK_HELPER_AUTO_HANDLER(m_bfml);
	m_buffer.clear();
	m_totle_datalen = 0;
	m_post_datalen = 0;
}

double SoundBuffer::Rate()
{
	LOCK_HELPER_AUTO_HANDLER(m_bfml);
	return (double)m_post_datalen / (double)m_totle_datalen;
}

SubmitBufferManager::SubmitBufferManager() :m_buffers(NULL)
{
	m_buffers = new BYTE[MAX_BUFFER_COUNT][STREAMING_BUFFER_SIZE];
	currentDiskReadBuffer = 0;
}

SubmitBufferManager::~SubmitBufferManager()
{
	SAFE_DELETE_ARRAY(m_buffers);
}

void SubmitBufferManager::Init()
{
	currentDiskReadBuffer = 0;
	memset(m_buffers, 0x00, MAX_BUFFER_COUNT*STREAMING_BUFFER_SIZE*sizeof(BYTE));
}

int SubmitBufferManager::PlayData(SoundBuffer *sbufer, BYTE * &ptr)
{
	int remainlen = sbufer->RemainLength();
	UINT32 cbValid = min(STREAMING_BUFFER_SIZE, remainlen);
	if (cbValid == 0)return 0;

	if (!sbufer->GetRemove(m_buffers[currentDiskReadBuffer], cbValid))
		return -1;
	ptr = m_buffers[currentDiskReadBuffer];
	currentDiskReadBuffer = ++currentDiskReadBuffer%MAX_BUFFER_COUNT;
	return cbValid;
}
