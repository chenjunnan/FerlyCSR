#include "NavigationImp.h"
#include <stdio.h>
#include <windows.h>
#include <complex>
#include <time.h>




//0 find mag
//1 >0.6 run
//2 >0.15 run
//3 >0 run
//4 runline reurn mag <15
//5 check angle 
//6 change angle 
//7 runline 0

//#define CENTER_OFFSER 17
 int CENTER_OFFSER = 17;

NavigationImp::NavigationImp()
{
	m_cl = NULL;
	step = 0;
	ok_count = 0;
	last_tims = 0;
	fsum = 0;
	bsum = 0;
	center = 0;
	OriSpeed = 1;
	fromx = 0;
	fromy = 0;
	fromangle = 0;
	findmemsstep = 0;
	timesmove = 0;
	LEVEL = 0;
	rotate_step = 0;
	angle_rotate = 0;
	rotate_speed = 0;
	dis_poie = 0.0;
}


void NavigationImp::Init(int Level)
{
	LEVEL = Level;
	if (LEVEL)
		CENTER_OFFSER = 84;

	int dis=::GetPrivateProfileInt("MASE", "search", 25,"maseconfig.ini");

	int logonoff = ::GetPrivateProfileInt("MASE", "log", 0, "maseconfig.ini");
	
	common_log_config(MODULE_NAME, false, true, logonoff);

	dis_poie = ((double)dis / 100.0);

	maybe_end_try_count = 3;
	max_try_find_count = 3;
	failed_runtoend = false;

}

void NavigationImp::setController(Controllerinterface *handler)
{
	m_cl = handler;
}

int NavigationImp::Handle(MagneticData *data)
{
	if (LEVEL)
	{
		if (data->bpos > 2)
			data->bpos =168- data->bpos;
		if (data->fpos > 2)
			data->fpos = 168 - data->fpos;
	}
	m_sensordata = *data;
	
	//旋转任务，等待结束
	if (rotate_step)
	{	
		if (rotate_step == 1)
		{
			LOGMD("rotate begin %0.5f", angle_rotate);
			if (m_cl->iRotate(angle_rotate, rotate_speed) != 0)
			{
				Sleep(100);
				return 100;
			}

			rotate_step = 2;
			return 100;
		}
		if (rotate_step == 2)
		{
			printf("wait %0.5f rotate.\n", angle_rotate);
			int waitstate = m_cl->iRotateWait();
			if (waitstate == -1)
				return 101;
			if (waitstate == 1)
			{
				Sleep(200);
				return 100;
			}
			LOGMD("rotate finish");
			printf("rotate finish  %d.\n", waitstate);
			rotate_step = 0;
			angle_rotate = 0;
			rotate_speed = 0;
		}
	}

	if (step == 0)
	{
		printf("fpos:%d\tbpos:%d\n", m_sensordata.fpos, m_sensordata.bpos);
		return SelfFindMode();

	}

	if (step == 1)
	{
		if (m_sensordata.bpos == 0 && m_sensordata.fpos == 0)
			step = 0;
		else
		{
			double ids = getDistance(m_sensordata);
			if (ids < 0)return 101;
			ChangeOri(ids);
			
			if (ids < 0.30&&OriSpeed==1)
				step = 3;
			else
			{
				if (maybe_end_try_count <= 0)
					step = 8;
				else
					return NormalMove(ids);
			}		
		}
	}
	if (step == 2)
	{

	}
	if (step == 3)
	{
		double ids = getDistance(m_sensordata);
		if (ids < 0)return 101;

		if ((OriSpeed == 1 && m_sensordata.fpos == 0 && ids < 0.15) || (maybe_end_try_count <= 0 && m_sensordata.fpos == 0))
		{
			if (ids > 0.4)
			{
				LOGMD("position error");
				step = 8;
			}
			else
			{
				m_cl->iMove(-0.01, 0);
				LOGMD("mission finish");
				printf("end mesms\n");
				return 0;//todo
				// 			if (m_sensordata.bpos != 0)
				// 				step = 4;
				// 			else
				// 				return 0;
			}

		}
		else
			return NormalMove(ids);
	}
	if (step == 8)
	{
		double ids = getDistance(m_sensordata);
		if (ids < 0)return 101;

		OriSpeed = 0;

		if (m_sensordata.bpos == 0 )
		{
			m_cl->iMove(0.1, 0);
			LOGMD("mission failed");
			return 102;
		}
		else
			return NormalMove(ids);
	}
// 	if (step == 4)
// 	{
// 		if (RunLine( true))
// 			step = 5;
// 		else
// 			return 100;
// 	}
// 
// 	if (step == 5)
// 		return CheckAngle();
// 
// 	if (step == 6)
// 		return AdjustAngle();
// 
// 	if (step == 7)
// 	{
// 		if (RunLine(false))
// 			return 0;
// 		else
// 			return 100;
// 	}
	return 100;
}

int NavigationImp::SelfFindMode()
{
	if (m_sensordata.bpos != 0 || m_sensordata.fpos != 0)
	{
		if (findmemsstep != 0)
		{
			LOGMD(" find position info ");
			m_cl->iMove(0.08, 0);
			Sleep(500);
			m_cl->iMove(0.08, 0);
			Sleep(500);
			PositionStc p;
			if (m_cl->iPosition(&p) < 0)
				return 101;
			StandbyRotate(p);
			findmemsstep = 0;

			double dss = CheckFindDistance();
			if (dss > (dis_poie + 0.10) || dss < 0)
			{
				LOGMD("long distance failed");
				return 101;
			}
			
			return 100;
		}
		maybe_end_try_count = 3;
		step = 1;
		return 100;
	}
	else
	{
		if (max_try_find_count <= 0)
			findmemsstep = 5;

		if (findmemsstep == 0)
		{
			printf("no position info ..\n");
			PositionStc p;
			if (m_cl->iPosition(&p) < 0)return 101;
			fromx = p.wx;
			fromy = p.wy;
			fromangle = p.angle;
			LOGMD(" no position info ,will find mase .now:x:%0.5f,y:%0.5f,r:%0.5f,to:%0.5f,y:%0.5f,r:%0.5f", fromx, fromy, fromangle, m_sensordata.desx, m_sensordata.desy, m_sensordata.desangle);
			StandbyRotate(p);
			findmemsstep = 1;
			max_try_find_count--;
			return 100;

		}
		if (findmemsstep == 1)
		{
			rotate_step = 1;
			angle_rotate = 90;
			rotate_speed = 0.4;
			findmemsstep = 2;
			LOGMD("find step 1 rotate 90");
			return 100;
		}
		double dss = CheckFindDistance();
		printf("%d fpos:%d\tbpos:%d dis :%0.4f\n", findmemsstep, m_sensordata.fpos, m_sensordata.bpos, dss);
		if (dss > (dis_poie + 0.10) || dss < 0)
		{
			LOGMD("long distance failed");
			return 101;
		}

		if (findmemsstep == 2)
		{
			if (dss < dis_poie)
			{
				m_cl->iMove(0.08, 0);
				Sleep(200);
				return 100;
			}
			else
			{
				rotate_step = 1;
				angle_rotate = 180;
				rotate_speed = 0.4;
				findmemsstep = 3;
				LOGMD("find step 2 rotate 180");
				return 100;
			}
		}
		if (findmemsstep == 3)
		{
			if (dss < (dis_poie-0.05))
				findmemsstep = 4;
			else
			{
				if (dss > (dis_poie + 0.10))		
				{
					LOGMD("long distance failed");
					return 101;
				}
				m_cl->iMove(0.08, 0);
				Sleep(200);
				return 100;
			}
		}
		if (findmemsstep == 4)
		{
			if (dss < dis_poie)
			{
				m_cl->iMove(0.08, 0);
				Sleep(200);
				return 100;
			}
			else
			{
				PositionStc p;
				if (m_cl->iPosition(&p) < 0)return 101;
				StandbyRotate(p);
				findmemsstep = 5;
				return 100;
			}
		}
		if (findmemsstep == 5)
		{
			LOGMD("mission failed");
			findmemsstep = 0;
			return 102;
		}
			
	}
	return 101;
}

void NavigationImp::StandbyRotate(PositionStc &p)
{
	int angle1 = m_sensordata.desangle - p.angle;
	if (angle1 > 180)
	{
		angle1 = 360 - angle1;
		angle1 *= -1;
	}
	if (angle1 < -180)
		angle1 = 360 + angle1;

	rotate_step = 1;
	angle_rotate = angle1;
	rotate_speed = 0.4;
	LOGMD("will fix angle:%0.5f", angle_rotate);
}

double NavigationImp::CheckFindDistance()
{
	MagneticData temp;
	temp.desx = fromx; temp.desy = fromy;
	return getDistance(temp);
}

void NavigationImp::ChangeOri(double ids)
{
	if (ids > 0.3)
	{
		int toUp = 0, toDown = 0;
		CalcOffset(CENTER_OFFSER, m_sensordata, toUp, toDown);

		int diff = abs(toUp - toDown);

		bool isok = false;
		int sum = toDown + toUp;
		int default_toUpDown = 5,default_angle=4;

		if (LEVEL)
		{
			default_toUpDown = 20;
			default_angle = 12;
		}

		if (m_sensordata.angle < default_angle && toUp < default_toUpDown && toDown < default_toUpDown)
			isok = true;

		if (ids < 0.53&&!isok)
			OriSpeed = 0;
		else
		{
			if (OriSpeed == 0)
			{
				if (ids > 0.9/* 1.1*/ || ids > 0.65 && isok)
				{
					LOGMD("change ori speed");
					OriSpeed = 1;
				}
			}
		}
	
	}
}

int NavigationImp::NormalMove(double ids)
{
	double als = 0, aas = 0;
	if (OriSpeed == 0)
		als = 0.1;
	else
		als = -0.1;
	unsigned char fpos = 0;
	unsigned char bpos = 0;
	int   angle = 0;


	LOGMD("front :%d\t back : %d \t angle:%d", m_sensordata.fpos, m_sensordata.bpos, m_sensordata.angle);
	int ret=getSpeed(CENTER_OFFSER, ids, m_sensordata, als, aas);
	LOGMD("ret :%d  quadrant:%d distobox %.3f to: ls:%.3f rs:%.3f", ret, m_sensordata.quadrant, ids, als, aas);
	printf("ret :%d fpos:%d\tbpos:%d\tangle:%d  quadrant:%d distobox %.3f to: ls:%.3f rs:%.3f\n",ret, m_sensordata.fpos, m_sensordata.bpos, m_sensordata.angle, m_sensordata.quadrant, ids, als, aas);
	if (ret == 0||ret==1&&ids>0.85)
	{	
		int ret = m_cl->iMove(als, aas);
		if (ids < 0.10)
			Sleep(400);
		return 100;
	}
	else
	{
		if (ret == 1)
		{
			if (step != 8)
			{
				if (OriSpeed == 0)
					OriSpeed = 1;
				else
					OriSpeed = 0;

				if (OriSpeed == 0 && m_sensordata.fpos==0)
					maybe_end_try_count--;

				step = 1;
			}
			
			LOGMD("change ori %d", OriSpeed);
			return 100;
		}
		if (ret == 2)
		{		
			if (ids > 0.6)
			{
				LOGMD("will find");
				step = 0;
				return 100;
			}
			else
				return 101;
		}
		

	}
	return 100;
}


NavigationImp::~NavigationImp()
{

}

int NavigationImp::CheckAngle()
{
	if (m_sensordata.fpos == 0)
	{
		step = 4;
		ok_count = 0;
		fsum = 0; bsum = 0;
		return 2;
	}

	if (last_tims != m_sensordata.times)
	{
		double llike_value = 0, rlike_value = 0;
		fsum += m_sensordata.fpos;
		bsum += m_sensordata.bpos;
		ok_count++;

		if (ok_count == 0)
		{
			center = ((double)((m_sensordata.bpos + m_sensordata.fpos))) / 2;
			if (center > 18)center = 18;
			if (center < 16)center = 16;
		}
		else
		{
			llike_value = ((double)fsum) / ((double)ok_count);
			rlike_value = ((double)bsum) / ((double)ok_count);
			center = ((double)((llike_value + rlike_value))) / 2;
		}

		// 				int toUp = 0, toDown = 0;
		// 				calc_offset(center, temp, toUp, toDown);
		// 				sum += toUp;
		// 				sum += toDown;
		if (ok_count >= 50)
		{

			double maxth = ((double)center + 0.8);
			double minth = ((double)center - 0.3);
			double center_d = CENTER_OFFSER;
			if (center < (center_d+0.1))maxth += 0.3;
			if (center >(center_d-0.1)){
				minth -= 0.5;
				maxth -= 0.3;
			}

			printf("center :%.5f to: lv:%.3f rv:%.3f\n", center, llike_value, rlike_value);
			if (
				// 						llike_value > (minth + 0.1) && llike_value<(maxth + 0.1) &&
				// 						rlike_value>minth && rlike_value < maxth &&
				(std::abs(llike_value - rlike_value) < 0.3))
			{
				step = 7;
				return 100;
			}

			else
			{
				step = 6;
				return 100;
			}
				
		}

	}
	Sleep(20);
	return 100;
}


int NavigationImp::getSpeed(int center, double dis, MagneticData &temp, double &als, double &aas)
{
	int toright = -1;
	int toleft = 1;
	double mu = /*100;//*/ 50;// 25;
	int value1 = 5;
	int value2 = 2;
	if (LEVEL)
	{
		mu = /*200*//*350*/100;
		value1 = 30;//同边角度判断阈值
		value2 = 12;//差距大小判断阈值
	}
		
	int toUp = 0, toDown = 0;
	int change_ori=0;

	int is_oneside = 0;

	aas = 0;
	CalcOffset(center, temp, toUp, toDown);

	int diff = abs(toUp - toDown);
	int diffsum = toUp + toDown;
	if (toUp > toDown)
	{
		if (temp.fpos > center)
			temp.quadrant = 2;
		else
			temp.quadrant = 1;
	}
	else if (toDown > toUp)
	{
		if (temp.bpos > center)
			temp.quadrant = 4;
		else
			temp.quadrant = 3;
	}
	else
		temp.quadrant = 0;

	if (toDown == toUp)
	{
		if (temp.bpos == temp.fpos)
		{
			if (temp.bpos > center)
				temp.quadrant = 2;
			else if (temp.bpos < center)
				temp.quadrant = 1;
			else
				temp.quadrant = 0;
		}
		else
			temp.quadrant = 0;
	}
	if (temp.fpos == 0)
		temp.quadrant = 5;
	if (temp.bpos == 0)
		temp.quadrant = 5;

	if (temp.fpos == 0 && temp.bpos == 0)
	{
		als = 0; aas = 0;
		return 2;
	}


	if (temp.quadrant == 0)
	{
		if (temp.bpos == temp.fpos&&temp.bpos == center)
		{
			als *= 1;
			aas = 0;
		}
		else
		{
			als /= 5;
			if (temp.fpos > center)
				aas = (double)(temp.fpos - center) / mu;

			else
				aas = (double)(center - temp.fpos) / mu;
			
			if (temp.fpos > center)
				aas *= toright;
			else
				aas *= toleft;
		}
	}
	else
	{

// 		if (diff < 4 && toUp>8 && toDown > 8)
// 		{
// 			int stoUp = (toUp < toDown ? toUp : toDown);
// 			temp.angle += stoUp;
// 		}
		aas = (double)(diffsum) / (mu*2);

		//aas = (double)(temp.angle) / (double)(mu /*/ toUp*/);

		if (temp.fpos > center && temp.bpos > center)
			is_oneside = 1;
		if (temp.fpos < center && temp.bpos < center && temp.fpos != 0 && temp.bpos != 0)
			is_oneside = 2;
		if (temp.fpos == 0)
			is_oneside = 3;
		if (temp.bpos == 0)
			is_oneside = 4;

		if (is_oneside == 0)
		{
			if (temp.quadrant == 2 || temp.quadrant == 3)
			{
				aas *= toright;
			}
			else
			{
				aas *= toleft;
			}
		}
		if (is_oneside == 1)
		{
			if (temp.quadrant == 2)
			{
				if (als < 0)
				{
					aas *= toright;
					aas *= 1;
				}
				else
				{
					if (diff > value1)
					{
						aas *= 0.2;
						aas *= toright;
					}
					else
					{
						aas *= 0.6;
						aas *= toleft;

					}

				}


			}
			if (temp.quadrant == 4)
			{
				if (als < 0)
				{
					if (diff > value1)
					{
						aas *= 0.2;
						aas *= toleft;
					}
					else
					{
						aas *= 0.6;
						aas *= toright;

					}
				}

				else
				{
					aas *= 1;
					aas *= toleft;
				}

			}

		}
		if (is_oneside == 2)
		{
			if (temp.quadrant == 1)
			{
				if (als < 0)
				{
					aas *= 1;
					aas *= toleft;
				}
				else
				{
					if (diff > value1)
					{
						aas *= 0.2;
						aas *= toleft;
					}
					else
					{
						aas *= 0.6;
						aas *= toright;

					}
				}


			}
			if (temp.quadrant == 3)
			{
				if (als < 0)
				{
					if (diff > value1)
					{
						aas *= 0.2;
						aas *= toright;
					}
					else
					{
						aas *= 0.6;
						aas *= toleft;

					}
				}

				else
				{
					aas *= 1;
					aas *= toright;
				}


			}
		}

		if (is_oneside == 3)
		{

			if (temp.bpos > center)
			{
				if (als > 0)
				{
					aas = 0.1;
					aas *= toleft;
				}
				else
				{
					
					aas = 0.5;
					aas *= toright;
					als *= 2;
					change_ori = 1;
				}

			}
			else 
			{			
				if (als > 0)
				{
					if (temp.bpos < center)
					{
						aas = 0.1;
						aas *= toright;
					}
					else
						aas = 0;
				}
				else
				{
					aas = 0.5;
					aas *= toleft;
					als *=2;
					change_ori = 1;
				}

			}

		}
		if (is_oneside == 4)
		{
			if (temp.fpos > center)
			{
				if (als > 0)
				{
					aas = 0.5;
					aas *= toleft;
					als *= 2;
					change_ori = 1;
				}
				else
				{
					aas = 0.1;
					aas *= toright;
				}
			}
			else
			{
				if (als > 0)
				{
					aas = 0.5;
					aas *= toright;
					als *= 2;
					change_ori = 1;
				}
				else
				{
					if (temp.fpos < center)
					{
						aas = 0.1;
						aas *= toleft;
					}
					else
						aas = 0;


				}
			}
		}
	}	
	if (aas > 0.6)aas = 0.6;
	if (aas < -0.6)aas = -0.6;

	if (dis < 0.50&&/*als<0 &&*/ abs(aas)>0.2)
	{
		if (change_ori == 1)
		{
			double tempaas = abs(((double)toDown) / 164.0);
			if (aas < 0)
				aas = tempaas*(-0.1);
			if (aas > 0)
				aas = tempaas;
		}

		while (abs(aas) > 0.2)
			aas /= 1.5;
	}

	if (dis < 0.3)
	{
		while (abs(aas) > 0.1)
			aas /= 2;
	}


	if (dis < 0.20)
	{
		if (abs(als) >= 0.1)
			als /= 5;

		if (temp.quadrant == 5)
		{
			aas = 0;
		}
		else
		{
			if (als != 0)
			{
				while (abs(aas)>abs(als))
					aas /= 2;
			}
		}
		if (diff<value2 && abs(aas)>0.03)
			aas /= 2;

	}



	return change_ori;
}


double NavigationImp::getDistance(MagneticData &point)
{
	PositionStc current_position;
	double distobox = -1;
	if (m_cl->iPosition(&current_position) == 0)
		distobox = sqrt(((current_position.wx - point.desx)*(current_position.wx - point.desx) +
		(current_position.wy - point.desy)*(current_position.wy - point.desy)));
	else
		return -101;
	return distobox;
}

int NavigationImp::AdjustAngle()
{
	if (m_sensordata.fpos == 0)
	{
		step = 4;
		ok_count = 0;
		fsum = 0; bsum = 0;
		return 100;
	}
	MagneticData temp = m_sensordata;
	srand(time(NULL));
	double llike_value = 0, rlike_value = 0;
	llike_value = ((double)fsum) / ((double)ok_count);
	rlike_value = ((double)bsum) / ((double)ok_count);

	ok_count = 0;
	fsum = 0; bsum = 0;
	step = 5;
	double als = 0, aas = 0;
	double distobox = getDistance(m_sensordata);
	if (distobox<0)return 101;

	center = ((double)((llike_value + rlike_value))) / 2;
	// 			if (center > 17.5)center = 17.5;
	// 			if (center < 16.5)center = 16.5;
	center += 0.5;
	if (center > 18)center = 18;
	if (center < 16)center = 16;

	if (rand() % 7 == 1)
		als = 0.02;
	else
		als = 0.01;

	if (distobox > 0.15 || (rand() % 2 == 0))
	{
		als *= -1;
	}
	if ((rand() % 5 >= 2))
		als = 0;

	temp.fpos = (int)(llike_value + 0.5);
	temp.bpos = (int)(rlike_value + 0.5);

	if (temp.fpos == temp.bpos)
	{
		temp.fpos = (int)(llike_value - 0.5);
		temp.bpos = (int)(rlike_value - 0.5);
	}

	temp.angle = (temp.fpos - temp.bpos) == 0 ? 1 : abs(temp.fpos - temp.bpos);
	// 			if (als >= 0)als = -0.02;
	// 			if (als < 0)als = 0.02;
	int ret = getSpeed((int)center, distobox, temp, als, aas);
	if (ret == 0)
	{
		printf("fpos:%d\tbpos:%d\tangle:%d  quadrant:%d distobox %.3f to: ls:%.3f rs:%.3f\n", m_sensordata.fpos, m_sensordata.bpos, m_sensordata.angle, m_sensordata.quadrant, distobox, als, aas);
		int ret = m_cl->iMove(als, aas);
		return 100;
	}
	return 101;
}

bool NavigationImp::RunLine(bool reach)
{
	double ls = 0;
	m_cl->iMove(0.0, 0.0);
	printf("find position :fpos:%d\tbpos:%d\n", m_sensordata.fpos, m_sensordata.bpos);
	if (reach)
	{
		if (m_sensordata.fpos == 0)
			ls = 0.01;
		else
			return true;
	}
	else
	{
		if (m_sensordata.fpos != 0)
			ls = -0.01;
		else
			return true;
	}
	if (ls > 1e-5 || ls < -1e-5)
	{
		m_cl->iMove(ls, 0.0);
		Sleep(500);
	}
	return false;
}


void NavigationImp::CalcOffset(int center, MagneticData &temp, int &toUp, int &toDown)
{
	if (temp.fpos > center)
		toUp = temp.fpos - center;
	else
		toUp = center - temp.fpos;

	if (temp.bpos > center)
		toDown = temp.bpos - center;
	else
		toDown = center - temp.bpos;
}


MAGNETICNAVIGATION_API CMagneticNavigation* CreateNavigation()
{
	return new NavigationImp();
}

MAGNETICNAVIGATION_API void DestoryNavigation(CMagneticNavigation** p)
{
	delete *p;
	*p = 0;
}
