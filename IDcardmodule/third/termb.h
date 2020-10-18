#ifndef _TERMB_H_
#define _TERMB_H_

#ifdef __cplusplus
extern "C"{
#endif 

int __stdcall InitComm(int port);
int __stdcall InitCommEx();

bool __stdcall IsVerifyKey();

int __stdcall CloseComm();
int __stdcall Authenticate();
int __stdcall Read_Content(int active);
int __stdcall Read_Content_Path(char* cPath,int active);
int __stdcall GetSAMID(char *c);
int __stdcall GetRFID(LPDWORD id);
int __stdcall GetCOMBaud(unsigned int *puiBaud);
int __stdcall SetCOMBaud(unsigned int uiCurrBaud, unsigned int  uiSetBaud);
int __stdcall SetBPBoxPort(int nComPort, int nTransPort);
int __stdcall SetAntPower(int nReaderSlot, bool bHalfPower, bool bPowerOn);
int __stdcall TransApduCommand(int nReaderSlot, unsigned char *pSendDatas, int nSendLen,unsigned char *pRecvDatas, int *nRecvLen);
int __stdcall ResetCard(int nReaderSlot, unsigned char *pRecvDatas, int *nRecvLen);
VOID __stdcall SetWZInfo(unsigned char *info);
BOOL __stdcall OpenWZFile(char *sFileName);
BOOL __stdcall DataToPic(LPCTSTR lpFileName, LPBYTE lpBuf);
BOOL __stdcall MakeIDCardImage(char* sHead, char* sIDCardImage);
BOOL __stdcall MakeIDCardImages(char* sHead, char* sIDCardImagePath);

char* __stdcall GetName(VOID);
char* __stdcall GetSex(VOID);
char* __stdcall GetNation(VOID);
char* __stdcall GetSexCode(VOID);
char* __stdcall GetNationCode(VOID);
char* __stdcall GetBornDate(VOID);
char* __stdcall GetAddress(VOID);
char* __stdcall GetIDNo(VOID);
char* __stdcall GetSignGov(VOID);
char* __stdcall GetStartDate(VOID);
char* __stdcall GetEndDate(VOID);
char* __stdcall GetNewAddress(VOID);

char* __stdcall GetFPR_ENName();	//英文姓名
char* __stdcall GetFPR_SEX();			//性别
char* __stdcall GetFPR_IDNo();		//永久居留证号码
char* __stdcall GetFPR_NationCode();//国籍或所在地区代码
char* __stdcall GetFPR_CHNName();//中文姓名
char* __stdcall GetFPR_ValidStartDate();//证件签发日期
char* __stdcall GetFPR_ValidEndDate();//证件终止日期
char* __stdcall GetFPR_Birthday();//出生日期
char* __stdcall GetFPR_IDVersion();//证件版本号
char* __stdcall GetFPR_IssuingAuthorityCode();//当次申请受理机关代码
char* __stdcall GetFPR_IDType();//证件类型标识,大写字母I
char* __stdcall GetFPR_Reserve();//预留
int __stdcall IsFPRIDCard();//是否外国人永久居留证
VOID __stdcall DeleteMBCSString(LPSTR str);
BOOL __stdcall GetPhoto2(LPBYTE sTmp, LPINT nLen);
BOOL __stdcall GetFingerPrint2(LPBYTE sTmp, LPINT nLen);
/**********************************************************
 ********************** 端口类API *************************
 **********************************************************/
int __stdcall SDT_SetMaxRFByte(int iPortID,unsigned char ucByte,int bIfOpen);
int __stdcall SDT_GetCOMBaud(int iComID,unsigned int *puiBaud);
int __stdcall SDT_SetCOMBaud(int iComID,unsigned int  uiCurrBaud,unsigned int  uiSetBaud);
int __stdcall SDT_ClosePort(int iPortID);
int __stdcall SDT_OpenPort(int iPortID);
int __stdcall SDT_GetErrorString(int ErrorCode, char * ErrorString);

/**********************************************************
 ********************** SAM类API **************************
 **********************************************************/
int __stdcall SDT_GetSAMStatus(int iPortID,int iIfOpen);
int __stdcall SDT_ResetSAM(int iPortID,int iIfOpen);
int __stdcall SDT_GetSAMID(int iPortID,unsigned char *pucSAMID,int iIfOpen);
int __stdcall SDT_GetSAMIDToStr(int iPortID,char *pcSAMID,int iIfOpen);

/**********************************************************
 ******************* 身份证卡类API ************************
 **********************************************************/
int __stdcall SDT_StartFindIDCard(int iPortID,unsigned char *pucIIN,int iIfOpen);
int __stdcall SDT_SelectIDCard(int iPortID,unsigned char *pucSN,int iIfOpen);
int __stdcall SDT_ReadBaseMsg(int iPortID,unsigned char * pucCHMsg,unsigned int *	puiCHMsgLen,unsigned char * pucPHMsg,unsigned int  *puiPHMsgLen,int iIfOpen);
int __stdcall SDT_ReadBaseMsgToFile(int iPortID,char * pcCHMsgFileName,unsigned int *	puiCHMsgFileLen,char * pcPHMsgFileName,unsigned int  *puiPHMsgFileLen,int iIfOpen);
int __stdcall SDT_ReadNewAppMsg(int iPortID,unsigned char * pucAppMsg,unsigned int *	puiAppMsgLen,int iIfOpen);
int	__stdcall SDT_ReadBaseFPMsg(int iPort, unsigned char *pucCHMsg, unsigned int *puiCHMsgLen, unsigned char *pucPHMsg, unsigned int *puiPHMsgLen, unsigned char *pucFPMsg, unsigned int *puiFPMsgLen, int iIfOpen);
int	__stdcall SDT_ReadBaseFPMsgToFile(int iPort, char *pcCHMsgFileName, unsigned int *puiCHMsgFileLen, char *pcPHMsgFileName, unsigned int *puiPHMsgFileLen, char *pcFPMsgFileName, unsigned int *puiFPMsgFileLen, int iIfOpen);

#ifdef __cplusplus
}
#endif 

#endif