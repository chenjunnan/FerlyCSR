#pragma once
#include <map>
#include <string>
#include <vector>
#include <Windows.h>
#include <WinDef.h>
#include "RobotModule_interface.h"

typedef bool (ROBOTMODULE_API_M *_Init)();
typedef bool (ROBOTMODULE_API_M *_Start)();
typedef bool (ROBOTMODULE_API_M *_Stop)();
typedef const char* (ROBOTMODULE_API_M *_GetModuleName)();
typedef void (ROBOTMODULE_API_M *_RegisterHandleMessageType)(char *strHandleList);
typedef int  (ROBOTMODULE_API_M *_SetProcessData)(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata, unsigned long  dExtlen);
typedef void (ROBOTMODULE_API_M *_SetPostFuntion)(fPostDataCB ptr);

class robot_module_manager;
class robot_module_wrap
{
public:
	robot_module_wrap();
	bool Load(std::string modulepath);
	bool Init();
	bool Start();
	bool Stop();
	std::string getModuleName();
	std::string getModuleVersion();
	std::vector<std::string > getHandleList();
	int ProcessData(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata = NULL, unsigned long  dExtlen = 0);
	virtual ~robot_module_wrap();
private:
	void close();
	std::string modulepath;
	std::string modulename;
	HINSTANCE     m_moduleinstance;
	_Init    m_Init;
	_Start   m_Start;
	_Stop    m_Stop;
	_GetModuleName m_GetModuleName;
	_GetModuleName m_GetModuleVersion;
	_RegisterHandleMessageType m_RegisterHandleMessageType;
	_SetProcessData m_SetProcessData;
	_SetPostFuntion m_SetPostFuntion;
	bool m_run;
};