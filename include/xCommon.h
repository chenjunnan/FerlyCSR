#include <string>
#include <vector>
#ifndef XCOMMON_H
#define  XCOMMON_H

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 XCOMMON_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// XCOMMON_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef WIN32
#ifdef XCOMMON_EXPORTS
#define XCOMMON_API __declspec(dllexport)
#else
#define XCOMMON_API __declspec(dllimport)
#endif
#else
	#define XCOMMON_API
#endif // WIN32
#define XCOMMON_API_M  __cdecl

#define  _EXIT_THREAD_ -2
typedef void(__stdcall *thread_function)(void * param);

typedef void* CHANDLE;

typedef std::vector<unsigned char> VEC_BYTE;

typedef void(__stdcall *trace_funtion)(const char *);

const char  * const MAIN_LOG_NAME = "robot";

enum ENUM_LOG_LEVEL
{
	LOG_LEVEL_TRACE = 0,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL,
};

class thread_interface
{
public:
	thread_interface(){};
	virtual void run() = 0;
	virtual~thread_interface(){};
};


//线程接口//
extern "C"
{
	//************************************
	// Method:    开启线程
	// FullName:  common_thread_create
	// Access:    public 
	// Returns:   bool 
	// Qualifier:
	// Parameter: thread_function funtction_address 线程函数
	// Parameter: void * thread_param 线程参数
	// Parameter: CHANDLE * threadid 返回线程句柄
	//************************************
	XCOMMON_API bool XCOMMON_API_M common_thread_create(thread_interface *thread_param, CHANDLE *threadid);
	//************************************
	// Method:    创建等待线程
	// FullName:  common_thread_create
	// Access:    public 
	// Returns:   bool 
	// Qualifier:
	// Parameter: thread_function funtction_address 线程函数
	// Parameter: void * thread_param 线程参数
	// Parameter: CHANDLE * threadid 返回线程句柄
	//************************************
	XCOMMON_API bool XCOMMON_API_M common_thread_create_wait(thread_interface *thread_param, CHANDLE *threadid);
	//************************************
	// Method:    开启等待线程
	// FullName:  common_thread_create
	// Access:    public 
	// Returns:   bool 
	// Qualifier:
	// Parameter: thread_function funtction_address 线程函数
	// Parameter: void * thread_param 线程参数
	// Parameter: CHANDLE * threadid 返回线程句柄
	//************************************
	XCOMMON_API bool XCOMMON_API_M common_thread_active_wait(CHANDLE threadid);
	//************************************
	// Method:    向线程传递消息
	// FullName:  common_thread_post_message
	// Access:    public 
	// Returns:   XCOMMON_API bool XCOMMON_API_M
	// Qualifier:
	// Parameter: CHANDLE threadid 线程句柄
	// Parameter: int dMessageType 消息类型
	// Parameter: const char * strMessage 字符消息
	// Parameter: void * ptr 对象消息
	//************************************
	XCOMMON_API bool XCOMMON_API_M common_thread_post_message(CHANDLE threadid, int dMessageType, const char *strMessage = NULL, void *ptr = NULL);

	//************************************
	// Method:    传递及时消息
	// FullName:  common_thread_rpost_message
	// Access:    public 
	// Returns:   XCOMMON_API bool XCOMMON_API_M
	// Qualifier:
	// Parameter: CHANDLE threadid 线程句柄
	// Parameter: int dMessageType 消息类型
	// Parameter: const char * strMessage 字符消息
	// Parameter: void * ptr 对象消息
	//************************************
	XCOMMON_API bool XCOMMON_API_M common_thread_rpost_message(CHANDLE threadid, int dMessageType, const char *strMessage = NULL, void *ptr = NULL);


	//************************************
	// Method:    线程获取消息
	// FullName:  common_thread_get_message
	// Access:    public 
	// Returns:   XCOMMON_API bool XCOMMON_API_M
	// Qualifier:
	// Parameter: CHANDLE threadid 线程句柄
	// Parameter: int * dMessageType 消息类型
	// Parameter: std::string * strMessage  字符消息
	// Parameter: void * * ptr 对象消息
	// Parameter: int waittime 等待超时 -1为永久
	//************************************
	XCOMMON_API bool XCOMMON_API_M common_thread_get_message(CHANDLE threadid, int *dMessageType, std::string *strMessage, void **ptr = NULL, int waittime = -1);

	//************************************
	// Method:    获取当前线程消息
	// FullName:  common_thread_get_currentthread_message
	// Access:    public 
	// Returns:   XCOMMON_API bool XCOMMON_API_M
	// Qualifier:
	// Parameter: int * dMessageType 消息类型
	// Parameter: std::string * strMessage  字符消息
	// Parameter: void * * ptr 对象消息
	// Parameter: int waittime 等待超时 -1为永久
	//************************************
	XCOMMON_API bool XCOMMON_API_M common_thread_get_currentthread_message(int *dMessageType, std::string *strMessage, void **ptr = NULL, int waittime = -1);


	//************************************
	// Method:    发送_EXIT_THREAD_退出线程并等待线程结束
	// FullName:  common_thread_wait_quit
	// Access:    public 
	// Returns:   XCOMMON_API bool XCOMMON_API_M
	// Qualifier:
	// Parameter: CHANDLE threadid 线程句柄
	// Parameter: int timeout 等待超时 -1为永久
	//************************************
	XCOMMON_API bool XCOMMON_API_M common_thread_wait_quit(CHANDLE threadid, int timeout = -1);

	//************************************
	// Method:    等待线程结束
	// FullName:  common_thread_wait_quit
	// Access:    public 
	// Returns:   XCOMMON_API bool XCOMMON_API_M
	// Qualifier:
	// Parameter: CHANDLE threadid 线程句柄
	// Parameter: int timeout 等待超时 -1为永久
	//************************************
	XCOMMON_API bool XCOMMON_API_M common_thread_wait(CHANDLE threadid, int timeout = -1);

	//************************************
	// Method:    清理线程消息
	// FullName:  common_thread_clean_message
	// Access:    public 
	// Returns:   XCOMMON_API bool XCOMMON_API_M
	// Qualifier:
	// Parameter: CHANDLE threadid 线程句柄
	//************************************
	XCOMMON_API bool XCOMMON_API_M common_thread_clean_message(CHANDLE threadid);

	XCOMMON_API bool XCOMMON_API_M common_thread_state(CHANDLE threadid);

	XCOMMON_API int XCOMMON_API_M common_thread_getid(CHANDLE threadid);

	//************************************
	// Method:    释放线程资源
	// FullName:  common_thread_release
	// Access:    public 
	// Returns:   XCOMMON_API void XCOMMON_API_M
	// Qualifier:
	// Parameter: CHANDLE threadid 线程句柄
	//************************************
	XCOMMON_API void XCOMMON_API_M common_thread_release(CHANDLE threadid);

	//**************************************************
	//延时
	//common_thread_sleep
	//ms 单位毫秒
	//**************************************************
	XCOMMON_API void XCOMMON_API_M common_thread_sleep(long ms);
}

//http接口//
extern "C"
{
	//************************************
	// Method:    获取http对象
	// FullName:  common_http_create
	// Access:    public 
	// Returns:   XCOMMON_API CHANDLE XCOMMON_API_M
	// Qualifier:
	//************************************
	XCOMMON_API CHANDLE XCOMMON_API_M common_http_create();

	//************************************
	// Method:    添加头部信息
	// FullName:  common_http_set_http_header
	// Access:    public 
	// Returns:   XCOMMON_API bool XCOMMON_API_M
	// Qualifier: 发送完清除
	// Parameter: CHANDLE httpid http句柄
	// Parameter: const char * strHeadName 头部字段
	// Parameter: const char * strValue 头部值
	//************************************
	XCOMMON_API bool XCOMMON_API_M common_http_set_header(CHANDLE httpid, const char *strHeadName, const char *strValue);


	//************************************
	// Method:    添加用户信息
	// FullName:  common_http_set_user_agent
	// Access:    public 
	// Returns:   XCOMMON_API bool XCOMMON_API_M
	// Qualifier: 不清除
	// Parameter: CHANDLE httpid
	// Parameter: const char * lpAgent 浏览器中获取
	//************************************
	XCOMMON_API bool XCOMMON_API_M common_http_set_user_agent(CHANDLE httpid, const char *lpAgent);

	XCOMMON_API bool XCOMMON_API_M common_http_set_log(CHANDLE httpid, bool	ignorelog);

	XCOMMON_API bool XCOMMON_API_M common_http_cancle(CHANDLE httpid);

	XCOMMON_API bool  XCOMMON_API_M  common_http_get(CHANDLE httpid, const char *url, VEC_BYTE *result, int timeout_s = 5);

	XCOMMON_API bool  XCOMMON_API_M common_http_post(CHANDLE httpid, const char *url, VEC_BYTE *postvalue, VEC_BYTE *result, int timeout_s = 5);

	//**************************************************
	//释放http对象
	//common_http_release
	//**************************************************
	XCOMMON_API void  XCOMMON_API_M common_http_release(CHANDLE httpid);
}

//日志接口//
extern "C"
{
	//**************************************************
	//开启某模块日志
	//common_http_create
	//**************************************************
	XCOMMON_API bool XCOMMON_API_M common_log_create(const char *name);
	//**************************************************
	//停止某模块日志
	//common_log_delete
	//**************************************************
	XCOMMON_API bool XCOMMON_API_M common_log_delete(const char *name);


	XCOMMON_API void common_log(const char *name, ENUM_LOG_LEVEL level, const char *file, const char *funtion, const  unsigned long line, const char *str, ...);

	//**************************************************
	//配置日志模块
	//common_log_config
	//display 是否显示 outfile 是否输出文件
	//**************************************************
	XCOMMON_API void XCOMMON_API_M common_log_config(const char *name, bool display = true, bool outfile = false,int loglevel=0);//TODO

	XCOMMON_API void XCOMMON_API_M common_log_exit();
}


//时间接口//
extern "C"
{
	XCOMMON_API CHANDLE XCOMMON_API_M common_timer_create();

	XCOMMON_API void XCOMMON_API_M common_timer_update_time(CHANDLE mtime);

	XCOMMON_API bool XCOMMON_API_M common_timer_isreach_time_ms(CHANDLE mtime, long time_v);

	XCOMMON_API void XCOMMON_API_M common_timer_release(CHANDLE mtime);

	XCOMMON_API struct tm XCOMMON_API_M common_time_2date(time_t t,int * ms);

	XCOMMON_API void XCOMMON_API_M common_get_cur_time_string(std::string* out);

	XCOMMON_API void XCOMMON_API_M common_get_cur_timedate_string(std::string* out);

	XCOMMON_API void XCOMMON_API_M common_get_cur_timedate_filename_string(std::string *out);

	XCOMMON_API unsigned long   XCOMMON_API_M common_get_cur_time_stampms();

	XCOMMON_API unsigned long long  XCOMMON_API_M common_get_longcur_time_stampms();

	XCOMMON_API unsigned long long  XCOMMON_API_M common_get_cur_time_stamps();

	XCOMMON_API bool XCOMMON_API_M common_issame_day(time_t t1, time_t t2);

	XCOMMON_API int  XCOMMON_API_M common_get_ms();

}

//配置接口//
extern "C"
{
	//一般dll不需要调用此接口
	XCOMMON_API bool XCOMMON_API_M common_config_set_path(const char* path);

	XCOMMON_API void XCOMMON_API_M common_config_update_persistence();

	XCOMMON_API bool XCOMMON_API_M common_config_get_field_string(const char*section, const char* field, std::string *out);

	XCOMMON_API bool XCOMMON_API_M common_config_get_field_int(const char* section, const char* field, int *out);

	XCOMMON_API void XCOMMON_API_M common_config_set_field(const char* section, const char* field, const char* data);
}

//同步接口//
extern "C"
{
	XCOMMON_API CHANDLE XCOMMON_API_M common_mutex_create_multisignal(int initc=1,int max = 1);

	XCOMMON_API void XCOMMON_API_M common_mutex_multisignal_post(CHANDLE mid);

	XCOMMON_API bool XCOMMON_API_M common_mutex_multisignal_wait(CHANDLE mid, int wait_time = -1);

	XCOMMON_API void XCOMMON_API_M common_mutex_multisignal_release(CHANDLE mid);



	XCOMMON_API CHANDLE XCOMMON_API_M common_mutex_create_threadlock();

	XCOMMON_API void XCOMMON_API_M common_mutex_threadlock_lock(CHANDLE mid);

	XCOMMON_API void XCOMMON_API_M common_mutex_threadlock_unlock(CHANDLE mid);

	XCOMMON_API void XCOMMON_API_M common_mutex_threadlock_release(CHANDLE mid);
}

//常用函数//
extern "C"
{
	XCOMMON_API unsigned int common_get_self_threadID();

	XCOMMON_API void  common_gbk_2utf8(const std::string *strGBK, std::string *strUTF8);

	XCOMMON_API void  common_utf8_2gbk(const std::string *strUTF8, std::string *strGBK);

	XCOMMON_API void  common_itoa_x(long i, std::string* out);

	XCOMMON_API void common_base64_encode_string(unsigned char *buffer, unsigned long inputlen, std::string*out);

	XCOMMON_API unsigned int common_base64_decode_string(std::string *input, VEC_BYTE*buffer);

	XCOMMON_API void  common_get_guid(std::string *out);

	//删除左右空格
	XCOMMON_API void common_trim(std::string* str);

	XCOMMON_API bool common_check_file_exist(const char* path);

	XCOMMON_API bool common_create_dir(const char* path);

	//修正为标准路径
	XCOMMON_API void common_fix_path(std::string *path);

	XCOMMON_API bool  common_isDirectory(const char* path);

	XCOMMON_API void common_get_current_dir(std::string *path);

	XCOMMON_API void common_urldecode(std::string* str, std::string *out);

	XCOMMON_API void common_urlencode(const std::string* str, std::string *out);

	XCOMMON_API int common_get_robot_serialID(std::string *out);
	//	netmac  disk  board  cpu  bios  boardtype  mac serial
	XCOMMON_API int common_get_hardware_serial(std::string devicetype, std::string *out);
}

typedef intptr_t CHANDLESOCKET;

class xSocketUserInterface
{
public:
	virtual bool GetInfo(std::string &ip, int &peerport,int &localport, CHANDLESOCKET &handle) = 0;
	virtual int  Write(unsigned char *buffer, int len, int timeout) = 0;
	virtual int  Close() = 0;
	virtual ~xSocketUserInterface(){}
};


class xSocketCallBackInterFace
{
public:
	xSocketCallBackInterFace(){}
	virtual void onConnect(xSocketUserInterface* handle) = 0;
	virtual void onAccept(xSocketUserInterface* handle) = 0;
	virtual void onRead(xSocketUserInterface* handle, VEC_BYTE data) = 0;
	virtual void onWrite(xSocketUserInterface* handle, int error) = 0;
	virtual void onClose(xSocketUserInterface* handle) = 0;
	virtual void onEvent(int error) = 0;
	virtual ~xSocketCallBackInterFace(){}
};
//网络相关函数//


class SocketServerInterface
{
public:
	virtual int Listen(int port) = 0;
	virtual int Connect(const char *ip, int port, int timeout) = 0;
	virtual void Close() = 0;
	virtual ~SocketServerInterface(){}
};

extern "C"
{
	XCOMMON_API SocketServerInterface * XCOMMON_API_M common_socket_create(xSocketCallBackInterFace *);

	XCOMMON_API  void  XCOMMON_API_M common_socket_destory(SocketServerInterface *);
}




class FTPInterface
{
public:
	virtual void	setlog(bool cb)=0;
	virtual void    cancleFTP()=0;
	virtual void    getFTPinfo(long long *uptotle, long long *up)=0;
	virtual int		getFTP(const char* url, const char* filepath, int port = 21, int timeouts = 30, const char* username = "anonymous", const char* password = "")=0;
	virtual int		putFTP(const char* url, const char* filepath, int port = 21, int timeouts = 30, const char* username = "anonymous", const char* password = "")=0;
	virtual ~FTPInterface(){}
};

extern "C"
{
	XCOMMON_API FTPInterface * XCOMMON_API_M common_ftp_create();

	XCOMMON_API  void  XCOMMON_API_M common_ftp_destory(FTPInterface *);
}

#ifdef WIN32
#define LOG_FORMAT(name, level, file,function,line, logformat, ...) \
{do{ \
	common_log(name,level,file,function,line,logformat, ##__VA_ARGS__);\
} while (0);}
#else
#endif

#define LOGT( fmt, ...) LOG_FORMAT(MAIN_LOG_NAME, LOG_LEVEL_TRACE, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGD( fmt, ...) LOG_FORMAT(MAIN_LOG_NAME, LOG_LEVEL_DEBUG, __FILE__,__FUNCTION__,  __LINE__, fmt, ##__VA_ARGS__)
#define LOGI( fmt, ...) LOG_FORMAT(MAIN_LOG_NAME, LOG_LEVEL_INFO, __FILE__,__FUNCTION__,  __LINE__, fmt, ##__VA_ARGS__)
#define LOGW( fmt, ...) LOG_FORMAT(MAIN_LOG_NAME,LOG_LEVEL_WARN, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGE( fmt, ...) LOG_FORMAT(MAIN_LOG_NAME, LOG_LEVEL_ERROR, __FILE__,__FUNCTION__,  __LINE__, fmt, ##__VA_ARGS__)
#define LOGF( fmt, ...) LOG_FORMAT(MAIN_LOG_NAME, LOG_LEVEL_FATAL, __FILE__,__FUNCTION__,  __LINE__, fmt, ##__VA_ARGS__)

#endif