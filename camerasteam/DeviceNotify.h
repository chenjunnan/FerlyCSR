#pragma once
#include <string>
#include "common_helper.h"
#include <windows.h>
#include <Dbt.h>

const GUID GUID_CAPTURE = { 0x65E8773DL, 0x8F56, 0x11D0, { 0xA3, 0xB9, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96 } };


typedef BOOL(_stdcall * PUnregisterDeviceNotification)(
	HDEVNOTIFY Handle
	);

typedef HDEVNOTIFY(_stdcall *PRegisterDeviceNotificationA)(
	HANDLE hRecipient,
	LPVOID NotificationFilter,
	DWORD Flags
	);
class CDeviceNotify:public Thread_helper
{
public:
	CDeviceNotify();
	virtual void quit();
	bool Init(GUID type, std::string windowsname);
	virtual ~CDeviceNotify();
private:
	virtual void run();
	std::string m_windowstitle;
	std::string m_windowsclassname;
	HDEVNOTIFY m_hDevNotify = 0;
	HWND m_hWnd;
	GUID   m_guid;
	PUnregisterDeviceNotification gpUnregisterDeviceNotification = 0;
	PRegisterDeviceNotificationA gpRegisterDeviceNotification = 0;

};
