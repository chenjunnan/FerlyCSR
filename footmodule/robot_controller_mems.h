#pragma once
#include "common_helper.h"
#include "SerialPort.h"
#include "robot_controller_interface.h"

struct MEMSCTROLS
{
	unsigned char fpos=0;
	unsigned char bpos = 0;
	int   angle = 0;
	int	   quadrant = 0;
	unsigned long long times=0;
	void clean()
	{
		 fpos = 0;
		 bpos = 0;
		  angle = 0;
		quadrant = 0;
		times = 0;
	}
};

class MEMSCTROLSHandler
{
public :
	virtual 	void mems_in(MEMSCTROLS &) = 0;;
};

class robot_controller_mems :public single_handle_thread_base, public CSerialPortRecv
{
public:
	robot_controller_mems();
	bool open_com(int port);
	bool autoopen_com(int port);
	virtual void onSerialRecv(unsigned char* buff, int port , int len);
	void writeSerial(unsigned char* buff, int len);
	void close_com();
	MEMSCTROLSHandler *handleer;
	virtual ~robot_controller_mems();
private:
	virtual void run();
	CSerialPort m_SerialPort;
	std::vector<char > m_serialbuffer;
	int				_level;
	volatile bool			isFinish;
	bool				autoopen;
	int					m_port;
	std::mutex		m_mfish;

};

