#include "robot_FaceDebug_Windows.h"
#include <atltypes.h>

#define WM_SHOWVIDEO WM_USER+10
robot_FaceDebug_Windows::robot_FaceDebug_Windows()
{
	citr_cache = common_mutex_create_threadlock();
}

void robot_FaceDebug_Windows::ShowMat(cv::Mat &image)
{
	if (!state()||image.empty())
		return;
	//LOCK_HELPER_AUTO_HANDLER(citr_cache);
	common_mutex_threadlock_lock(citr_cache);
	image.copyTo(m_cache);
	common_mutex_threadlock_unlock(citr_cache);
	::PostMessage(m_hWnd, WM_SHOWVIDEO, 0, (LPARAM)this);
}

void robot_FaceDebug_Windows::ShowVideo(HDC hdc, char* szFrame, int iImgWidth, int iImgHeight)
{
	BITMAPINFO BmpInfo;
	BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BmpInfo.bmiHeader.biWidth = (LONG)iImgWidth;
	BmpInfo.bmiHeader.biHeight = -1*(LONG)iImgHeight;

	BmpInfo.bmiHeader.biPlanes = 1;
	BmpInfo.bmiHeader.biBitCount = 24;
	BmpInfo.bmiHeader.biCompression = BI_RGB;
	BmpInfo.bmiHeader.biSizeImage = 0;
	BmpInfo.bmiHeader.biXPelsPerMeter = 0;
	BmpInfo.bmiHeader.biYPelsPerMeter = 0;
	BmpInfo.bmiHeader.biClrUsed = 0;
	BmpInfo.bmiHeader.biClrImportant = 0;

	SetStretchBltMode(hdc, COLORONCOLOR);
	StretchDIBits(hdc, 0, 0, iImgWidth, iImgHeight,
		0, 0, iImgWidth, iImgHeight, szFrame, &BmpInfo, DIB_RGB_COLORS, SRCCOPY);
	
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
	case WM_SHOWVIDEO:
	{
		if (lParam != 0)
		{
			robot_FaceDebug_Windows *on = (robot_FaceDebug_Windows*)lParam;
			on->Show();
		}
	}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
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



bool robot_FaceDebug_Windows::Init(std::string windowsname)
{
	if (state() || windowsname.empty())
		return false;
	m_windowstitle = windowsname;
	common_get_guid(&m_windowsclassname);
	clean_msgqueue();
	return start();
}

void robot_FaceDebug_Windows::run()
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
		return;
	}

	m_hWnd = CreateWindowA(m_windowsclassname.c_str(), m_windowstitle.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, hInstance, nullptr);

	if (!m_hWnd)
	{
		LOGE("create camera monitor window failed error :%d.", GetLastError());
		return;
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

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
}

void robot_FaceDebug_Windows::exitWindows()
{
	if (!state())
		return;
	::SendMessage(m_hWnd, WM_QUIT, 0, 0);
	Thread_helper::quit();
}

robot_FaceDebug_Windows::~robot_FaceDebug_Windows()
{
	exitWindows();
	common_mutex_threadlock_release(citr_cache);
}

void robot_FaceDebug_Windows::Show()
{
	LOCK_HELPER_AUTO_HANDLER(citr_cache);
	HDC hdc;
	hdc = ::GetDC(m_hWnd);
	CRect cr, wrc;
	GetWindowRect(m_hWnd, &wrc);
	GetClientRect(m_hWnd, &cr);//获取对话框客户区域大小
	if (cr.Width() != m_cache.cols || cr.Height() != m_cache.rows)
		MoveWindow(m_hWnd, wrc.left, wrc.top,m_cache.cols, m_cache.rows, TRUE);//设置窗口显示的位置以及大小
	ShowVideo(hdc, (char*)(m_cache.data), m_cache.cols, m_cache.rows);
	ReleaseDC(m_hWnd, hdc);	
}
