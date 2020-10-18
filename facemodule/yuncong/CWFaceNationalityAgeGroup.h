#ifndef CWFaceNationalityAgeGroup_H
#define CWFaceNationalityAgeGroup_H

#include "CWFaceConfig.h"


#if defined (__cplusplus)
extern "C" {
#endif



	/**
	 * ���ܣ��������������ʶ����
	 * ���룺
	 *		pConfigurePath - ģ�������ļ�
	 *      pLicence       - ��Ȩ�루�����ڰ�׿ƽ̨��PC�˴�NULL���ɣ�
	 *		bGPU           - �Ƿ�ʹ��GPU
	 * �����
	 *      errCode        - �ɹ�����CW_OK��ʧ�ܷ�������
	 * ����ֵ��
	 *		void*          - ��������ξ��: �ɹ����ؾ����ʧ�ܷ���0
	 */
CW_FACE_API 
void* cwCreateNationalityAgeGroupHandle(cw_errcode_t* errCode, const char* pConfigurePath, const char* pLicence);


    /**
	 * ���ܣ��ͷŹ��������ʶ����
	 * ���룺
	 *		pHandle - ���������ʶ����
	 * �����
	 *      ��
	 * ����ֵ��
	 *		��
	 */
CW_FACE_API 
void cwReleaseNationalityAgeGroupHandle(void* pHandle);


    /**
	 * ���ܣ���ù�������εĹ���ֵ
	 * ���룺
	 *		pRecogHandle      - ���������ʶ����
	 *      pAlignedFace      - ������������
	 * �����
	 *      pAgeGroup         - ����ι���ֵ�� 0ΪС����1Ϊ�����ˣ�2Ϊ����
	 *      pNationality      - ��������ֵ��0Ϊ�й��ˣ�1Ϊ�����
	 * ����ֵ��
	 *		������
	 */
CW_FACE_API
cw_errcode_t cwGetNationalityAgeGroupEval(void* pHandle, cw_aligned_face_t* pAlignedFace, int* pAgeGroup, int* pNationality);



#if defined (__cplusplus)
}
#endif


#endif

