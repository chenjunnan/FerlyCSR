#pragma once
// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� SOUNDPLAYER_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// SOUNDPLAYER_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef SOUNDPLAYER_EXPORTS
#define SOUNDPLAYER_API __declspec(dllexport)
#else
#define SOUNDPLAYER_API __declspec(dllimport)
#endif

// �����Ǵ� soundplayer.dll ������
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