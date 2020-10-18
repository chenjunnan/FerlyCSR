#include "robot_main_app.h"
#include <shlwapi.h>



bool close_service_flag = false;

HANDLE appInstance;

extern HWND hwnd;

FILE *m_stream=NULL;

robot_main_app *robot_main_app::m_instance = NULL;

HANDLE  m_event_wait=NULL;


robot_main_app::robot_main_app()
{
	close_flag = false;
	m_ok = true;
	m_stream = NULL;
	m_event_wait = CreateEvent(NULL, FALSE, FALSE, NULL);
}

BOOL WINAPI consolehandler(DWORD ce)
{
	
	switch (ce)
	{
	case  CTRL_C_EVENT:
		break;
	case CTRL_BREAK_EVENT:
		break;
	case CTRL_LOGOFF_EVENT:
		break;
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	{
		LOGW("Main catch %d", ce);
		robot_main_app::getinstance()->close_service();
	}
		break;
	}
	return true;
}



robot_main_app* robot_main_app::getinstance()
{
	if (m_instance == NULL)
	{
		if (m_instance == NULL)
			m_instance = new robot_main_app;
	}
	return  m_instance;
}


void robot_main_app::run()
{
	if (!init())
		return;
	robot_message_handler::getptr()->start();
	robot_message_dispatch::getptr()->start();
	robot_module_manager::get_Instance()->Init();
	robot_module_manager::get_Instance()->Start();
	m_ok=ServerInterface::getPtr()->set_address(NULL, 50316);
	if (!m_ok)MessageBox(NULL, "端口监听失败（netstat -aon|findstr \"50316\"）！", "上海方立科技", MB_OK);
	else{ServerInterface::getPtr()->start(); m_ok = m_input.start();}
	wait();
}



bool robot_main_app::init()
{
	SECURITY_ATTRIBUTES sa;

	sa.bInheritHandle = TRUE;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	appInstance = CreateMutex(&sa, FALSE, "ferly_service_mutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(NULL, "服务已在运行已经启动，不能重复运行！", "上海方立科技", MB_OK);
		CloseHandle(appInstance);
		appInstance = NULL;
		return false;
	}
	hwnd = FindWindow("ConsoleWindowClass", NULL);
	if (hwnd)
	{
		ShowOwnedPopups(hwnd, SW_HIDE);
		ShowWindow(hwnd, SW_HIDE);
	}
	

	if (SetConsoleCtrlHandler(consolehandler, true) == false)
	{
		MessageBox(NULL, "发生系统系统错误！", "上海方立科技", MB_OK);
		return false;
	}
	int loglevel = 0;
	char exeFullPath[MAX_PATH]; // MAX_PATH在WINDEF.h中定义了，等于260  
	memset(exeFullPath, 0, MAX_PATH);

	GetModuleFileName(NULL, exeFullPath, MAX_PATH);
	std::string exepath = exeFullPath;
	if (exepath.rfind("\\") != -1)
	{
		exepath = exepath.substr(0, exepath.rfind("\\"));
		common_fix_path(&exepath);
		exepath.append("robotserviceconfig.ini");
	}
	else
		exepath = "robotserviceconfig.ini";
	if (!common_config_set_path(exepath.c_str()))
	{
		MessageBox(NULL, "丢失配置文件！", "上海方立科技", MB_OK);
		return false;
	}
	
	common_config_get_field_int("Setting", "log", &loglevel);
	common_log_config(MAIN_LOG_NAME, true, true, loglevel);
	if (hwnd&&loglevel==0)
	{
		ShowOwnedPopups(hwnd, SW_SHOW);
		ShowWindow(hwnd, SW_SHOW);
	}

	return true;
}

void robot_main_app::main_close()
{
	m_input.order_c = 'q';
	SetEvent(m_event_wait);
}

void robot_main_app::wait()
{
	while (m_ok)
	{
		DWORD	waitret=WaitForSingleObject(m_event_wait, -1);
		if (waitret != WAIT_OBJECT_0)continue;
		if (m_input.order_c == 'q')
			break;
		if (m_input.order_c == 'm')
			robot_module_manager::get_Instance()->ShowModuleList();
		if (m_input.order_c == 'v')
			printf("robot service version : %s.(build :%s %s).\n", robot_module_manager::get_version().c_str(), __DATE__, __TIME__);
		m_input.order_c = 0;
	}
	close_service();
}

void robot_main_app::close_service()
{
	if (!close_flag)
	{
		close_flag = true;
		robot_module_manager::get_Instance()->ShutDownNotify();
		common_thread_sleep(300);

		ServerInterface::getPtr()->quit();
		robot_module_manager::get_Instance()->Stop();
		robot_message_dispatch::getptr()->quit();
		robot_message_handler::getptr()->quit();
		common_log_exit();
	}
}

robot_main_app::~robot_main_app()
{
}

void printf_hardware()
{
	std::string out;
	int ret = common_get_hardware_serial("netmac", &out);	printf("ret:%d\n", ret);
	if (ret)printf("netmac:%s\n", out.c_str());
	ret = common_get_hardware_serial("disk", &out);	printf("ret:%d\n", ret);
	if (ret)printf("%s\n", out.c_str());
	ret = common_get_hardware_serial("board", &out);	printf("ret:%d\n", ret);
	if (ret)printf("%s\n", out.c_str());
	ret = common_get_hardware_serial("cpu", &out);	printf("ret:%d\n", ret);
	if (ret)printf("%s\n", out.c_str());
	ret = common_get_hardware_serial("bios", &out);	printf("ret:%d\n", ret);
	if (ret)printf("%s\n", out.c_str());
	ret = common_get_hardware_serial("boardtype", &out);	printf("ret:%d\n", ret);
	if (ret)printf("%s\n", out.c_str());
	ret = common_get_hardware_serial("mac", &out);	printf("ret:%d\n", ret);
	if (ret)printf("%s\n", out.c_str());
}

void robot_input_thread::run()
{
	//printf_hardware();
	while (true)
	{
		char c = getchar();
		if (c != '\n')order_c = c;
		SetEvent(m_event_wait);
	}
		
}
