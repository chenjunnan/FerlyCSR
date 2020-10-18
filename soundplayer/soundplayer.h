#pragma once
// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 SOUNDPLAYER_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// SOUNDPLAYER_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef SOUNDPLAYER_EXPORTS
#define SOUNDPLAYER_API __declspec(dllexport)
#else
#define SOUNDPLAYER_API __declspec(dllimport)
#endif

// 此类是从 soundplayer.dll 导出的
class SoundStateNotify
{
public :
	virtual void onResume() = 0;
	virtual void onPause()=0;
	virtual void onFinsih()=0;
	virtual void onProcess(double rate) = 0;
	virtual ~SoundStateNotify(){};
};

class  CSoundPlayer {
public:
	virtual void set_notify(SoundStateNotify*_notify)=0;
	virtual int init(short nChannels,
		int       nSamplesPerSec,    /* sample rate */
		int       nAvgBytesPerSec,   /* for buffer estimation */
		short        nBlockAlign,       /* block size of data */
	short        wBitsPerSample /* number of bits per sample of mono data */) = 0;
	virtual bool reset() = 0;
	virtual bool stop() = 0;
	virtual bool resume() = 0;
	virtual bool pause() = 0;
	virtual bool isfinsh() = 0;
	virtual void brccache()=0;
	virtual void quit()=0;
	virtual void sound(unsigned char *data, int len)=0;
	virtual void endsound()=0;
	virtual ~CSoundPlayer(void);
};

SOUNDPLAYER_API CSoundPlayer *Getplayer();

SOUNDPLAYER_API void Destoryplayer(CSoundPlayer *);