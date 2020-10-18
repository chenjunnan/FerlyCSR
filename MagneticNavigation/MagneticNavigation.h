#pragma once
// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 MAGNETICNAVIGATION_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// MAGNETICNAVIGATION_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef MAGNETICNAVIGATION_EXPORTS
#define MAGNETICNAVIGATION_API __declspec(dllexport)
#else
#define MAGNETICNAVIGATION_API __declspec(dllimport)
#endif

struct MagneticData
{
	unsigned char fpos = 0;
	unsigned char bpos = 0;
	int   angle = 0;
	int	   quadrant = 0;
	unsigned long long times = 0;
	double desx;
	double desy;
	double desangle;
	MagneticData()
	{
		fpos = 0;
		bpos = 0;
		angle = 0;
		quadrant = 0;
		times = 0;
		desx = 0;
		desy = 0;
		desangle = 0;
	}
};

struct PositionStc
{
	double wx;
	double wy;
	double ww;
	double wz;
	int x;
	int y;
	double angle;
};

struct ProtectorStc
{
	bool state[6];
};

class Controllerinterface
{
public :
	virtual int iMove(double ls,double as)=0;
	virtual int iRotate(double angle, double as) = 0;
	virtual int iRotateWait() = 0;
	virtual int iRotateCancle() = 0;
	virtual int iPosition(PositionStc *Ps) = 0;
	virtual int iProtector(ProtectorStc *Ps) = 0;
};

class CMagneticNavigation {
public:
	virtual void Init(int Level) = 0;
	virtual void setController(Controllerinterface *handler)=0;
	virtual int  Handle(MagneticData *data)=0;
};


extern "C" MAGNETICNAVIGATION_API CMagneticNavigation* CreateNavigation();
extern "C" MAGNETICNAVIGATION_API void DestoryNavigation(CMagneticNavigation** p);
