#pragma  once
// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 BAIDUSPEECHAPI_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// BAIDUSPEECHAPI_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
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

// 此类是从 BaiduSpeechApi.dll 导出的
class BAIDUSPEECHAPI_API BaiduSpeechApi {
public:
	virtual ~BaiduSpeechApi(void) {}
	virtual int BaiduTTS(const char* text, TTSPCMDATA *outPCM,const char* per = "4", const char*  spd = "5", const char*  pit = "5")=0;
	virtual void CancelBTTS() = 0;
	// TODO:  在此添加您的方法。
};

extern "C"
{
	BAIDUSPEECHAPI_API BaiduSpeechApi *CreateSpeechEngine(const char *AppKey,const char *SecretKey,const char * ClientID);
	BAIDUSPEECHAPI_API void DestorySpeechEngine(BaiduSpeechApi *engine);
}
