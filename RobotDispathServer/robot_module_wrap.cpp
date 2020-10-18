#include "robot_module_wrap.h"
#include "xCommon.h"
#include "robot_module_manager.h"



class robot_module_manager;
robot_module_wrap::robot_module_wrap()
{
	m_moduleinstance = NULL;
	m_Init = NULL;
	m_Start = NULL;
	m_Stop = NULL;
	m_GetModuleName = NULL;
	m_GetModuleVersion = NULL;
	m_RegisterHandleMessageType = NULL;
	m_SetProcessData = NULL;
	m_SetPostFuntion = NULL;
	m_run = false;
}



bool robot_module_wrap::Load(std::string strmodulepath)
{
	close();
	std::string path;
	common_get_current_dir(&path);
	common_fix_path(&path);
	path.append(strmodulepath);
	if (!common_check_file_exist(path.c_str()))
	{
		LOGE("Module file %s missing!!!!", strmodulepath.c_str());
		return false;
	}
	LOGD("start load %s", strmodulepath.c_str());
	//m_moduleinstance = LoadLibrary(strmodulepath.c_str());
	m_moduleinstance=LoadLibraryEx(strmodulepath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (m_moduleinstance){
		m_Init = (_Init)GetProcAddress(m_moduleinstance, "Init");
		m_Start = (_Start)GetProcAddress(m_moduleinstance, "Start");
		m_Stop = (_Stop)GetProcAddress(m_moduleinstance, "Stop");
		m_GetModuleName = (_GetModuleName)GetProcAddress(m_moduleinstance, "GetModuleName");
		m_GetModuleVersion = (_GetModuleName)GetProcAddress(m_moduleinstance, "GetModuleVersion");
		m_RegisterHandleMessageType = (_RegisterHandleMessageType)GetProcAddress(m_moduleinstance, "RegisterHandleMessageType");
		m_SetProcessData = (_SetProcessData)GetProcAddress(m_moduleinstance, "SetProcessData");
		m_SetPostFuntion = (_SetPostFuntion)GetProcAddress(m_moduleinstance, "SetPostFuntion");
		if (!(m_Init&&m_Start&&m_Stop&&m_GetModuleName&&m_RegisterHandleMessageType&&m_SetProcessData&&m_SetPostFuntion))
		{
			LOGE("Module %s unknow interface or miss interface ,load failed!!!!", strmodulepath.c_str());
			close();
			return false;
		}
		modulepath = strmodulepath;
		modulename = getModuleName();
		LOGD("load %s finish", strmodulepath.c_str());
		return true;
	}
	else{
		LOGE("Module file %s load failed %d !!!!", strmodulepath.c_str(),GetLastError());
	}
	return false;
}

bool robot_module_wrap::Init()
{
	LOGD("Module (%s) call Init...", modulepath.c_str());
	m_SetPostFuntion(robot_module_manager::ModulePostMessage);
	bool ret = m_Init();
	LOGD("Module (%s) call Init .ret:%d.", modulepath.c_str(), ret);
	return ret;
	
}

bool robot_module_wrap::Start()
{
	LOGD("Module (%s) call Start...", modulepath.c_str());
	bool ret = m_Start();
	m_run = ret;
	LOGD("Module (%s) call Start .ret:%d.", modulepath.c_str(), ret);
	return ret;
}

bool robot_module_wrap::Stop()
{
	LOGD("Module (%s) call Stop...", modulepath.c_str());
	bool ret = m_Stop();
	m_run = false;
	LOGD("Module (%s) call Stop .ret:%d.", modulepath.c_str(), ret);
	return ret;
}

std::string robot_module_wrap::getModuleName()
{
	LOGD("Module (%s) call getModuleName...", modulepath.c_str());
	std::string temp;
	temp = m_GetModuleName();
	return temp;
}

std::string robot_module_wrap::getModuleVersion()
{
	LOGD("Module (%s) call getModuleVersion...", modulepath.c_str());
	std::string temp;
	if (m_GetModuleVersion)
		temp = m_GetModuleVersion();
	return temp;
}

std::vector<std::string > robot_module_wrap::getHandleList()
{
	std::vector<std::string >  temp;
	LOGD("Module (%s) call getModuleName...", modulepath.c_str());

	char *bufferptr = new char[8000];
	memset(bufferptr, 0x00, 8000);
	m_RegisterHandleMessageType(bufferptr);

	char *posptr = bufferptr;

	while ((*posptr) != NULL)
	{
		int message_list_typelen = strlen(posptr);

		temp.push_back(posptr);

		posptr = posptr + message_list_typelen + 1;
	}
	delete bufferptr;

	return temp;
}

int robot_module_wrap::ProcessData(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen,
	const unsigned char *strExtdata /*= NULL*/, unsigned long dExtlen /*= 0*/)
{
	if (!m_run) return -1;
	int ret = m_SetProcessData(dAppid, dMessageType, strFrom, strMessage, dMessagelen, strExtdata, dExtlen);
	return ret;
}

void robot_module_wrap::close()
{
	m_run = false;
	if (m_moduleinstance)
		FreeLibrary(m_moduleinstance);
	m_moduleinstance = NULL;
	m_Init = NULL;
	m_Start = NULL;
	m_Stop = NULL;
	m_GetModuleName = NULL;
	m_GetModuleVersion = NULL;
	m_RegisterHandleMessageType = NULL;
	m_SetProcessData = NULL;
	m_SetPostFuntion = NULL;
}

robot_module_wrap::~robot_module_wrap()
{
	close();
}
