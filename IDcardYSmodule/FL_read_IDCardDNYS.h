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


//����֤��Ϣ
typedef struct
{
	wchar_t szName[31];   //����
	char szSex[3];     //�Ա�
	char szNation[20]; //����
	wchar_t szBornDay[10]; //����
	wchar_t szAddress[128]; //��ַ
	wchar_t szIDNum[20];   //���֤���
	wchar_t szIssued[31];  //ǩ������
	wchar_t szBeginValidity[10]; //��ʼ��Ч����
	wchar_t szEndValidity[10];   //��ֹ��Ч����
	unsigned char IDFinger[1024];  //ָ������
	int FingerLen;             //ָ�����ݳ���
	int Age;           //����

}CRTDef_IDInfo_YS, *pCRTDef_IDInf_YS;

//��������þ���֤��Ϣ
typedef struct
{
	char szEnName[128];  // Ӣ����
	char szSex[3];       // �Ա�
	char szIDNum[31];    // ���þ���֤����
	char szAddress[20];  // ���������ڵ�������
	char szCnName[31];   // ������
	char szBeginValidity[16];   // ֤��ǩ������
	char szEndValidity[16];     // ֤����ֹ����
	char szBornDay[16];         // ��������
	char szVersion[5];          // ֤���汾��
	char szIssued[8];           // ����������ش���
	char szType[2];             // ֤�����ͱ�ʶ
	int Age;                //����
}CRTDef_PRTDInfo_YS, *pCRTDef_PRTDInfo_YS;

//�۰�̨��ס֤��Ϣ
typedef struct
{
	char szName[31];   //����
	char szSex[3];     //�Ա�
	char szBornDay[10]; //����
	char szAddress[128]; //��ַ
	char szIDNum[20];   //���֤���
	char szIssued[31];  //ǩ������
	char szBeginValidity[10]; //��ʼ��Ч����
	char szEndValidity[10];   //��ֹ��Ч����
	char szPassNo[18];		//ͨ��֤����
	char szIssueNum[4];		//ǩ������
	char szIdType[2];		//֤�����ͱ�ʶ
	unsigned char IDFinger[1024];  //ָ������
	int FingerLen;             //ָ�����ݳ���
	int Age;                //����
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