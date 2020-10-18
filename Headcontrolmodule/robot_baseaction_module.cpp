#include "robot_baseaction_module.h"
#include "robot_PostMessage.h"
extern const char *MODULE_NAME;


robot_baseaction_module::robot_baseaction_module()
{
	isDefaultAction=false;
	runDefineAction = -1;
	isIniting = true;
}


robot_baseaction_module::~robot_baseaction_module()
{
}

void robot_baseaction_module::run()
{
	bool loop = true;
	timx = 150;
	update_initstate();

	while (loop)
	{
		Message msg = getq(timx);

		MotorTask * task = (MotorTask*)msg.msgObject;
		if (task)
		{		
			do_client_command(task);
			SAFE_DELETE(task);
			continue;
		}
		if (msg.msgType == _EXIT_THREAD_)
		{
			loop = false;
			continue;
		}
		if (isIniting)	
			update_initstate();
	
		synstatus();	
	}
	m_control.CloseDevice();
}

bool robot_baseaction_module::synstatus()
{
	bool hasError = false;
	if (isDefaultAction)
	{

	}
	else
	{
		for (auto it = m_motor_state.begin(); it != m_motor_state.end(); it++)
		{
			int currentangle = 0, devicestate = 0, runstate = 0;


			if (it->second.doaction)
			{
				if (m_control.SM_GetMotorState(it->first, &devicestate))
				{
					hasError = true;
					break;
				}
				m_motor_state[it->first].devicestate = devicestate;
				if (it->second.devicestate != 2)
				{
					it->second.doaction = false;
					std::string reps = m_json.rotatefinish(it->first, devicestate);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, reps.c_str(), reps.length());
				}
			}
			if (m_control.SM_GetMotorAngle(it->first, &currentangle) == 0)
				m_motor_state[it->first].currentangle = currentangle;
			else
			{
				hasError = true;
				break;
			}
		}
	}



	if (hasError)
	{
		timx += timx;
		isIniting = true;
	}	
	return hasError;
}

void robot_baseaction_module::update_initstate()
{
	bool last_init = isIniting;
	isIniting = false; int ret = 0;
	for (int i = 0; i < MAXMOTORCOUNT; i++)
	{
		int currentangle = 0, devicestate = 0, runstate = 0;
		ret=m_control.SM_GetMotorRunState(i, &runstate);
		if (ret < 0)
		{
			isIniting = true;
			break;
		}

		m_motor_state[i].runstate = runstate;
		if (runstate == 1)
			isIniting = true;
	}
	if (ret == DEV_ERR)
	{
		timx += timx;
		isIniting = true;
	}
	if (last_init == true && isIniting == false)
	{
		std::string reps = m_json.reinitfinish();
		robot_PostMessage::getPostManager()->post(MODULE_NAME, reps.c_str(), reps.length());
	}

}

void robot_baseaction_module::do_client_command(MotorTask * task)
{
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(task->context, json_object);
	if (!b_json_read)
		return;
	std::string reps;
	int ret = -1;
	switch (task->cmd)
	{
	case HEAD_ROTATE:
	{
		int rotatetype = 0, rotateangle=0;
		if (json_object.isMember("rotate_type") && json_object["rotate_type"].isInt())
			rotatetype = json_object["rotate_type"].asInt();
		if (json_object.isMember("rotate_angle") && json_object["rotate_angle"].isInt())
			rotateangle = json_object["rotate_angle"].asInt();
		if (!isIniting)
		{
			if (m_motor_state[HEADLEVELMOTOR].runstate == 3)
			{
				int anglecurrent = m_motor_state[HEADLEVELMOTOR].currentangle;
				if (rotatetype != 0)rotateangle *= -1;


				if (rotatetype != 0)// -
				{
					if ((anglecurrent + rotateangle) < -60)
						rotateangle = rotateangle + abs((anglecurrent + rotateangle) + 60);
				}
				else
				{
					if ((anglecurrent + rotateangle) > 60)
						rotateangle = rotateangle - abs((anglecurrent + rotateangle) - 60);
				}
				printf("fix head %d\n", rotateangle);
				ret = m_control.SM_Head_Rotate(rotateangle);
			}
			else
			{
				LOGD("curent head state :%d\t angle :%d \n", m_motor_state[HEADLEVELMOTOR].runstate, m_motor_state[HEADLEVELMOTOR].currentangle);
			}
		}
		else
			ret = 9210;
		
		reps = m_json.head_rotate2(ret);	
		if (ret == 0)m_motor_state[HEADLEVELMOTOR].doaction = true;
	}
	break;
	case ARM_ROTATE:
	{
		int rotatetype = 0, rotateangle = 0, id = 1;

		if (json_object.isMember("id") && json_object["id"].isInt())
			id = json_object["id"].asInt();
		if (json_object.isMember("rotate_type") && json_object["rotate_type"].isInt())
			rotatetype = json_object["rotate_type"].asInt();
		if (json_object.isMember("rotate_angle") && json_object["rotate_angle"].isInt())
			rotateangle = json_object["rotate_angle"].asInt();
		if (!isIniting)
		{
			if (m_motor_state[id].runstate == 3)
			{
				if (rotatetype != 0)rotateangle *= -1;
				ret = m_control.SM_Arm_Rotate(id, rotateangle);
			}
			else
			{
				LOGD("curent arm left state :%d\t angle :%d\t arm right state :%d\t angle :%d \n",
					m_motor_state[LEFTSHOULDERARM1MOTOR].runstate, m_motor_state[LEFTSHOULDERARM1MOTOR].currentangle,
					m_motor_state[RIGHTSHOULDERARM1MOTOTOR].runstate, m_motor_state[RIGHTSHOULDERARM1MOTOTOR].currentangle);
			}
		}else
			ret = 9210;

		reps = m_json.arm_rotate(ret);
		if (ret == 0)m_motor_state[id].doaction = true;
	}
		break;
	case HEAD_GETANGLE:
	{
		int angle = 0;
		ret = m_control.SM_GetMotorAngle(HEADLEVELMOTOR,&angle);
		m_motor_state[HEADLEVELMOTOR].currentangle = angle;
		reps = m_json.head_getrotate(ret, angle);
	}
		break;
	case ARM_GETANGLE:
	{
		int angle=0, id = 1;
		if (json_object.isMember("id") && json_object["id"].isInt())
			id = json_object["id"].asInt();
		//LOGD("1");
		ret = m_control.SM_GetMotorAngle(id, &angle);
		//LOGD("2");
		m_motor_state[id].currentangle = angle;
		reps = m_json.arm_getrotate(ret, id, angle);
	}
		break;
	case ROBOT_DO_ACTION:
	{
		int actionid = 1;
		if (json_object.isMember("actionid") && json_object["actionid"].isInt())
			actionid = json_object["actionid"].asInt();

		if (!isIniting)
		{
			ret = 0;
			if (actionid == 0x30)
				ret = init_action_armswing();

			if (ret==0)
				ret = m_control.SM_SetDefaultAction(actionid);

		}
		reps = m_json.do_defaultaction(ret);
	}
		break;
	case HEAD_STOP:
	{
		if (!isIniting)
		{
			if (m_motor_state[HEADLEVELMOTOR].runstate == 3)
				ret = m_control.SM_Motor_Stop(HEADLEVELMOTOR);
		}
		else
			ret = 9210;

		reps = m_json.do_headstop(ret);
		if (ret == 0)m_motor_state[HEADLEVELMOTOR].doaction = false;
	}
		break;
	case ARM_STOP:
	{
		int angle = 0, id = 1;
		if (json_object.isMember("id") && json_object["id"].isInt())
			id = json_object["id"].asInt();

		if (!isIniting)
		{
			if (m_motor_state[id].runstate == 3)
				ret = m_control.SM_Motor_Stop(id);
		}
		else
			ret = 9210;
		reps = m_json.do_armstop(ret, id);
		if (ret == 0)m_motor_state[id].doaction = false;
	}
		break;
	case CHANGE_SPEED:
	{
		int level = 0, id = 1;
		if (json_object.isMember("id") && json_object["id"].isInt())
			id = json_object["id"].asInt();
		if (json_object.isMember("level") && json_object["level"].isInt())
			level = json_object["level"].asInt();
		ret = m_control.SM_SetMotor_Speed(id, level);
		reps = m_json.change_speed(ret, id);
	}break;
	case ROBOT_STOP_ACTION:
	{
		ret = m_control.SM_SetDefaultActionStop();
		if (ret==0)
			ret = wait_action_stopping();//todo only check arm ? should add head check
		reps = m_json.stop_action(ret);
	}break;
	case SET_SWING_ANGLE:
	{
		int fangle = 0, bangle =0 ;
		if (json_object.isMember("frontangle") && json_object["frontangle"].isInt())
			fangle = json_object["frontangle"].asInt();
		if (json_object.isMember("backangle") && json_object["backangle"].isInt())
			bangle = json_object["backangle"].asInt();
		ret = m_control.SM_SetSwingAngle(fangle, bangle);
		reps = m_json.setswing(ret);
	}break;
	case ROBOT_REINIT:
	{
		ret = m_control.SM_Reinit();
		reps = m_json.reinit(ret);
		if (reps.length())robot_PostMessage::getPostManager()->post(MODULE_NAME, reps.c_str(), reps.length(), task->from.c_str());
		common_thread_sleep(100);
		isIniting = true;
		update_initstate();
		if (isIniting)timx = 150;
		reps.clear();
		return;
	}
	default:
		break;
	}
	if (ret == 0 && m_control.isOpened())timx = 10;
	if (reps.length())robot_PostMessage::getPostManager()->post(MODULE_NAME, reps.c_str(), reps.length(), task->from.c_str());
}

int robot_baseaction_module::wait_action_stopping()
{
	int maxcount = 50;
	int ret = 0;
	while (true)
	{
		common_thread_sleep(100);
		int angle[2] = { 0 };
		int statetemp[2] = { 0 };

		for (int i = 1; i < 3; i++)
		{
			ret = m_control.SM_GetMotorAngle(i, &angle[i - 1]);
			if (ret != 0)break;
			ret = m_control.SM_GetMotorState(i, &statetemp[i - 1]);
			if (ret != 0)break;
		}
		if (ret != 0)
			break;
		if ((statetemp[0] != 2 && statetemp[1] != 2))
		{
			if (angle[0] >= 18 && angle[0] <= 22 || angle[1] >= 18 && angle[1] <= 22)
				ret = 0;
			else
				ret = -900;
			break;
		}
		if (maxcount-- <= 0)
			break;
	}		
	return ret;
}


int robot_baseaction_module::fix_angle_toinit(int ang)
{
	if (ang < 0 && ang >= -160)
		return abs(ang) + 20;
	else
	{
		if (ang < 0)ang = 180 + (180 - abs(ang));
		return  20-ang;
	}
}

int robot_baseaction_module::init_action_armswing()
{
	int maxcount = 50;
	int ret = 0;

	int angle[2] = { 0 };
	int statetemp[2] = { 0 };

	for (int i = 1; i < 3; i++)
	{
		ret = m_control.SM_Motor_Stop(i);
		if (ret != 0)break;
		int maxwaitcount = 30;
		while (1)
		{
			common_thread_sleep(20);
			ret = m_control.SM_GetMotorState(i, &statetemp[i - 1]);
			if (ret != 0)break;
			if (statetemp[i - 1] !=2)
				break;
			if (maxwaitcount-- <= 0)
				return -902;
		}
		if (ret != 0)break;
		ret = m_control.SM_GetMotorAngle(i, &angle[i - 1]);
		if (ret != 0)break;

	}
	if (ret != 0)
		return ret;

	if ((statetemp[0] == -1 || statetemp[1] == -1))
		return  -901;

	angle[0]=	fix_angle_toinit(angle[0]);
	angle[1] = fix_angle_toinit(angle[1]);

	ret = m_control.SM_Arm_Rotate(1, angle[0]);
	if (ret)return ret;
	ret = m_control.SM_Arm_Rotate(1, angle[0]);
	if (ret)return ret;

	while (true)
	{		
		common_thread_sleep(20);
		for (int i = 1; i < 3; i++)
		{
			ret = m_control.SM_GetMotorState(i, &statetemp[i - 1]);
			if (ret != 0)break;
		}
		if (ret != 0)break;

		if ((statetemp[0] != 2 && statetemp[1] != 2))
			break;
		if (maxcount-- <= 0)
			break;
	}
	return ret;
}