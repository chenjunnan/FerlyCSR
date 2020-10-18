// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� MEMORYSWAPER_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// MEMORYSWAPER_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef MEMORYSWAPER_EXPORTS
#define MEMORYSWAPER_API __declspec(dllexport)
#else
#define MEMORYSWAPER_API __declspec(dllimport)
#endif
typedef void *HANDLE;
extern "C"
{
	MEMORYSWAPER_API bool Processon_Memory_Create(const char *memory_name, unsigned int maxsize, unsigned int count);

	MEMORYSWAPER_API HANDLE Processon_Memory_Open(const char *memory_name,const char *processon_name);

	MEMORYSWAPER_API bool Processon_Memory_Write(HANDLE pmhandle, const unsigned char *buffer, unsigned int datalen);

	MEMORYSWAPER_API int Processon_Memory_Read(HANDLE pmhandle, unsigned char *buffer, unsigned int *datalen);

	MEMORYSWAPER_API void Processon_Memory_Close(HANDLE pmhandle);

	MEMORYSWAPER_API void Processon_Memory_Destory(const char *memory_name);
}
