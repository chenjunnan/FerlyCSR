#pragma once
#include "common_helper.h"
#include "robot_json_rsp.h"
#include "RobotControlDevice.h"


enum ACTIONCOMMAND
{
	UNCOMMAND=-1,
	HEAD_ROTATE=0,
	ARM_ROTATE,
	HEAD_GETANGLE,
	ARM_GETANGLE,
	ROBOT_DO_ACTION,
	HEAD_STOP,
	ARM_STOP,
	CHANGE_SPEED,
	ROBOT_STOP_ACTION,
	SET_SWING_ANGLE,
	ROBOT_REINIT
};


struct MotorTask 
{
	ACTIONCOMMAND cmd = UNCOMMAND;
	int timeout=2;
	std::string context;
	std::string from;
};

const int MAXMOTORCOUNT=  3 ;

struct MotorState
{
	int currentangle=0;
	int devicestate=0;//-1 err 2 run 1 stop
	int runstate=3 ;//1 zeroing 2 running 3 done 4 limit
	bool doaction=false;
};


class robot_baseaction_module :
	public Thread_helper
{
public:
	robot_baseaction_module();
	virtual ~robot_baseaction_module();

private:
	virtual void run();

	bool synstatus();

	void update_initstate();

	void do_client_command(MotorTask * task);

	int  wait_action_stopping();
	int  fix_angle_toinit(int ang);
	int  init_action_armswing();
	robot_json_rsp m_json;
	RobotControlDevice m_control;

	bool isDefaultAction;
	int  runDefineAction;

	bool isIniting;
	int timx;
	std::map<int, MotorState> m_motor_state;

};

