#include <windows.h>
#include "DeviceNotify.h"
#include "xCommon.h"
#include <setupapi.h>
#include <comdef.h>
#include "VideoManager.h"
#pragma comment(lib, "Setupapi.lib")

CDeviceNotify::CDeviceNotify()
{
}

void CDeviceNotify::quit()
{
	::SendMessage(m_hWnd, WM_QUIT, 0, 0);
	Thread_helper::wait();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_DEVICECHANGE:
	{
		if (DBT_DEVICEARRIVAL != wParam && DBT_DEVICEREMOVECOMPLETE != wParam)
			break;

		PDEV_BROADCAST_HDR pdbh = (PDEV_BROADCAST_HDR)lParam;
		if (pdbh->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
		{
			break;
		}

		PDEV_BROADCAST_DEVICEINTERFACE pdbi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
		// Check for capture devices.
		if (pdbi->dbcc_classguid != GUID_CAPTURE)
		{
			break;
		}
		else
		{
			if (wParam == DBT_DEVICEARRIVAL )
				VideoManager::get()->update_device();
			if (wParam == DBT_DEVICEREMOVECOMPLETE)
				VideoManager::get()->remove_device();
		}
	}
	break;
	case WM_QUIT:
	{
		PostQuitMessage(0);
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}



bool CDeviceNotify::Init(GUID type,std::string windowsname)
{
	m_windowstitle = windowsname;
	common_get_guid(&m_windowsclassname);
	m_guid = type;
	return start();
}

CDeviceNotify::~CDeviceNotify()
{
	if (state())Thread_helper::quit();
}

void CDeviceNotify::run()
{
	HINSTANCE hInstance = ::GetModuleHandle(NULL);

	WNDCLASSEXA wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, NULL);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = m_windowsclassname.c_str();
	wcex.hIconSm = LoadIcon(wcex.hInstance, NULL);

	DWORD ret = RegisterClassExA(&wcex);
	if (ret == 0)
	{
		LOGE("create camera monitor class failed error :%d.", GetLastError());
		return ;
	}

	m_hWnd = CreateWindowA(m_windowsclassname.c_str(), m_windowstitle.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!m_hWnd)
	{
		LOGE("create camera monitor window failed error :%d.", GetLastError());
		return ;
	}

	ShowWindow(m_hWnd, SW_HIDE);
	UpdateWindow(m_hWnd);

	DEV_BROADCAST_DEVICEINTERFACE filterData;
	ZeroMemory(&filterData, sizeof(DEV_BROADCAST_DEVICEINTERFACE));

	filterData.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	filterData.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	filterData.dbcc_classguid = m_guid;

	gpUnregisterDeviceNotification = NULL;
	gpRegisterDeviceNotification = NULL;
	// dynload device removal APIs
	{

		HMODULE hmodUser = GetModuleHandle(TEXT("user32.dll"));
		if (hmodUser)
		{
			gpUnregisterDeviceNotification = (PUnregisterDeviceNotification)
				GetProcAddress(hmodUser, "UnregisterDeviceNotification");

			gpRegisterDeviceNotification = (PRegisterDeviceNotificationA)
				GetProcAddress(hmodUser,
				"RegisterDeviceNotificationA"
				);
		}
		else
		{
			return ;
		}

	}

	m_hDevNotify = NULL;

	if (gpRegisterDeviceNotification)
	{
		m_hDevNotify = gpRegisterDeviceNotification(m_hWnd, &filterData, DEVICE_NOTIFY_WINDOW_HANDLE);
		if (!m_hDevNotify)
		{
			LOGE("register device notify failed %d.", GetLastError());
			return ;
		}
	}
	else
	{
		LOGE("get register device interface failed %d.", GetLastError());
		return ;
	}
	MSG msg;
	Message msgth;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		msgth = getq(0);
		if (msgth.msgType == _EXIT_THREAD_)break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	DestroyWindow(m_hWnd);
	UnregisterClass(m_windowsclassname.c_str(), hInstance);
	LOGI("exit device notify thread.");
}
