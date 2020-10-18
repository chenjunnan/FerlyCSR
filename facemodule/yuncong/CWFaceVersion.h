#ifndef CWFaceVersion_H
#define CWFaceVersion_H


#include "CWFaceConfig.h"


#if defined (__cplusplus)
extern "C" {
#endif


	/**
	 * ���ܣ���ȡSDK�汾��Ϣ
	 * ���룺
	 *		iBuffLen     - ���buf�����ֽڳ���
	 * �����
	 *      pVersion     - �汾��Ϣ�������ȷ����ڴ�
	 * ����ֵ��
	 *		cw_errcode_t - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 */
CW_FACE_API 
cw_errcode_t cwGetFaceSDKVersion(char* pVersion, int iBuffLen);


    /**
	 * ���ܣ�ͨ��������Ȩ��װlicence�����ɵ�V2C��Ȩ�ļ��ᱣ���ڵ�ǰĿ¼
	 * ���룺
	 *		sAuthorizedSerial  - ��Ȩ���кţ�����ƴӿƼ���ȡ
	 *		iInstallFlag       - ��װ��Ȩ��־������ʹ��CW_RETERROR_IFEXIST�����������ʽ��Ȩ�򲻼�����װ
	 *		iSaveFlag          - ��װ��Ȩʱ�����ļ���־������ʹ��CW_SAVE_ALL�����豸�ļ�����Ȩ�ļ������浽��ǰ·��
	 * �����
	 *      ��
	 * ����ֵ��
	 *		cw_errcode_t       - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 */
CW_FACE_API 
cw_errcode_t cwInstallLicence(const char* sAuthorizedSerial, cw_install_key_flag_t iInstallFlag, cw_save_key_flag_t iSaveFlag);


    /**
	 * ���ܣ���ȡ��Ȩ������
	 * ���룺
	 *      pLicence      - ��Ȩ�루�����ڰ�׿��PC��NULL���ɣ�
	 * �����
	 *      ��
	 * ����ֵ��
	 *		int   -  ��Ȩ������
	 */
CW_FACE_API 
int cwGetMaxHandlesNum(const char* pLicence);


	/**
	 * ���ܣ���ȡ�豸Ψһ�룬���ƶ�����Ч
	 * ���룺
	 *		iBuffLen     - ���buf�����ֽڳ��ȣ�������160�ֽ�
	 * �����
	 *      pDeviceInfo  - �豸Ψһ�룬�����ȷ����ڴ棬������160�ֽ�
	 *      iUseLen      - ������豸��ĳ���
	 * ����ֵ��
	 *		cw_errcode_t - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 */
CW_FACE_API 
cw_errcode_t cwGetDeviceInfo(char* pDeviceInfo, int iBuffLen, int *iUseLen);


    /**
	 * ���ܣ���ȡ�ƶ�����Ȩ��licence��PC����Ч
	 * ���룺
	 *		pCusName     - �ͻ����ƣ�����ƴӿƼ���ȡ
	 *      pCusCode     - �ͻ��˺ţ�����ƴӿƼ���ȡ    
	 *      iBuffLen     - �����ֽڳ��ȣ�������300�ֽ�
	 * �����
	 *      pLicence     - ���licence�룬�����ȷ����ڴ棬������300�ֽ�
	 *      iUseLen      - �����licence��ĳ���
	 * ����ֵ��
	 *		cw_errcode_t - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 */
CW_FACE_API 
cw_errcode_t cwGetLicence(const char* pCusName, const char* pCusCode, char* pLicence, int iBuffLen, int *iUseLen);


    /**
	 * ���ܣ���ȡ�ƶ�����Ȩ��licence��PC����Ч������������
	 * ���룺
	 *		pCusName     - �ͻ����ƣ�����ƴӿƼ���ȡ
	 *      pCusCode     - �ͻ��˺ţ�����ƴӿƼ���ȡ    
	 *      pDeviceInfo  - �豸����Ϣ����cwGetDeviceInfo�ӿڻ�ȡ
	 *      iBuffLen     - �����ֽڳ��ȣ�������300�ֽ�
	 * �����
	 *      pLicence     - ���licence�룬�����ȷ����ڴ棬������300�ֽ�
	 *      iUseLen      - �����licence��ĳ���
	 * ����ֵ��
	 *		cw_errcode_t - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 */
CW_FACE_API 
cw_errcode_t cwGetLicenceForCustom(const char* pCusName, const char* pCusCode, const char* pDeviceInfo, char* pLicence, int iBuffLen, int *iUseLen);


#if defined (__cplusplus)
}
#endif


#endif



