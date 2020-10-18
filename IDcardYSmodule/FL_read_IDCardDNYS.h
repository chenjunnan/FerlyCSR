#ifndef __FL_READ_IDCARDDNYS_H__
#define __FL_READ_IDCARDDNYS_H__

#include <windows.h>
#include <string>
#define OP_OK				 0
#define OP_ERR				-1

typedef int(__stdcall *FL_InitComm)(int port);
typedef int(__stdcall *FL_InitCommExt)();
typedef int(__stdcall *FL_CloseComm)();
typedef int(__stdcall *FL_Authenticate)();
typedef int(__stdcall *FL_Read_Content)(int active);
typedef int(__stdcall *FL_Read_Content_Path)(char* cPath, int active);
typedef int(__stdcall *FL_GetDeviceID)(char * pMsg);
typedef char*(__stdcall *FL_GetSAMID)();
typedef int(__stdcall *FL_GetPhoto)(char * Wlt_File);
typedef int(__stdcall *FL_MfrInfo)(char * cDeviceType, char * cDeviceCategory, char * cDeviceName, char * cMfr);


//二代证信息
typedef struct
{
	wchar_t szName[31];   //姓名
	char szSex[3];     //性别
	char szNation[20]; //民族
	wchar_t szBornDay[10]; //出生
	wchar_t szAddress[128]; //地址
	wchar_t szIDNum[20];   //身份证编号
	wchar_t szIssued[31];  //签发机关
	wchar_t szBeginValidity[10]; //开始有效日期
	wchar_t szEndValidity[10];   //截止有效日期
	unsigned char IDFinger[1024];  //指纹数据
	int FingerLen;             //指纹数据长度
	int Age;           //年龄

}CRTDef_IDInfo_YS, *pCRTDef_IDInf_YS;

//外国人永久居留证信息
typedef struct
{
	char szEnName[128];  // 英文名
	char szSex[3];       // 性别
	char szIDNum[31];    // 永久居留证号码
	char szAddress[20];  // 国籍或所在地区代码
	char szCnName[31];   // 中文名
	char szBeginValidity[16];   // 证件签发日期
	char szEndValidity[16];     // 证件终止日期
	char szBornDay[16];         // 出生日期
	char szVersion[5];          // 证件版本号
	char szIssued[8];           // 申请受理机关代码
	char szType[2];             // 证件类型标识
	int Age;                //年龄
}CRTDef_PRTDInfo_YS, *pCRTDef_PRTDInfo_YS;

//港澳台居住证信息
typedef struct
{
	char szName[31];   //姓名
	char szSex[3];     //性别
	char szBornDay[10]; //出生
	char szAddress[128]; //地址
	char szIDNum[20];   //身份证编号
	char szIssued[31];  //签发机关
	char szBeginValidity[10]; //开始有效日期
	char szEndValidity[10];   //截止有效日期
	char szPassNo[18];		//通行证号码
	char szIssueNum[4];		//签发次数
	char szIdType[2];		//证件类型标识
	unsigned char IDFinger[1024];  //指纹数据
	int FingerLen;             //指纹数据长度
	int Age;                //年龄
}CRTDef_GATResidencePermit, *pCRTDef_GATResidencePermit;


class Read_IDCardDNYS
{
public:
	Read_IDCardDNYS(){}
	~Read_IDCardDNYS(){}

private:
	FL_InitComm InitComm;
	FL_InitCommExt InitCommExt;
	FL_CloseComm CloseComm;
	FL_Authenticate Authenticate;
	FL_Read_Content Read_Content;
	FL_Read_Content_Path Read_Content_Path;
	FL_GetDeviceID GetDeviceID;
	FL_GetSAMID GetSAMID;
	FL_GetPhoto GetPhoto;
	FL_MfrInfo MfrInfo;

public:
	void Init_Dll();
	int Send_OpenDev(std::string port);
	void Send_CloseDev();
	int Send_FindCard();
	int Send_ChooseCard();
	int Send_ReadIDData();
	std::string get_error();
	CRTDef_IDInfo_YS IDInfo;
	CRTDef_PRTDInfo_YS PRTDInfo;
	CRTDef_GATResidencePermit GATInfo;

private:
	bool index_open = false;
	char error_buffer[4096];
	void PopErrMsg(char ret);

};

#endif