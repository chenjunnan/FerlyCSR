
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "FSocket.h"
#include <string>

#if defined WIN32
#include <windows.h>
#define BLOCKREADWRITE 0
#define NONBLOCKREADWRITE 0
#define SENDNOSIGNAL 0
#define ETRYAGAIN(x) (x==WSAEWOULDBLOCK||x==WSAETIMEDOUT)
#define gxsprintf sprintf_s

#else
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define ISSOCKHANDLE(x) (x>0)
#define BLOCKREADWRITE MSG_WAITALL
#define NONBLOCKREADWRITE MSG_DONTWAIT
#define SENDNOSIGNAL MSG_NOSIGNAL
#define ETRYAGAIN(x) (x==EAGAIN||x==EWOULDBLOCK)
#define gxsprintf snprintf

#endif
#include <ws2tcpip.h>
#include "common_helper.h"


#define INVALIDSOCKHANDLE INVALID_SOCKET


FSocket::FSocket(int tcpudp/*=1*/)
{
	memset(&m_stAddr, 0, sizeof(sockaddr_in));
	m_tcpudp = tcpudp;
	m_hSocket = INVALIDSOCKHANDLE;
	init(m_tcpudp);
}

FSocket::FSocket(HSocket socket_t)
{
	if (socket_t > 0)
	{
		m_hSocket = socket_t;
		int len = sizeof(sockaddr_in);
		int ret = getpeername(m_hSocket, (sockaddr*)&m_stAddr, &len);
		m_tcpudp = 1/*m_stAddr.sin_family*/;
	}
}


FSocket::~FSocket()
{
	socket_close();
}


void FSocket::set_address(const char *ip, int port)
{
	memset(&m_stAddr, 0, sizeof(sockaddr_in));
	m_stAddr.sin_family = AF_INET;/*地址类型为AF_INET*/
	if (ip)
	{
		inet_pton(AF_INET, ip, (void *)&m_stAddr);
	}
	else
	{
		m_stAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	m_stAddr.sin_port = htons(port);/*端口号*/
}

std::string FSocket::get_ip_string()
{
	char buffer_ip[1024] = { 0 };
	std::string ipstr;
	unsigned char *p = (unsigned char *)(&(m_stAddr.sin_addr));
	gxsprintf(buffer_ip, 1024, "%u.%u.%u.%u", *p, *(p + 1), *(p + 2), *(p + 3));
	return ipstr = buffer_ip;
}

int FSocket::get_port()
{
	return m_stAddr.sin_port;
}

int FSocket::get_socket_error()
{
#ifdef WIN32

	return WSAGetLastError();
#else
	return errno;
#endif
}

bool FSocket::isvalid()
{
	return ISSOCKHANDLE(m_hSocket);
}

void FSocket::socket_close()
{
	if (ISSOCKHANDLE(m_hSocket))
	{
#ifdef WIN32
		//shutdown(m_hSocket, SD_BOTH);
		closesocket(m_hSocket);
#else
		close(m_hSocket);
#endif
		m_hSocket = INVALIDSOCKHANDLE;
	}
}

HSocket FSocket::init(int tcpudp)
{
	int protocol = 0;
#if defined(_WIN32_PLATFROM_)
	if (tcpudp == SOCK_STREAM) protocol = IPPROTO_TCP;
	else if (tcpudp == SOCK_DGRAM) protocol = IPPROTO_UDP;
#endif
	m_hSocket = socket(AF_INET, tcpudp, protocol);
	return m_hSocket;
}

int  FSocket::socket_bind(HSocket hs, sockaddr_in *paddr)
{
	return bind(hs, (struct sockaddr *)paddr, sizeof(sockaddr_in));
}

int FSocket::socket_linsten(HSocket hs, int maxconn)
{
	return listen(hs, maxconn);
}



HSocket FSocket::socket_accept(sockaddr_in *paddr)
{
#ifdef WIN32
	int cliaddr_len = sizeof(sockaddr_in);
#else
	socklen_t cliaddr_len = sizeof(sockaddr_in);
#endif
	return accept(m_hSocket, (struct sockaddr *)paddr, &cliaddr_len);
}

// if timeout occurs, nbytes=-1, nresult=1
// if socket error, nbyte=-1, nresult=-1
// if the other side has disconnected in either block mode or nonblock mode, nbytes=0, nresult=-1
// otherwise nbytes= the count of bytes sent , nresult=0
int FSocket::socket_send(HSocket hs, const char *ptr, int nbytes)
{
	if (!ISSOCKHANDLE(hs)) return SOCKET_OCCERROR;
	int send_len = 0;
	if (!ptr || nbytes < 1) return SOCKET_OCCERROR;

	//Linux: flag can be MSG_DONTWAIT, MSG_WAITALL, 使用MSG_WAITALL的时候, socket 必须是处于阻塞模式下，否则WAITALL不能起作用
	send_len = send(hs, ptr, nbytes, BLOCKREADWRITE | SENDNOSIGNAL);
	if (send_len > 0)
	{
		return send_len == nbytes ? SOCKET_OK : send_len;
	}
	else if (send_len == 0)
	{
		return SOCKET_DISCONNECT;
	}
	else
	{
		return ETRYAGAIN(get_socket_error()) ? SOCKET_EAGAIN : SOCKET_OCCERROR;
	}
}

// if timeout occurs, nbytes=-1, nresult=1
// if socket error, nbyte=-1, nresult=-1
// if the other side has disconnected in either block mode or nonblock mode, nbytes=0, nresult=-1
int FSocket::socket_recv(HSocket hs, char *ptr, int nbytes)
{
	if (!ISSOCKHANDLE(hs)) return -1;
	int recv_len = 0;
	if (!ptr) return -1;
	
	recv_len = recv(hs, ptr, nbytes, BLOCKREADWRITE);
	if (recv_len > 0)
	{
		return recv_len == nbytes ? SOCKET_RECVMAX : recv_len;
	}
	else if (recv_len == 0)
	{
		return SOCKET_DISCONNECT;//disconnect
	}
	else
	{
		return ETRYAGAIN(get_socket_error()) ? SOCKET_EAGAIN : SOCKET_OCCERROR;
	}

}
//? nbytes= the count of bytes sent
// if timeout occurs, nresult=1
// if socket error,? nresult=-1,
// if the other side has disconnected in either block mode or nonblock mode, nresult=-2
int FSocket::try_socket_send(HSocket hs, const char *ptr, int nbytes, int milliseconds)
{
	if (!ISSOCKHANDLE(hs)) return -1;
	int alread_send = 0;
	if (!ptr || nbytes < 1) return -999;

	int n;
	Timer_helper start;
	while (1)
	{
		n = send(hs, ptr + alread_send, nbytes, NONBLOCKREADWRITE | SENDNOSIGNAL);
		if (n > 0)
		{
			alread_send += n;
			nbytes -= n;
			if (alread_send >= nbytes) { return  SOCKET_OK; }
		}
		else if (n == 0)
		{
			return SOCKET_DISCONNECT;//disconnect 
		}
		else
		{
			if (!ETRYAGAIN(get_socket_error()))
			{
				return SOCKET_OCCERROR;
			}
		}
		if (start.isreach(milliseconds)){ return SOCKET_EAGAIN; }
	}
}
// if timeout occurs, nbytes=-1, nresult=1
// if socket error, nbyte=-1, nresult=-1
// if the other side has disconnected in either block mode or nonblock mode, nbytes=0, nresult=-1
int  FSocket::try_socket_recv(HSocket hs, char *ptr, int nbytes, int milliseconds)
{
	if (!ISSOCKHANDLE(hs)) return -1;
	int alread_recv = 0;
	if (!ptr || nbytes < 1) return -999;

	if (milliseconds > 2)
	{
		Timer_helper start;
		while (1)
		{
			alread_recv = recv(hs, ptr, nbytes, NONBLOCKREADWRITE);
			if (alread_recv > 0)
			{
				return  SOCKET_OK;
			}
			else if (alread_recv == 0)
			{

				return SOCKET_DISCONNECT;//disconnect 
			}
			else
			{
				if (!ETRYAGAIN(get_socket_error()))
				{
					return SOCKET_OCCERROR;
				}
			}
			if (start.isreach(milliseconds)){ return SOCKET_EAGAIN; }

		}
	}
	else
	{
		return socket_recv(hs, ptr, nbytes);
	}
}

void FSocket::clear_system_buffer(HSocket hs)
{
#ifdef WIN32
	struct timeval tmOut;
	tmOut.tv_sec = 0;
	tmOut.tv_usec = 0;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(hs, &fds);
	int nRet = 1;
	char tmp[100];
	while (nRet > 0)
	{
		nRet = select(FD_SETSIZE, &fds, NULL, NULL, &tmOut);
		if (nRet > 0)
		{
			nRet = recv(hs, tmp, 100, 0);
		}
	}
#else
	char tmp[100];
	while (xrecv(hs, tmp, 100, NONBLOCKREADWRITE) > 0);
#endif
}

int  FSocket::socket_block(HSocket hs, bool bblock)
{
	unsigned long mode;
	if (ISSOCKHANDLE(hs))
	{
#ifdef WIN32
		mode = bblock ? 0 : 1;
		return ioctlsocket(hs, FIONBIO, &mode);
#else
		mode = fcntl(hs, F_GETFL, 0); ?
			return bblock ? fcntl(hs, F_SETFL, mode&~O_NONBLOCK) : fcntl(hs, F_SETFL, mode | O_NONBLOCK);
#endif
	}
	return -1;
}

int FSocket::socket_timeout(HSocket hs, int recvtimeout, int sendtimeout, int lingertimeout)
{
	int rt = -1;
	struct linger lin;
	if (ISSOCKHANDLE(hs))
	{
		rt = 0;
#ifdef WIN32
		if (lingertimeout > -1)
		{
			lin.l_onoff = 1;
			lin.l_linger = lingertimeout;
			rt = setsockopt(hs, SOL_SOCKET, SO_LINGER, (const char*)&lin, sizeof(linger)) == 0 ? 0 : 0x1;
		}
		else
		{
			lin.l_onoff = 0;
			rt = setsockopt(hs, SOL_SOCKET, SO_LINGER, (const char*)&lin, sizeof(linger)) == 0 ? 0 : 0x1;
		}
		if (recvtimeout > 0 && rt == 0)
		{
			rt = rt | (setsockopt(hs, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvtimeout, sizeof(int)) == 0 ? 0 : 0x2);
		}
		if (sendtimeout > 0 && rt == 0)
		{
			rt = rt | (setsockopt(hs, SOL_SOCKET, SO_SNDTIMEO, (char *)&sendtimeout, sizeof(int)) == 0 ? 0 : 0x4);
		}
#else
		struct timeval timeout;
		if (lingertimeout > -1)
		{
			struct linger ? lin;
			lin.l_onoff = lingertimeout > 0 ? 1 : 0;
			lin.l_linger = lingertimeout / 1000;
			rt = setsockopt(hs, SOL_SOCKET, SO_LINGER, (const char*)&lin, sizeof(linger)) == 0 ? 0 : 0x1;
		}
		if (recvtimeout > 0 && rt == 0)
		{
			timeout.tv_sec = recvtimeout / 1000;
			timeout.tv_usec = (recvtimeout % 1000) * 1000;
			rt = rt | (setsockopt(hs, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == 0 ? 0 : 0x2);
		}
		if (sendtimeout > 0 && rt == 0)
		{
			timeout.tv_sec = sendtimeout / 1000;
			timeout.tv_usec = (sendtimeout % 1000) * 1000;
			rt = rt | (setsockopt(hs, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == 0 ? 0 : 0x4);
		}
#endif
	}
	return rt;
}


int InitializeSocketEnvironment()
{
#ifdef WIN32
	WSADATA Ws;
	//Init Windows Socket
	if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0)
	{
		LOGF("WIN32 network init failed !!!!!!");
		return -1;
	}
#endif
	return 0;
}

void FreeSocketEnvironment()
{
#ifdef WIN32
	WSACleanup();
#endif
}



void FSocket::reopen(bool bForceClose)
{
	if (ISSOCKHANDLE(m_hSocket) && bForceClose) socket_close();
	if (!ISSOCKHANDLE(m_hSocket))
	{
		m_hSocket = init(m_tcpudp);
	}

}

void FSocket::close()
{
	socket_close();
}


void FSocket::set_address(sockaddr_in *addr)
{
	memcpy(&m_stAddr, addr, sizeof(sockaddr_in));
}

int FSocket::set_time_out(int recvtimeout, int sendtimeout, int lingertimeout) //in milliseconds
{

	return socket_timeout(m_hSocket, recvtimeout, sendtimeout, lingertimeout);
}

int FSocket::set_buffer_size(int recvbuffersize, int sendbuffersize) //in bytes
{
	int nErr = -1;
	if (ISSOCKHANDLE(m_hSocket))
	{
#ifdef WIN32
		if (recvbuffersize > -1)
		{
			nErr = setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&recvbuffersize, sizeof(int));
			if (nErr == -1)
			{
				LOGE(("setsockopt SO_RCVBUF error %d"), WSAGetLastError());
				return -1;
			}
		}
		if (sendbuffersize > -1)
		{
			nErr = setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char *)&sendbuffersize, sizeof(int));
			if (nErr == -1)
			{
				LOGE(("setsockopt SO_SNDBUF error %d"), WSAGetLastError());
				return -1;
			}
		}
#endif
	}
	return nErr;
}

int FSocket::set_block(bool bblock)
{
	return socket_block(m_hSocket, bblock);
}

int FSocket::set_socket_default()
{
	BOOL bReuseaddr = TRUE;
	int nErr = setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&bReuseaddr, sizeof(BOOL));
	if (nErr == -1)
	{
		LOGE(("setsockopt SO_REUSEADDR error %d"), WSAGetLastError());
		return -1;
	}

	BOOL bDontLinger = FALSE;
	nErr = setsockopt(m_hSocket, SOL_SOCKET, SO_DONTLINGER, (const char*)&bDontLinger, sizeof(BOOL));
	if (nErr == -1)
	{
		LOGE(("setsockopt SO_DONTLINGER error %d"), WSAGetLastError());
		return -1;
	}
	const char chOpt = 1;
	nErr = setsockopt(m_hSocket, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));
	if (nErr == -1)
	{
		LOGE(("setsockopt TCP_NODELAY error %d"), WSAGetLastError());
		return -1;
	}
	return 0;
}

int  FSocket::xsend(void *ptr, int nbytes)
{
	return socket_send(m_hSocket, (const char *)ptr, nbytes);
}
int FSocket::xrecv(void *ptr, int nbytes)
{
	return socket_recv(m_hSocket, (char *)ptr, nbytes);
}
int FSocket::try_xsend(void *ptr, int nbytes, int milliseconds)
{
	return try_socket_send(m_hSocket, (const char *)ptr, nbytes, milliseconds);
}
int FSocket::try_xrecv(void *ptr, int nbytes, int milliseconds)
{
	return try_socket_recv(m_hSocket, (char *)ptr, nbytes, milliseconds);
}

void FSocket::clear_buffer()
{
	clear_system_buffer(m_hSocket);
}

FSocket::_xwin32Helper FSocket::m_win32helper;

int FSocket::xlisten(int max)
{
	int ret=socket_bind(m_hSocket, &m_stAddr);
	if (ret !=0)
		return -1;
	return socket_linsten(m_hSocket, max);
}

FSocket::_xwin32Helper::_xwin32Helper()
{
	InitializeSocketEnvironment();
}

FSocket::_xwin32Helper::~_xwin32Helper()
{
	FreeSocketEnvironment();
}
