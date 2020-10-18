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
// Qualifier: const char *strModuleName,		������ģ������
//			  const char *strResult,			��Ϣ
//			  unsigned long dResultLen,			����
//			  const char *desModuleName,		������guid����ģ�������߿�
//			  const unsigned char *strExtdata,	��չ����ʱδ�ӣ�
//			  unsigned long dExtlen				��չ����ʱδ�ӣ�
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
	// Parameter: const char * * handlelist //mssage type ��ά���� ��˫NULL��β ��NULL �з� ��󳤶�8000 bytes
	//************************************
	ROBOTMODULE_API void ROBOTMODULE_API_M RegisterHandleMessageType(char *strHandleList);

	//************************************
	// Method:    SetProcessData
	// FullName:  SetProcessData
	// Access:    public 
	// Returns:   ROBOTMODULE_API int ROBOTMODULE_API_M
	// Qualifier:
	// Parameter: int dAppid						������appid �����ģ���Ϊ0;
	// Parameter: int dMessageType					��Ϣ����
	// Parameter: const char * strFrom				��Ϣ�����ߣ�Ϊģ�������߷�����guid
	// Parameter: const char * strMessage			��Ϣ��
	// Parameter: unsigned long dMessagelen			��Ϣ����
	// Parameter: const unsigned char * strExtdata	��չ����ʱδ�ӣ�
	// Parameter: unsigned long dExtlen				��չ����ʱδ�ӣ�
	//************************************
	// ���ӻ�ȡģ��״̬����Ϣ���ӿ�
	ROBOTMODULE_API int ROBOTMODULE_API_M SetProcessData(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata = NULL, unsigned long  dExtlen = 0);

	ROBOTMODULE_API void ROBOTMODULE_API_M SetPostFuntion(fPostDataCB ptr);


}

#endif
