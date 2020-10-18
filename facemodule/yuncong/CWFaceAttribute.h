#ifndef CWFaceAttribute_H
#define CWFaceAttribute_H

#include "CWFaceConfig.h"


#if defined (__cplusplus)
extern "C" {
#endif



	/**
	 * ���ܣ����������Ա����Ծ��
	 * ���룺
	 *		pConfigurePath - ģ�������ļ�
	 *      pLicence       - ��Ȩ�루�����ڰ�׿ƽ̨��PC�˴�NULL���ɣ�
	 * �����
	 *      errCode        - �ɹ�����CW_OK��ʧ�ܷ�������
	 * ����ֵ��
	 *		void*          - ���Ծ��: �ɹ����ؾ����ʧ�ܷ���0
	 */
CW_FACE_API 
void* cwCreateAttributeHandle(cw_errcode_t* errCode, const char* pConfigurePath, const char* pLicence);


    /**
	 * ���ܣ��ͷ������Ա���
	 * ���룺
	 *		pAttributeHandle - ���Ծ��
	 * �����
	 *      ��
	 * ����ֵ��
	 *		��
	 */
CW_FACE_API 
void cwReleaseAttributeHandle(void* pAttributeHandle);


    /**
	 * ���ܣ���������Ա����
	 * ���룺
	 *		pAttributeHandle - ���Ծ��
	 *      pAlignedFace     - ������������
	 * �����
	 *      pAge             - �������ֵ
	 *      pGender          - �Ա����ֵ��1Ϊ��ʿ��2ΪŮʿ
	 * ����ֵ��
	 *		������
	 */
CW_FACE_API
cw_errcode_t cwGetAgeGenderEval(void* pAttributeHandle, cw_aligned_face_t* pAlignedFace, int* pAge, int* pGender);



#if defined (__cplusplus)
}
#endif


#endif

