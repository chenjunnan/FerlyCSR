#include <stddef.h>
#ifndef ROBOTMODULE_API_H
#define  ROBOTMODULE_API_H

#ifdef WIN32
#define ROBOTMODULE_API_EXPORTS
#ifdef ROBOTMODULE_API_EXPORTS
#define ROBOTMODULE_API __declspec(dllexport)
#else
#define ROBOTMODULE_API __declspec(dllimport)
#endif
#else
#define ROBOTMODULE_API
#endif // WIN32

#define ROBOTMODULE_API_M  __cdecl

//************************************
// Method:    funPostData
// FullName:  funPostData
// Access:    public 
// Returns:   void
// Qualifier: const char *strModuleName,		发送者模块名字
//			  const char *strResult,			消息
//			  unsigned long dResultLen,			长度
//			  const char *desModuleName,		接收者guid或者模块名或者空
//			  const unsigned char *strExtdata,	扩展（暂时未加）
//			  unsigned long dExtlen				扩展（暂时未加）
// Parameter: ROBOTMODULE_API_M * funPostData
//************************************
typedef void(ROBOTMODULE_API_M *fPostDataCB)(const char *strModuleName, const char *strResult, unsigned long dResultLen, 
	const char *desModuleName, const unsigned char *strExtdata, unsigned long  dExtlen);


extern "C"
{
	ROBOTMODULE_API bool ROBOTMODULE_API_M Init();

	ROBOTMODULE_API bool ROBOTMODULE_API_M Start();

	ROBOTMODULE_API bool ROBOTMODULE_API_M Stop();

	ROBOTMODULE_API const char* ROBOTMODULE_API_M GetModuleName();

	ROBOTMODULE_API const char* ROBOTMODULE_API_M GetModuleVersion();

	//************************************
	// Method:    RegisterHandleMessageType
	// FullName:  RegisterHandleMessageType
	// Access:    public 
	// Returns:   ROBOTMODULE_API_EXPORTS void
	// Qualifier: 
	// Parameter: const char * * handlelist //mssage type 二维数组 以双NULL结尾 单NULL 切分 最大长度8000 bytes
	//************************************
	ROBOTMODULE_API void ROBOTMODULE_API_M RegisterHandleMessageType(char *strHandleList);

	//************************************
	// Method:    SetProcessData
	// FullName:  SetProcessData
	// Access:    public 
	// Returns:   ROBOTMODULE_API int ROBOTMODULE_API_M
	// Qualifier:
	// Parameter: int dAppid						发送者appid 如果是模块间为0;
	// Parameter: int dMessageType					消息类型
	// Parameter: const char * strFrom				消息发送者，为模块名或者发送者guid
	// Parameter: const char * strMessage			消息体
	// Parameter: unsigned long dMessagelen			消息长度
	// Parameter: const unsigned char * strExtdata	扩展（暂时未加）
	// Parameter: unsigned long dExtlen				扩展（暂时未加）
	//************************************
	// 增加获取模块状态、信息、接口
	ROBOTMODULE_API int ROBOTMODULE_API_M SetProcessData(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata = NULL, unsigned long  dExtlen = 0);

	ROBOTMODULE_API void ROBOTMODULE_API_M SetPostFuntion(fPostDataCB ptr);


}

#endif
