

#ifndef __CSERIALPORT_H__
#define __CSERIALPORT_H__

#include "stdio.h"
#include "tchar.h"
#define  WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>
#include <list>
#include <mutex>

struct serialPortBuffer
{
	int len;
	PBYTE buffer;
};
class CSerialPortRecv
{
public:
	virtual void onSerialRecv(unsigned char* buff, int port, int len) = 0;
};
#ifndef WM_COMM_MSG_BASE 
#define WM_COMM_MSG_BASE		    WM_USER + 109		//!< 消息编号的基点  
#endif

#define WM_COMM_BREAK_DETECTED		WM_COMM_MSG_BASE + 1	// A break was detected on input.
#define WM_COMM_CTS_DETECTED		WM_COMM_MSG_BASE + 2	// The CTS (clear-to-send) signal changed state. 
#define WM_COMM_DSR_DETECTED		WM_COMM_MSG_BASE + 3	// The DSR (data-set-ready) signal changed state. 
#define WM_COMM_ERR_DETECTED		WM_COMM_MSG_BASE + 4	// A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY. 
#define WM_COMM_RING_DETECTED		WM_COMM_MSG_BASE + 5	// A ring indicator was detected. 
#define WM_COMM_RLSD_DETECTED		WM_COMM_MSG_BASE + 6	// The RLSD (receive-line-signal-detect) signal changed state. 
#define WM_COMM_RXCHAR				WM_COMM_MSG_BASE + 7	// A character was received and placed in the input buffer. 
#define WM_COMM_RXFLAG_DETECTED		WM_COMM_MSG_BASE + 8	// The event character was received and placed in the input buffer.  
#define WM_COMM_TXEMPTY_DETECTED	WM_COMM_MSG_BASE + 9	// The last character in the output buffer was sent.  
#define WM_COMM_RXSTR               WM_COMM_MSG_BASE + 10   // Receive string

#define MaxSerialPortNum 200    ///有效的串口总个数，不是串口的号 //add by itas109 2014-01-09
#define IsReceiveString  1      //采用何种方式接收：ReceiveString 1多字符串接收（对应响应函数为Wm_SerialPort_RXSTR），ReceiveString 0一个字符一个字符接收（对应响应函数为Wm_SerialPort_RXCHAR）



	class  CSerialPort
	{
	public:
		// contruction and destruction
		CSerialPort();
		virtual ~CSerialPort();

		// port initialisation		
		// UINT stopsbits = ONESTOPBIT   stop is index 0 = 1 1=1.5 2=2 
		// 切记：stopsbits = 1，不是停止位为1。
		// by itas109 20160506
		BOOL		InitPort(UINT portnr = 1, UINT baud = 9600,
			TCHAR parity = _T('N'), UINT databits = 8, UINT stopsbits = ONESTOPBIT,
			DWORD dwCommEvents = EV_RXCHAR | EV_CTS, UINT nBufferSize = 512,

			DWORD ReadIntervalTimeout = 1000,
			DWORD ReadTotalTimeoutMultiplier = 1000,
			DWORD ReadTotalTimeoutConstant = 1000,
			DWORD WriteTotalTimeoutMultiplier = 1000,
			DWORD WriteTotalTimeoutConstant = 1000);

		// start/stop comm watching
		///控制串口监视线程
		BOOL		 StartMonitoring();//开始监听
		BOOL		 ResumeMonitoring();//恢复监听
		BOOL		 SuspendMonitoring();//挂起监听
		BOOL         IsThreadSuspend(HANDLE hThread);//判断线程是否挂起 //add by itas109 2016-06-29

		DWORD		 GetWriteBufferSize();///获取写缓冲大小
		DWORD		 GetCommEvents();///获取事件
		DCB			 GetDCB();///获取DCB

		///写数据到串口
		void		WriteToPort(char* string, size_t n); // add by mrlong 2007-12-25
		void		WriteToPort(PBYTE Buffer, size_t n);// add by mrlong
		void		ClosePort();					 // add by mrlong 2007-12-2  
		BOOL		IsOpened();


		CSerialPortRecv* sendMessage;

		 std::mutex    m_ml;
		std::string GetVersion();

		void SerialThread();
	protected:
		// protected memberfunctions
		void		ProcessErrorMessage(TCHAR* ErrorText);///错误处理
		static DWORD WINAPI CommThread(LPVOID pParam);///线程函数
		static void	ReceiveChar(CSerialPort* port);
		static void ReceiveStr(CSerialPort* port); //add by itas109 2016-06-22
		static void	WriteChar(CSerialPort* port);

	private:
		// thread
		HANDLE			    m_Thread;
		BOOL                m_bIsSuspened;///thread监视线程是否挂起

		// synchronisation objects
		//CRITICAL_SECTION	m_csCommunicationSync;///临界资源

		std::mutex					m_protecter;
		volatile bool				m_bThreadAlive;///监视线程运行标志

		// handles
		HANDLE				m_hShutdownEvent;  //stop发生的事件
		HANDLE				m_hComm;		   // 串口句柄 
		HANDLE				m_hWriteEvent;	 // write

		// Event array. 
		// One element is used for each event. There are two event handles for each port.
		// A Write event and a receive character event which is located in the overlapped structure (m_ov.hEvent).
		// There is a general shutdown when the port is closed. 
		///事件数组，包括一个写事件，接收事件，关闭事件
		///一个元素用于一个事件。有两个事件线程处理端口。
		///写事件和接收字符事件位于overlapped结构体（m_ov.hEvent）中
		///当端口关闭时，有一个通用的关闭。
		HANDLE				m_hEventArray[3];

		// structures
		OVERLAPPED			m_ov;///异步I/O
		COMMTIMEOUTS		m_SerialPortTimeouts;///超时设置
		DCB					m_dcb;///设备控制块
		// misc
		UINT				m_nPortNr;		///串口号
		PBYTE				m_szWriteBuffer;///写缓冲区
		std::list<serialPortBuffer> m_bufferList;
		serialPortBuffer m_bufferStruct;
		DWORD				m_dwCommEvents;
		DWORD				m_nWriteBufferSize;///写缓冲大小

		size_t				m_nWriteSize;//写入字节数 //add by mrlong 2007-12-25
	};


#endif __CSERIALPORT_H__

	void SerialThread();
