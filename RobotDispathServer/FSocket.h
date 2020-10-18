#pragma once
#ifdef WIN32
#include <winsock2.h>
typedef SOCKET HSocket;
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef int HSocket;
#define SOCKET_ERROR (-1)
#define INVALID_SOCKET 0
#endif

#define  SOCKET_OK		  0
#define  SOCKET_OCCERROR  -1
#define  SOCKET_EAGAIN   -2
#define  SOCKET_DISCONNECT	  -3
#define  SOCKET_RECVMAX   -4
#define ISSOCKHANDLE(x) (x!=INVALID_SOCKET)

#include <string>
#include "xCommon.h"

class FSocket
{
public:
	FSocket(int tcpudp = SOCK_STREAM);
	FSocket(HSocket socket_t);
	~FSocket();

	bool isvalid();

	void set_address(const char *ip, int port);
	void set_address(sockaddr_in *addr);

	int set_time_out(int recvtimeout, int sendtimeout, int lingertimeout);
	int set_buffer_size(int recvbuffersize, int sendbuffersize);
	int set_block(bool bblock);
	int set_socket_default();

	std::string get_ip_string();
	int get_port();
	HSocket socket_accept(sockaddr_in *paddr);
	HSocket get_handle() { return m_hSocket; }

	void reopen(bool bForceClose);
	void close();

	int xsend(void *ptr, int nbytes);
	int xrecv(void *ptr, int nbytes);
	int try_xsend(void *ptr, int nbytes, int milliseconds);
	int try_xrecv(void *ptr, int nbytes, int milliseconds);
	void clear_buffer();
	int xlisten(int max);
protected:
	class _xwin32Helper{
	public:
		_xwin32Helper();
		~_xwin32Helper();
	};
	static _xwin32Helper m_win32helper;
	int get_socket_error();

	void socket_close();
	HSocket init(int tcpudp);
	int socket_bind(HSocket hs, sockaddr_in *paddr);
	int socket_linsten(HSocket hs, int maxconn);

	int socket_send(HSocket hs, const char *ptr, int nbytes);
	int socket_recv(HSocket hs, char *ptr, int nbytes);
	int try_socket_send(HSocket hs, const char *ptr, int nbytes, int milliseconds);
	int try_socket_recv(HSocket hs, char *ptr, int nbytes, int milliseconds);
	void clear_system_buffer(HSocket hs);
	int socket_block(HSocket hs, bool bblock);
	int socket_timeout(HSocket hs, int recvtimeout, int sendtimeout, int lingertimeout);
	HSocket m_hSocket;
	sockaddr_in m_stAddr;
	int m_tcpudp;
};

