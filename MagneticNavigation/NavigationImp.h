#pragma once
#include "MagneticNavigation.h"
#include <stdio.h>
#include "xCommon.h"

#ifdef WIN32
#define LOGMASE_FORMAT(name, level, file,function,line, logformat, ...) \
{do{ \
	common_log(name,level,file,function,line,logformat, ##__VA_ARGS__);\
} while (0);}
#else
#endif
const char * MODULE_NAME = "mase";

#define LOGMT( fmt, ...) LOGMASE_FORMAT(MODULE_NAME, LOG_LEVEL_TRACE, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGMD( fmt, ...) LOGMASE_FORMAT(MODULE_NAME, LOG_LEVEL_DEBUG, __FILE__,__FUNCTION__,  __LINE__, fmt, ##__VA_ARGS__)
#define LOGMI( fmt, ...) LOGMASE_FORMAT(MODULE_NAME, LOG_LEVEL_INFO, __FILE__,__FUNCTION__,  __LINE__, fmt, ##__VA_ARGS__)
#define LOGMW( fmt, ...) LOGMASE_FORMAT(MODULE_NAME,LOG_LEVEL_WARN, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGME( fmt, ...) LOGMASE_FORMAT(MODULE_NAME, LOG_LEVEL_ERROR, __FILE__,__FUNCTION__,  __LINE__, fmt, ##__VA_ARGS__)
#define LOGMF( fmt, ...) LOGMASE_FORMAT(MODULE_NAME, LOG_LEVEL_FATAL, __FILE__,__FUNCTION__,  __LINE__, fmt, ##__VA_ARGS__)


enum STATE_TASK
{
	
};

class NavigationImp :public CMagneticNavigation
{
public:
	NavigationImp();
	virtual void Init(int Level);
	virtual void setController(Controllerinterface *handler);
	virtual int  Handle(MagneticData *data);
	virtual ~NavigationImp();
private:
	int		NormalMove(double ids);
	int		CheckAngle();
	int		getSpeed(int center, double dis, MagneticData &temp, double &als, double &aas);
	double  getDistance(MagneticData &point);
	int		AdjustAngle();
	void	ChangeOri(double ids);
	bool	RunLine(bool reach);
	void	CalcOffset(int center, MagneticData &temp, int &toUp, int &toDown);
	double  CheckFindDistance();
	int		SelfFindMode();
	void	StandbyRotate(PositionStc &p);
	Controllerinterface * m_cl;
	int					step;
	int	 ok_count = 0;
	unsigned long long last_tims = 0;
	unsigned int fsum = 0, bsum = 0;
	double center = 0;
	int OriSpeed = 1;
	MagneticData m_sensordata;
	int  findmemsstep;
	double fromx;
	double fromy;
	double fromangle;
	unsigned long timesmove;
    int LEVEL = 0;
	double dis_poie;
	int maybe_end_try_count;
	int max_try_find_count;
	bool failed_runtoend;

	int rotate_step;
	double angle_rotate;
	double rotate_speed;
};

