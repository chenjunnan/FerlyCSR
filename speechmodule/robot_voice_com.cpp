#include "robot_voice_com.h"
#include <iostream>
#include "robot_PostMessage.h"
#include "robot_voice_struct.h"

#define ONE5STOPBITS 1


robot_voice_com::robot_voice_com()
{
	m_hCom = NULL;
	default_angle = 0;
	autowakeup = true;
}


bool robot_voice_com::open(const char * port)
{
	close_com();

	char buffer[64] = { 0 };

	if (strlen(port)==4)
	{
		sprintf(buffer, "%s", port);
	}else if (strlen(port) == 5)
	{
		sprintf(buffer, "\\\\.\\%s", port);
	}
	else
	{
		LOGE("ifly com port error %s",port);
		return false;
	}
	m_hCom = CreateFile(buffer,
		GENERIC_READ | GENERIC_WRITE,//允许读和写；
		0,//独占方式；
		NULL,
		OPEN_EXISTING,//打开而不是创建；
		FILE_ATTRIBUTE_NORMAL,//重叠方式；
		NULL);

	if (m_hCom == INVALID_HANDLE_VALUE){
		LOGE("error %d:open ifly  %s  com failed !!", GetLastError(), buffer);
		close_com();
		return false;
	}

	COMMTIMEOUTS timeouts;
	GetCommTimeouts(m_hCom, &timeouts);
	timeouts.ReadIntervalTimeout = 10;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.ReadTotalTimeoutConstant = 1;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;

	if (!SetCommTimeouts(m_hCom, &timeouts))
	{
		LOGE("ifly com SetCommTimeouts failed!");
		close_com();
		return false;
	}

	DCB m_dcb;
	int ret = 0;

	m_dcb.DCBlength = sizeof(DCB);
	GetCommState(m_hCom, &m_dcb);
	m_dcb.BaudRate = 115200;
	m_dcb.Parity = NOPARITY;
	m_dcb.ByteSize = 8;
	m_dcb.StopBits = 0;

	if (!SetCommState(m_hCom, &m_dcb)){
		LOGE("error %d:SetCommState ifly com failed !!", GetLastError());
		close_com();
		return false;
	}

	if (!GetCommState(m_hCom, &m_dcb))
	{
		LOGE("error %d:GetCommState ifly com failed !!", GetLastError());
		close_com();
		return false;
	}

	if (!PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT))
	{
		LOGE("error %d:PurgeComm ifly com failed !!", GetLastError());
		close_com();
		return false;
	}		
	return start();
}

bool robot_voice_com::Beam(int i)
{
	pushq(i);
	return true;
}

bool robot_voice_com::Reset()
{
	pushq(-10);
	return true;
}

bool robot_voice_com::Reset_imp()
{
	char command[10] = { 'R', 'E', 'S', 'E', 'T', 0x0D, 0x0A, 0x00, 0x00, 0x00 };

	if (m_hCom == NULL)
		return true;

	DWORD write_len = 0;

	if (!WriteFile(m_hCom, command, 7, &write_len, NULL))
	{
		LOGE("error %d:write command to ifly com failed !!", GetLastError());
		open(port_str.c_str());
		return false;
	}
	else
	{
		LOGD("ifly will enter wait ");
	}
	autowakeup = false;
	return true;
}

bool robot_voice_com::write_imp(int i)
{
	char command[10] = { 'B', 'E', 'A', 'M', ' ', i + 0x30, 0x0D, 0x0A, 0x00, 0x00 };

	//sprintf_s(command,10, "BEAM %d\n",i);

	// 	if (m_lasttime.isreach_time_ms(60000*5))
	// 	{
	// 		m_lasttime.update_time();

	if (m_hCom == NULL)
		return true;

	DWORD write_len = 0;
	
	if (!WriteFile(m_hCom, command, 8, &write_len, NULL))
	{
		LOGE("error %d:write command to ifly com failed !!", GetLastError());
		open(port_str.c_str());
		return false;
	}
	else
	{
		LOGD("ifly will beam %d ", i);
		;// printf("com ifly config ok %d !!\n", write_len);
	}
	//}

	return true;
}


std::string robot_voice_com::substr_range(std::string str,int &begin,const char *key,char endspitc)
{
	std::string temp;
	if (begin<0 || begin>str.length())return temp;
	int sposb = str.find(key, begin);
	if (sposb == -1)return temp;
	if (sposb + strlen(key) > str.length())return temp;
	int spose = str.find(endspitc, sposb + strlen(key));
	if (spose == -1)return temp;
	temp = str.substr(sposb + strlen(key), spose - sposb - strlen(key));
	begin = spose;
	return temp;
}


void robot_voice_com::run()
{
	bool loop = true;
	Timer_helper   iflycom;
	std::string iflyout;
	std::vector<char> buffer;
	bool     wakeupntf = false;
	Timer_helper callwaketime;
	while (loop)
	{
		Message xMSG = getq(0);
		if (xMSG.msgType == _EXIT_THREAD_)return;
		if (xMSG.msgType >=0)
		{
			;// clean_msgqueue();
			int spit_a = (xMSG.msgType) / 60;;
			if (spit_a < 0)spit_a = 0;
			else if (spit_a == 6)spit_a = default_angle;
			else if (spit_a>6)spit_a = 0;
			autowakeup = true;
			if (write_imp(spit_a))
				default_angle = spit_a;
			callwaketime.update();
		}
		if (xMSG.msgType == -10)
		{
			Reset_imp();
		}
		if (autowakeup&&callwaketime.isreach(60000))
		{
			write_imp(default_angle);
			callwaketime.update();
		}

		unsigned char tc;
		if (readB(&tc))
		{
			buffer.push_back(tc);
			iflycom.update();
		}

		if (!wakeupntf&&buffer.size()>17)
		{
			iflyout.clear();
			iflyout.assign(buffer.begin(), buffer.end());
			int anglepos=iflyout.find("WAKE UP!angle:");
			if (anglepos != -1)
			{
				int angleposend = iflyout.find(0x0a, anglepos);
				if (angleposend != -1)
				{
					if (/*(angleposend - anglepos-strlen()) > 6 &&*/ (buffer.size() - anglepos)>32)
					{
						std::string angle = substr_range(iflyout, anglepos, "WAKE UP!angle:", 0x20);
						std::string score = substr_range(iflyout, anglepos, "score:", 0x20);
						std::string keyword = substr_range(iflyout, anglepos, "key_word:", 0x0a); 

						if (!angle.empty())
						{
							LOGI("call voice wakeup angle:%s.", angle.c_str());
							wakeupntf = true;

							std::string out = m_json.voice_wakeup_ntf(0, atoi(angle.c_str()),atoi(score.c_str()),keyword);
							robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length());
						}
					}
					else
					{
						std::string angle = substr_range(iflyout, anglepos, "WAKE UP!angle:", 0x0a);
						if (!angle.empty())
						{
							LOGI("call voice wakeup angle:%s.", angle.c_str());
							wakeupntf = true;

							std::string out = m_json.voice_wakeup_ntf(0, atoi(angle.c_str()));
							robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length());
						}


					}
				}
				
			}
		

		}

		if (iflycom.isreach(1500))
		{
			buffer.clear();
			iflyout.clear();
			wakeupntf = false;
		}
	}


}

bool robot_voice_com::readB(unsigned char* tranChar)
{
	DWORD    num;
	COMSTAT  comstat;
	DWORD    dwError;
	BOOL     bSuccess;

	if (tranChar == NULL)
		return FALSE;

	if (m_hCom == NULL)
		return FALSE;

	num = 0;
	*tranChar = 0;

	if (!ClearCommError(m_hCom, &dwError, &comstat))
	{
		return FALSE;
	}

	//读一个字节的数据
	bSuccess = ReadFile(m_hCom, tranChar, 1, &num, NULL);
	if (!bSuccess || num != 1)
	{
		ClearCommError(m_hCom, &dwError, &comstat);
		return FALSE;
	}

	return TRUE;
}

void robot_voice_com::close_com()
{
	if (m_hCom != NULL){
		CloseHandle(m_hCom);
		m_hCom = NULL;
	}
}

robot_voice_com::~robot_voice_com()
{
	if (state())quit();
	close_com();

}
