#pragma once
// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� MAGNETICNAVIGATION_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// MAGNETICNAVIGATION_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
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
