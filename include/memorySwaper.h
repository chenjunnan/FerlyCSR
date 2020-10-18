// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 MEMORYSWAPER_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// MEMORYSWAPER_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
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
