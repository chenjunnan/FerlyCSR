/*******************************************************
运动控制
********************************************************/
#include <string>
#include "RobotControlDevice.h"
#include "USB.h"
#include "xCommon.h"

RobotControlDevice::RobotControlDevice()
{
	m_handle_device = NULL;
	m_ov.hEvent = NULL;
	m_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

RobotControlDevice::~RobotControlDevice()
{
	CloseDevice();
	if (m_ov.hEvent != NULL)
		CloseHandle(m_ov.hEvent);
}

void RobotControlDevice::PopErrMsg(int ret)
{
	switch (ret)
	{
	case -1:
		strcpy(error_buffer, "数据发送失败");
		break;
	case -2:
		strcpy(error_buffer, "读取数据失败");
		break;
	case 0x10:
		strcpy(error_buffer, "包头错误");
		break;
	case 0x20:
		strcpy(error_buffer, "校验失败");
		break;
	case 0x30:
		strcpy(error_buffer, "角度超范围");
		break;
	case 0x40:
		strcpy(error_buffer, "头部校验失败");
		break;
	case 0x50:
		strcpy(error_buffer, "不存在该指令");
		break;
	case 0x60:
		strcpy(error_buffer, "写FLASH失败");
		break;
	default:
		break;
	}
}


//打开设备
int RobotControlDevice::OpenDevice()
{
	if (m_handle_device&&m_handle_device!=INVALID_HANDLE_VALUE)
		return OP_OK;
	m_ov.Offset = 0;
	m_ov.OffsetHigh = 0;
	int type = 0;
	if (m_ov.hEvent)
		type = 1;

	m_handle_device = OpenHIDDevice(type);
	if (m_handle_device == INVALID_HANDLE_VALUE)
	{
		int error = GetLastError();

		//LOGE("Open device HID failed :%d", error);
		return OP_ERR;
	}
	common_thread_sleep(100);
	return OP_OK;
}

//关闭设备
int RobotControlDevice::CloseDevice()
{
	if (!m_handle_device || m_handle_device == INVALID_HANDLE_VALUE)
		return OP_OK;

	LOGD("CloseDevice HID");
	CloseHandle(m_handle_device);
	m_handle_device = NULL;
	return OP_OK;
}

bool RobotControlDevice::isOpened()
{
	return m_handle_device != NULL;
}

char  RobotControlDevice::CheckNOR(char *Buf, int Len)
{
	int i;
	char value;
	value = *Buf;
	Buf = Buf + 1;
	for (i = 0; i < Len - 1; i++)
	{
		value = value ^ *Buf;
		Buf++;
	}
	return value;
}

void RobotControlDevice::InsertHead(char *buffer, char * cmd, unsigned char cmdlen)
{
	buffer[0] = 0x00;
	buffer[1] = 0xEF;
	buffer[2] = 0x01;
	buffer[3] = cmdlen;
	memcpy(buffer+4,cmd,cmdlen);
	//*(buffer + 4 + cmdlen) = CheckNOR((char *)buffer + 4, cmdlen);
	buffer[64] = CheckNOR((char *)buffer + 4, cmdlen);
}

void RobotControlDevice::BuildMotorCommand(char *buffer, unsigned char cmdtype, unsigned char motor,  char value)
{
	char cmd[3];
	cmd[0] = cmdtype;
	cmd[1] = motor;
	cmd[2] = value;
	InsertHead(buffer,cmd,3);
}

void RobotControlDevice::BuildMotorCommand(char *buffer, unsigned char cmdtype, unsigned char motor,  short value)
{
	char cmd[4];
	cmd[0] = cmdtype;
	cmd[1] = motor;
	cmd[2] = (char)(0x00ff & (value));
	cmd[3] = (char)(0x00ff & (value >> 8));
	InsertHead(buffer, cmd, 4);
}

void RobotControlDevice::BuildMotorCommand(char *buffer, unsigned char cmdtype, unsigned char motor)
{
	char cmd[2];
	cmd[0] = cmdtype;
	cmd[1] = motor;
	InsertHead(buffer, cmd, 2);
}


void RobotControlDevice::BuildMotorCommand(char *buffer, unsigned char cmdtype)
{
	char cmd[1];
	cmd[0] = cmdtype;
	InsertHead(buffer, cmd, 1);
}

int RobotControlDevice::WriteCommand(char *buffer,int len)
{
	if (!m_handle_device || m_handle_device == INVALID_HANDLE_VALUE)
	{
		PopErrMsg(-1);
		return OP_ERR;
	}
 	DWORD bytes=0;
// 	if (!WriteFile(m_handle_device, buffer, len, &bytes, NULL))
// 	{
// 		PopErrMsg(-1);
// 		CloseDevice();
// 		return OP_ERR;
// 	}
// 	if (bytes != len)
// 	{
// 		PopErrMsg(-1);
// 		return OP_ERR;
// 	}
//	return OP_OK;

	m_ov.Offset = 0;
	m_ov.OffsetHigh = 0;

	bool bResult = WriteFile(m_handle_device,						
		buffer,
		len,	
		&bytes,	
		&m_ov);

	if (!bResult)
	{
		DWORD dwError = GetLastError();
		switch (dwError)
		{
		case ERROR_IO_PENDING:
		{
			// continue to GetOverlappedResults()
			break;
		}
		case ERROR_ACCESS_DENIED:///拒绝访问 erroe code:5
		{
			CloseDevice();
			LOGE(("HID ERROR_ACCESS_DENIED，WriteFile() Error Code:%d"), GetLastError());
			return OP_ERR;
		}
		case ERROR_INVALID_HANDLE:///打开串口失败 erroe code:6
		{
			CloseDevice();
			LOGE(("HID ERROR_INVALID_HANDLE，WriteFile() Error Code:%d"), GetLastError());
			return OP_ERR;
		}
		case ERROR_BAD_COMMAND:///连接过程中非法断开 erroe code:22
		{
			CloseDevice();
			LOGE(("HID ERROR_BAD_COMMAND，WriteFile() Error Code:%d"), GetLastError());
			return OP_ERR;
		}
		default:
		{
			// all other error codes
			LOGE(("HID DEFAULT，WriteFile() Error Code:%d"), GetLastError());
		}
		}

	}
	else
	{
		if (bytes != len)
		{
			PopErrMsg(-1);
			CloseDevice();
			return OP_ERR;
		}
		return OP_OK;
	}

	DWORD wais = WaitForSingleObject(m_ov.hEvent, 1000);
	if (wais == WAIT_OBJECT_0)
	{
		bResult = GetOverlappedResult(m_handle_device,	
			&m_ov,		// Overlapped structure
			&bytes,		// Stores number of bytes sent
			TRUE); 			// Wait flag

		if (!bResult)
		{
			CloseDevice();
			return OP_ERR;
		}
		else
		{
			if (bytes != len)
			{
				PopErrMsg(-1);
				CloseDevice();
				return OP_ERR;
			}
			return OP_OK;
		}
	}
	else
	{
		CloseDevice();
		return OP_ERR;
	}


}

int RobotControlDevice::ReadResult(char *buffer, int *len)
{
	DWORD bytes = 0; *len = 0;
// 	if (!ReadFile(m_handle_device, buffer, REPORT_COUNT + 1, &bytes, NULL))
// 	{
// 		PopErrMsg(-2);
// 		return OP_ERR;
// 	}
// 	*len = bytes;
	//todo check lasys nor


	BOOL fWaitingOnRead = FALSE;
	m_ov.Offset = 0;
	m_ov.OffsetHigh = 0;


	bool bResult = ReadFile(m_handle_device, buffer, REPORT_COUNT + 1, &bytes, &m_ov);

	if (!bResult)
	{
		DWORD dwError = GetLastError();
		switch (dwError)
		{
		case ERROR_IO_PENDING:
		{
			// continue to GetOverlappedResults()
			break;
		}
		case ERROR_ACCESS_DENIED:///拒绝访问 erroe code:5
		{
			CloseDevice();
			LOGE(("HID ERROR_ACCESS_DENIED，ReadFile() Error Code:%d"), GetLastError());
			return OP_ERR;
		}
		case ERROR_INVALID_HANDLE:///打开串口失败 erroe code:6
		{
			CloseDevice();
			LOGE(("HID ERROR_INVALID_HANDLE，ReadFile() Error Code:%d"), GetLastError());
			return OP_ERR;
		}
		case ERROR_BAD_COMMAND:///连接过程中非法断开 erroe code:22
		{
			CloseDevice();
			LOGE(("HID ERROR_BAD_COMMAND，ReadFile() Error Code:%d"), GetLastError());
			return OP_ERR;
		}
		default:
		{
			// all other error codes
			LOGE(("HID DEFAULT，WriteFile() Error Code:%d"), GetLastError());
		}
		}

	}
	else
	{
		*len = bytes;
		return OP_OK;
	}

	DWORD wais = WaitForSingleObject(m_ov.hEvent, 2000);
	if (wais == WAIT_OBJECT_0)
	{
		bResult = GetOverlappedResult(m_handle_device,
			&m_ov,		// Overlapped structure
			&bytes,		// Stores number of bytes sent
			TRUE); 			// Wait flag

		if (!bResult)
		{
			CloseDevice();
			return OP_ERR;
		}
		else
			return OP_OK;
	}
	else
	{
		CloseDevice();
		return OP_ERR;
	}



	return OP_OK;
}

int RobotControlDevice::ResultSuccess(char *buf)
{
	if (buf[3] != 0x00)
	{
		PopErrMsg(buf[3]);
		return OP_ERR;
	}
	return OP_OK;
}

int RobotControlDevice::WriteAndRead(char * buf, int *readcount)
{
	if(OpenDevice())
		return DEV_ERR;

	if (WriteCommand(buf, REPORT_COUNT + 1))
		return OP_ERR;

	memset(buf, 0x00, REPORT_COUNT + 1);

	if (ReadResult(buf, readcount))
		return OP_ERR;

	return OP_OK;
}

int RobotControlDevice::WriteReadSimpleCall(char * buf, int *writeandreadcount)
{
	if (WriteAndRead(buf, writeandreadcount))
		return OP_ERR;

	if (!ResultSuccess(buf))
		return OP_ERR;

	return OP_OK;
}

int RobotControlDevice::ReadValueChangeLight()
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	BuildMotorCommand(buf, 0x22);

	if( WriteReadSimpleCall(buf, &readcount))
		return OP_ERR;

	float result_lx;
	unsigned short result = buf[4];
	result = (result << 8) + buf[5];
	result_lx = (float)result / 1.2;
	int k = 30;
	int n = log(result_lx);
	GammaRamp.SetBrightness(NULL, k*n);

	return OP_OK;
}

int RobotControlDevice::Set_Luminance(int luminance)
{
	LOGD("Set_Luminance");
	int k = 30;
	int n = log(luminance);
	GammaRamp.SetBrightness(NULL,k*n);

	LOGD("Set_Luminance end");
	return OP_OK;
}

//0x00,0x01,0x10,0x11,0x55,0x66
int RobotControlDevice::HeadControl(int type,int angle)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	char atype = 0x00;
	char angled = 0;
	char maxangle = 5;

	if (type == 0)
	{
		atype = 0x00;
		maxangle=40;
	}
	else if (type == 1)
	{
		atype = 0x01;
		maxangle = 40;
	}
	else if (type == 2)
	{
		atype = 0x10;
		maxangle = 5;
	}
	else if (type == 3)
	{
		atype = 0x11;
		maxangle = 15;
	}
	else if (type == 4)
	{
		atype = 0x55;
		maxangle = 40;
	}
	else if (type == 5)
	{
		atype = 0x66;
		maxangle = 5;
	}
	if (abs(angle) > maxangle)
		angled = angle% maxangle;

	BuildMotorCommand(buf, atype, angled);

	if (WriteReadSimpleCall(buf, &readcount))
		return OP_ERR;

	return OP_OK;
}

int RobotControlDevice::Head_AngleControl(int anglelevel, int anglevertical)
{
	char buf[REPORT_COUNT + 1] = {0};

	int readcount = 0;
	char cmd[5];
	cmd[0] = 0x77;

	if (anglelevel < 0)
		cmd[1] = 0x00;
	else
		cmd[1] = 0x01;

	if (abs(anglelevel) > 40)
		cmd[2] = abs(anglelevel) % 40;
	else
		cmd[2] = abs(anglelevel);

	if (anglevertical < 0)
	{
		cmd[3] = 0x01;
		if (abs(anglevertical) > 15)
			cmd[4] = abs(anglevertical) % 15;
		else
			cmd[4] = abs(anglevertical);
	}	
	else
	{
		cmd[3] = 0x00;
		if (abs(anglevertical) > 5)
			cmd[4] = abs(anglevertical) % 5;
		else
			cmd[4] = abs(anglevertical);
	}
	InsertHead(buf, cmd, 5);

	if (WriteReadSimpleCall(buf, &readcount))
		return OP_ERR;

	return OP_OK;
}

int RobotControlDevice::Head_GetCurrentAngle(int *anglelevel, int *anglevertical)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	BuildMotorCommand(buf, 0x44);

	if (WriteReadSimpleCall(buf, &readcount))
		return OP_ERR;	
	
	if (buf[4] > 180)
		*anglelevel = -(256 - buf[4]);
	else
		*anglelevel = buf[4];

	if (buf[5] > 180)
		*anglevertical = -(256 - buf[5]);
	else
		*anglevertical = buf[5];

	return OP_OK;
}

int RobotControlDevice::Head_Init()
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	BuildMotorCommand(buf, 0x33);

	if (WriteReadSimpleCall(buf, &readcount))
		return OP_ERR;

	return OP_OK;
}

int RobotControlDevice::Head_SetSpeed(int speed)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	char speedc = speed % 8;

	BuildMotorCommand(buf, 0xe0,speedc);

	if (WriteReadSimpleCall(buf, &readcount))
		return OP_ERR;

	return OP_OK;
}

int RobotControlDevice::Head_GetSpeed(int* speed)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	BuildMotorCommand(buf, 0xe1);

	if (WriteReadSimpleCall(buf, &readcount))
		return OP_ERR;

	*speed = buf[4];
	return OP_OK;
}

int RobotControlDevice::Head_GetRotateState(int *State)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	BuildMotorCommand(buf, 0xe2);

	if (WriteReadSimpleCall(buf, &readcount))
		return OP_ERR;

	*State = buf[4];//TODO  *State = reportBuf[3];//1为正在转动 2为转动完成
	return OP_OK;
}

int RobotControlDevice::Head_SetLight(int R, int G, int B)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	char cmd[4] = {0};

	cmd[0] = 0xd1;
	cmd[1] = R;
	cmd[2] = G;
	cmd[3] = B;

	InsertHead(buf, cmd,4);

	if (WriteReadSimpleCall(buf, &readcount))
		return OP_ERR;

	return OP_OK;
}

int RobotControlDevice::Head_CloseLight()
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	BuildMotorCommand(buf, 0xdf);

	if (WriteReadSimpleCall(buf, &readcount))
		return OP_ERR;

	return OP_OK;
}

std::string RobotControlDevice::get_error()
{
	return error_buffer;
}

char RobotControlDevice::GetMotorIndex(int id)
{
	if (id == 0)
		return  HEADLEVELMOTOR;
	if (id == 1)
		return   LEFTSHOULDERARM1MOTOR;
	else if (id == 2)
		return  RIGHTSHOULDERARM1MOTOTOR;
	else
		return OP_ERR;
}

bool RobotControlDevice::CheckRPS(char* buf,char cmd)
{
	if (buf[0] != (char)0x00 || buf[1] != (char)0xff || buf[2] != (char)0x77 || buf[4] != cmd)
		return false;

	char checksum = buf[64];
	char checksumt = CheckNOR((char *)buf + 4, buf[3]);
	if (checksum != checksum)
		return false;
	return true;
}

int RobotControlDevice::SM_Rotate(int id, int angle)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	short RotateAngle = 0;

	int type = 0;

	if (angle < 0)
		type = 1;
	else
		type = 0;
	RotateAngle = angle;
	char motorid = 0;

	motorid = GetMotorIndex(id);
	if (id <0)
		return OP_ERR;

	BuildMotorCommand(buf, 0x21, motorid, RotateAngle);

	int ret = WriteAndRead(buf, &readcount);
	if (ret)return ret;

	if (!CheckRPS(buf, 0x21))
	{
		printf("error checkrps\n");
		return OP_ERR;
	}


	if (buf[5] == motorid)
		return OP_OK;

	printf("error return : %d\n", buf[5]);
	return OP_ERR;
}

int RobotControlDevice::SM_Head_Rotate( int angle)
{
	return SM_Rotate(HEADLEVELMOTOR, angle);
}

int RobotControlDevice::SM_Arm_Rotate(int id, int angle)
{
	return SM_Rotate(id, angle);
}

int RobotControlDevice::SM_Motor_Stop(int id)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	char armnum = GetMotorIndex(id);
	if (id < 0)
		return OP_ERR;

	BuildMotorCommand(buf, 0x20, armnum);

	int ret = WriteAndRead(buf, &readcount);
	if (ret)return ret;

	if (!CheckRPS(buf, 0x20))
		return OP_ERR;

	if (buf[5] != armnum)
		return OP_ERR;

	return OP_OK;
}

int RobotControlDevice::SM_SetMotor_Speed(int id,int speed)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	char armnum = GetMotorIndex(id);
	if (id < 0)
		return OP_ERR;

	BuildMotorCommand(buf, 0x22, armnum, (char)(speed%7));

	int ret = WriteAndRead(buf, &readcount);
	if (ret)return ret;


	if (!CheckRPS(buf, 0x22))
		return OP_ERR;

	if (buf[5] != armnum)
		return OP_ERR;

	return OP_OK;
}

int RobotControlDevice::SM_SetDefaultAction(int acid)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	char armnum =0;

	BuildMotorCommand(buf, 0x23, armnum, (char)(acid));

	int ret = WriteAndRead(buf, &readcount);
	if (ret)return ret;


	if (!CheckRPS(buf, 0x23))
		return OP_ERR;

	return OP_OK;
}

int RobotControlDevice::SM_GetDefaultActionState(int *stats)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	char armnum = 0;

	BuildMotorCommand(buf, 0x24, armnum);

	int ret = WriteAndRead(buf, &readcount);
	if (ret)return ret;

	if (!CheckRPS(buf, 0x24))
		return OP_ERR;

	*stats = buf[6];

	return OP_OK;
}

int RobotControlDevice::SM_GetMotorState(int id, int *stats)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	char armnum = GetMotorIndex(id);
	if (id < 0)
		return OP_ERR;

	BuildMotorCommand(buf, 0x10, armnum);

	int ret = WriteAndRead(buf, &readcount);
	if (ret)return ret;

	if (!CheckRPS(buf, 0x10))
		return OP_ERR;

	if (buf[5] != armnum)
		return OP_ERR;

	*stats = (int)(buf[6]);

	return OP_OK;
}

int RobotControlDevice::SM_GetMotorRunState(int id, int *stats)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	char armnum = GetMotorIndex(id);
	if (id < 0)
		return OP_ERR;

	BuildMotorCommand(buf, 0x12, armnum);

	int ret = WriteAndRead(buf, &readcount);
	if (ret)return ret;

	if (!CheckRPS(buf, 0x12))
		return OP_ERR;

	if (buf[5] != armnum)
		return OP_ERR;

	*stats = buf[6];

	return OP_OK;
}

int RobotControlDevice::SM_GetMotorAngle(int id, int *angle)
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;
	*angle = 0;
	char armnum = GetMotorIndex(id);
	if (id < 0)
		return OP_ERR;

	BuildMotorCommand(buf, 0x11, armnum);

	int ret = WriteAndRead(buf, &readcount);
	if (ret)return ret;

	if (!CheckRPS(buf, 0x11))
		return OP_ERR;

	if (buf[5] != armnum)
		return OP_ERR;

	short angle_temp = 0;
	angle_temp = (((buf[7] << 8) & 0xff00) |( buf[6] & 0x00ff));
	*angle = angle_temp;

	return OP_OK;
}

int RobotControlDevice::SM_SetDefaultActionStop()
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	BuildMotorCommand(buf, 0x27, 0x00);

	int ret = WriteAndRead(buf, &readcount);
	if (ret)return ret;

	if (!CheckRPS(buf, 0x27))
		return OP_ERR;

	return OP_OK;
}

int RobotControlDevice::SM_SetSwingAngle(int frontangle,int backangle )
{
	char buf[REPORT_COUNT + 1] = { 0 };
	unsigned char cmd[5] = { 0 };
	int readcount = 0;

	cmd[0] = (unsigned char)0x30;
	cmd[1] = (unsigned char)0x00;
	cmd[2] = abs(frontangle) % 180;
	cmd[3] = abs(backangle) % 180;

	InsertHead(buf, (char*)cmd, 4);

	int ret = WriteAndRead(buf, &readcount);
	if (ret)return ret;

	if (!CheckRPS(buf, 0x30))
		return OP_ERR;

	return OP_OK;
}

int RobotControlDevice::SM_Reinit()
{
	char buf[REPORT_COUNT + 1] = { 0 };

	int readcount = 0;

	BuildMotorCommand(buf, 0xF0, 0x00);

	if (OpenDevice())
		return DEV_ERR;

	if (WriteCommand(buf, REPORT_COUNT + 1))
		return OP_ERR;

	CloseDevice();

	return OP_OK;

	//int ret = WriteAndRead(buf, &readcount); //only tianjin restart 
// 	if (ret)return ret;
// 
// 	if (!CheckRPS(buf, 0xF0))
// 		return OP_ERR;
// 
// 	return OP_OK;
}