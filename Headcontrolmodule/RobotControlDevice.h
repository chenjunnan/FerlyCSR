#ifndef __FL_HEAD_CONTROL_H__
#define __FL_HEAD_CONTROL_H__

#include <windows.h>
#include <string>
#include "gammaramp.h"

#define OP_OK				 0
#define OP_ERR				-1
#define DEV_ERR				-2

enum MOTOR_TYPE
{
	HEADLEVELMOTOR,
	LEFTSHOULDERARM1MOTOR,
	RIGHTSHOULDERARM1MOTOTOR,
};

class RobotControlDevice
{
public:
	RobotControlDevice();
	~RobotControlDevice();

public:
	int CloseDevice();
	bool isOpened();
	int ReadValueChangeLight();
	int Set_Luminance(int luminance);
	int HeadControl(int type,int angle);
	int Head_AngleControl(int anglelevel,int anglevertical);
	int Head_GetCurrentAngle(int *anglelevel,int *anglevertical);
	int Head_Init();
	int Head_SetSpeed(int speed);
	int Head_GetSpeed(int *speed);
	int Head_GetRotateState(int *State);
	int Head_SetLight(int R,int G,int B);
	int Head_CloseLight();

	std::string get_error();

	int SM_Head_Rotate( int angle);
	int SM_Arm_Rotate(int id, int angle);
	int SM_Motor_Stop(int id);
	int SM_SetMotor_Speed(int id, int speed);
	int SM_SetDefaultAction(int acid);
	int SM_GetDefaultActionState(int *stats);
	int SM_SetDefaultActionStop();
	int SM_SetSwingAngle(int frontangle, int backangle);
	int SM_Reinit();
	int SM_GetMotorState(int id, int *stats);
	int SM_GetMotorRunState(int id, int *stats);
	int SM_GetMotorAngle(int id, int *angle);
	
private:
	int OpenDevice();
	int SM_Rotate(int id,  int angle);
	char GetMotorIndex(int id);
	bool CheckRPS(char* buf, char cmd);
	char CheckNOR(char *Buf, int Len);
	void InsertHead(char *buffer, char * cmd, unsigned char cmdlen);
	int  WriteAndRead(char * buf, int *readcount);
	int  WriteReadSimpleCall(char * buf, int *writeandreadcount);
	int ResultSuccess(char *buf);
	void BuildMotorCommand(char *buffer, unsigned char cmdtype, unsigned char motor, char value);
	void BuildMotorCommand(char *buffer, unsigned char cmdtype, unsigned char motor, short value);
	void BuildMotorCommand(char *buffer, unsigned char cmdtype, unsigned char motor);
	void BuildMotorCommand(char *buffer, unsigned char cmdtype);

	int WriteCommand(char *buffer, int len);
	int ReadResult(char *buffer, int *len);
	HANDLE m_handle_device;
/*	BYTE recvDataBuf[65], reportBuf[65];*/
	CGammaRamp GammaRamp;
	char error_buffer[4096];
	void PopErrMsg(int ret);
	OVERLAPPED			m_ov;


};

#endif