// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 SLAMWAREROBOTCONTROLMODULE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// SLAMWAREROBOTCONTROLMODULE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef SLAMWAREROBOTCONTROLMODULE_EXPORTS
#define SLAMWAREROBOTCONTROLMODULE_API __declspec(dllexport)
#else
#define SLAMWAREROBOTCONTROLMODULE_API __declspec(dllimport)
#endif

//// 此类是从 SlamWareRobotControlModule.dll 导出的
//class SLAMWAREROBOTCONTROLMODULE_API CSlamWareRobotControlModule {
//public:
//	CSlamWareRobotControlModule(void);
//	// TODO: 在此添加您的方法。
//};
//
//extern SLAMWAREROBOTCONTROLMODULE_API int nSlamWareRobotControlModule;
//
//SLAMWAREROBOTCONTROLMODULE_API int fnSlamWareRobotControlModule(void);

typedef void (*LOGSINK)(const char*);


enum enROBOTSTATE
{
	STATUS_ActionStatusWaitingForStart,

	// The action is running
	STATUS_ActionStatusRunning,

	// The action is finished successfully
	STATUS_ActionStatusFinished,

	// The action is paused
	STATUS_ActionStatusPaused,

	// The action is stopped
	STATUS_ActionStatusStopped,

	// The action is under error
	STATUS_ActionStatusError,

	STATUS_Disconnect,

	STATUS_ActiveEmpty
};


struct tSLPOINT
{
	double x;
	double y;
};

struct tLASERPOINT
{
	int value;
	double angle;
	double distance;
};

struct tSlamRobotChargeState
{
	int onDock;//0 unknow 1 在桩上  2 buzai
	int PowerStageState;//0 weizhi  1 start 2 run 3 restart 4 close 5 error
	bool isCharging;
	bool isDCConnect;
	bool emergencystop;
	bool sdpconnected;
};

extern "C"
{
	SLAMWAREROBOTCONTROLMODULE_API int InitSlamWare(const char*ip,LOGSINK fun);

	SLAMWAREROBOTCONTROLMODULE_API void Bye();

	SLAMWAREROBOTCONTROLMODULE_API int GetCurrentPosition(double *x ,double *y,double *yaw);

	SLAMWAREROBOTCONTROLMODULE_API int NavigationTo(double x ,double y,double yaw,int mode);

	SLAMWAREROBOTCONTROLMODULE_API enROBOTSTATE WaitNavigation();

	SLAMWAREROBOTCONTROLMODULE_API const char *GetWaitErrorResult();

	SLAMWAREROBOTCONTROLMODULE_API int CancelCurrentAction();

	SLAMWAREROBOTCONTROLMODULE_API int RotateAngleTo(int angle,double speed);

	SLAMWAREROBOTCONTROLMODULE_API int RotateAngle(int angle,double speed);

	SLAMWAREROBOTCONTROLMODULE_API int Move(int ori,double speed);

	SLAMWAREROBOTCONTROLMODULE_API int ScanMap();

	SLAMWAREROBOTCONTROLMODULE_API int CancelScanMap();

	SLAMWAREROBOTCONTROLMODULE_API int SaveMap(const char* path);

	SLAMWAREROBOTCONTROLMODULE_API int GetMapBmp(const char* bmppath);

	SLAMWAREROBOTCONTROLMODULE_API int ConvertMap2Bmp(const char* mappath,const char* bmppath);

	SLAMWAREROBOTCONTROLMODULE_API int SetMap(double x ,double y,double yaw,const char* path);

	SLAMWAREROBOTCONTROLMODULE_API int ReLocalization(int count,tSLPOINT *list=0 );

	SLAMWAREROBOTCONTROLMODULE_API enROBOTSTATE GetRobotStatus();

	SLAMWAREROBOTCONTROLMODULE_API int GoHome();

	SLAMWAREROBOTCONTROLMODULE_API bool SetHome(double x ,double y,double yaw);

	SLAMWAREROBOTCONTROLMODULE_API int GetBattery();

	SLAMWAREROBOTCONTROLMODULE_API int GetLaserData(tLASERPOINT **data);//<0 error 

	SLAMWAREROBOTCONTROLMODULE_API bool SetSpeed(int speed);//0 heigh  1 low 2 mid

	SLAMWAREROBOTCONTROLMODULE_API int SetAPNetwork(const char *ssid,const char* password,const char *ip,const char* channel);

	SLAMWAREROBOTCONTROLMODULE_API int SetSTANetwork(const char *ssid,const char* password);

	SLAMWAREROBOTCONTROLMODULE_API const char * GetHealth();

	SLAMWAREROBOTCONTROLMODULE_API int GetChargeStatus(tSlamRobotChargeState *data);

	SLAMWAREROBOTCONTROLMODULE_API int ClearAllError();

	SLAMWAREROBOTCONTROLMODULE_API int GetRemainPath(tSLPOINT **path);

	SLAMWAREROBOTCONTROLMODULE_API int ResetRobot(bool softmode);

	SLAMWAREROBOTCONTROLMODULE_API int ShutdownRobot(int shutdowntime,int starttime);
};