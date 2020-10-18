#include "robot_controller_mems.h"
#define	Cdh_StartCmd		0X5A //0X5A
#define	Cdh_EndCmd			0X15

#define	    POS_Err 1
#define	    LE_Speed 2
#define	    RE_Speed 3
#define	    CHECK 4
#define     END 5

robot_controller_mems::robot_controller_mems()
{
	m_SerialPort.sendMessage = this;
	isFinish = false;
	m_port = 0;
	autoopen = false;
	common_config_get_field_int("robotunderpan", "level", &_level);
	if (_level < 0 || _level>1)_level = 0;
}


bool robot_controller_mems::open_com(int port)
{
	m_mfish.lock();
	isFinish = false;
	m_mfish.unlock();

	if (m_SerialPort.IsOpened()||state())
		return true;

	if (!m_SerialPort.InitPort(port, 115200))
		return false;
	m_port = port;
	autoopen = false;
	return start();
}

bool robot_controller_mems::autoopen_com(int port)
{
	m_mfish.lock();
	isFinish = false;
	m_mfish.unlock();

	if (m_SerialPort.IsOpened() || state())
		return true;

	m_port = port;
	autoopen = true;

	return start();
}

void dump_row(long count, int numinrow, int *chs) {
	int i;

	printf("%08lX:", count - numinrow);

	if (numinrow > 0) {
		for (i = 0; i < numinrow; i++) {
			if (i == 8) {
				printf(" :");
			}
			printf(" %02X", chs[i]);
		}
		for (i = numinrow; i < 16; i++) {
			if (i == 8) {
				printf(" :");
			}
			printf("   ");
		}
		printf("  ");
		for (i = 0; i < numinrow; i++) {
			if (isprint(chs[i])) {
				printf("%c", chs[i]);
			}
			else {
				printf(".");
			}
		}
	}
	printf("\n");
}

int rows_eq(int *a, int *b) {
	int i;

	for (i = 0; i < 16; i++)
		if (a[i] != b[i]) {
			return 0;
		}

	return 1;
}

void amqp_dump(void const *buffer, size_t len) {
	unsigned char *buf = (unsigned char *)buffer;
	long count = 0;
	int numinrow = 0;
	int chs[16];
	int oldchs[16] = { 0 };
	int showed_dots = 0;
	size_t i;

	for (i = 0; i < len; i++) {
		int ch = buf[i];

		if (numinrow == 16) {
			int j;

			if (rows_eq(oldchs, chs)) {
				if (!showed_dots) {
					showed_dots = 1;
					printf(
						"          .. .. .. .. .. .. .. .. : .. .. .. .. .. .. .. ..\n");
				}
			}
			else {
				showed_dots = 0;
				dump_row(count, numinrow, chs);
			}

			for (j = 0; j < 16; j++) {
				oldchs[j] = chs[j];
			}

			numinrow = 0;
		}

		count++;
		chs[numinrow++] = ch;
	}

	dump_row(count, numinrow, chs);

	if (numinrow != 0) {
		printf("%08lX:\n", count);
	}
}
// int coun_m = 0;
// Timer_helper p;
void robot_controller_mems::onSerialRecv(unsigned char* buff, int port, int len)
{
	//amqp_dump(buff, len);
	m_mfish.lock();
	bool isfinfs = isFinish;
	m_mfish.unlock();
	if (isfinfs)return;

	m_serialbuffer.insert(m_serialbuffer.end(), buff, buff + len);

	bool hasFullFrame = false;

	bool find_head = false;
	if (m_serialbuffer.size() < 6)
		return;


	int beginpos = 0;
	for (int i = 0; i < m_serialbuffer.size(); i++)
	{
		if (m_serialbuffer[0] == Cdh_StartCmd)
		{
			if (m_serialbuffer.size() >= 7)
			{
				if (m_serialbuffer[6] == Cdh_EndCmd)
				{
					char   Check = 0;
					unsigned int     Check_temp = 0;
					if (_level)
						Check_temp = (m_serialbuffer[1] + m_serialbuffer[2]+ m_serialbuffer[3]*2 + m_serialbuffer[4]);  //校验计算
					else
						Check_temp = (m_serialbuffer[1] + m_serialbuffer[2] + m_serialbuffer[3] + m_serialbuffer[4]);  //校验计算
					Check = Check_temp & 0x00ff;

					if (Check == m_serialbuffer[5])//校验通过
					{
						MEMSCTROLS temp;
						temp.fpos = m_serialbuffer[1];
						temp.bpos = m_serialbuffer[2];
						temp.angle = m_serialbuffer[3];
						temp.quadrant = m_serialbuffer[4];
						handleer->mems_in(temp);
					}
					m_serialbuffer.erase(m_serialbuffer.begin(), m_serialbuffer.begin() + i);
					i = 0;
					find_head = false;
				}
				else
					m_serialbuffer.erase(m_serialbuffer.begin());
			}
			else
				break;

		}
		else
		{
			m_serialbuffer.erase(m_serialbuffer.begin());
		}

	}

}


void robot_controller_mems::writeSerial(unsigned char* buff, int len)
{
	m_SerialPort.WriteToPort(buff, len);
}

void robot_controller_mems::close_com()
{
	m_mfish.lock();
	isFinish = true;
	m_mfish.unlock();
	m_serialbuffer.clear();

}


robot_controller_mems::~robot_controller_mems()
{
}

void robot_controller_mems::run()
{
	if (autoopen)
	{
		if (!m_SerialPort.InitPort(m_port, 115200))
		{
			m_mfish.lock();
			isFinish = true;
			m_mfish.unlock();
			LOGW("serial thread exit!!!");
			return;
		}
	}
	m_serialbuffer.clear();
	m_SerialPort.SerialThread();
	m_SerialPort.ClosePort();


	m_mfish.lock();
	isFinish = true;
	m_mfish.unlock();
	LOGW("serial thread exit!!!");
}
