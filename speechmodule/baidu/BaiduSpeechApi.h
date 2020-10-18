#pragma  once
// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� BAIDUSPEECHAPI_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// BAIDUSPEECHAPI_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef BAIDUSPEECHAPI_EXPORTS
#define BAIDUSPEECHAPI_API __declspec(dllexport)
#else
#define BAIDUSPEECHAPI_API __declspec(dllimport)
#endif


struct TTSPCMDATA
{
	unsigned char *data=0;
	unsigned long long len = 0;
};

// �����Ǵ� BaiduSpeechApi.dll ������
class BAIDUSPEECHAPI_API BaiduSpeechApi {
public:
	virtual ~BaiduSpeechApi(void) {}
	virtual int BaiduTTS(const char* text, TTSPCMDATA *outPCM,const char* per = "4", const char*  spd = "5", const char*  pit = "5")=0;
	virtual void CancelBTTS() = 0;
	// TODO:  �ڴ�������ķ�����
};

extern "C"
{
	BAIDUSPEECHAPI_API BaiduSpeechApi *CreateSpeechEngine(const char *AppKey,const char *SecretKey,const char * ClientID);
	BAIDUSPEECHAPI_API void DestorySpeechEngine(BaiduSpeechApi *engine);
}
