#ifndef CWFaceRecognition_H
#define CWFaceRecognition_H


#include "CWFaceConfig.h"



#if defined (__cplusplus)
extern "C" {
#endif



	/**
	 * ���ܣ�����ʶ����
	 * ���룺
	 *		pConfigurePath - ģ�������ļ�
	 *      pLicence       - ��Ȩ�루�����ڰ�׿ƽ̨��PC�˴�NULL���ɣ�
	 *		emRecogPattern - �����ľ������
	 * �����
	 *      errCode        - �����룺�ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 * ����ֵ��
	 *		void*          - ʶ�������ɹ����ؾ����ʧ�ܷ���0
	 */
CW_FACE_API 
void* cwCreateRecogHandle(cw_errcode_t* errCode, 
                          const char* pConfigurePath,
						  const char* pLicence, 
						  cw_recog_pattern_t emRecogPattern);


    /**
	 * ���ܣ��ͷ�ʶ����
	 * ���룺
	 *		pRecogHandle - ʶ����
	 * �����
	 *      ��
	 * ����ֵ��
	 *		��
	 */
CW_FACE_API 
void cwReleaseRecogHandle(void* pRecogHandle);


    /**
	 * ���ܣ���ȡ��������
	 * ���룺
	 *		pRecogHandle - ʶ����
	 * �����
	 *      ��
	 * ����ֵ��
	 *		int          - ��������
	 */
CW_FACE_API 
int cwGetFeatureLength(void* pRecogHandle);


    /**
	 * ���ܣ���ȡ����ע�������������һ��ֻ��ȡһ������
	 * ���룺
	 *		pRecogHandle  - ʶ����
	 *      alignedFace   - �������������ָ��
	 * �����
	 *      pFeatueData    - ���ص��������ݣ���ҪԤ�ȷ����㹻�ռ�
	 * ����ֵ��
	 *		cw_errcode_t   - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 */
CW_FACE_API 
cw_errcode_t cwGetFiledFeature(void*              pRecogHandle, 
                               cw_aligned_face_t* alignedFace, 
							   void*              pFeatueData);


    /**
	 * ���ܣ���ȡ������������������һ��ֻ��ȡһ������
	 * ���룺
	 *		pRecogHandle  - ʶ����
	 *      alignedFace   - �������������ָ��
	 * �����
	 *      pFeatueData    - ���ص��������ݣ���ҪԤ�ȷ����㹻�ռ�
	 * ����ֵ��
	 *		cw_errcode_t   - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 */
CW_FACE_API
cw_errcode_t cwGetProbeFeature(void*              pRecogHandle, 
                               cw_aligned_face_t* alignedFace, 
							   void*              pFeatueData);



    /**
	 * ���ܣ�������ע�������ת��Ϊ���ڼ���������
	 * ���룺
	 *		pRecogHandle  - ʶ����
	 *      pFeaFiled     - ����ע�������
	 *      iFeaFiledDim  - ����ע�����������
	 *      iFeaFiledNum  - ����ע�����������
	 * �����
	 *      pFeaProbe     - �������ڼ�����������������ҪԤ�ȷ���ռ�
	 * ����ֵ��
	 *		cw_errcode_t  - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 */
CW_FACE_API 
cw_errcode_t cwConvertFiledFeatureToProbeFeature(void*       pRecogHandle, 
                                                 const void* pFeaFiled, 
												 int         iFeaFiledDim, 
												 int         iFeaFiledNum, 
												 void*       pFeaProbe);


    /**
	 * ���ܣ�����n��������m�����������ƶȣ����ص����ƶ�pScores�ĸ���Ϊm * n
	 * ���룺
	 *		pRecogHandle  - ʶ����
	 *      ppFeaProbe    - ���ڼ���������
	 *      iFeaProbeDim  - ���ڼ�������������
	 *      iFeaProbeNum  - ���ڼ�������������
	 *      pFeaFiled     - ����ע�������
	 *      iFeaFiledDim  - ����ע�����������
	 *      iFeaFiledNum  - ����ע�����������
	 * �����
	 *      pScores       - ���ص����ƶȷ������飬����ΪiFeaProbeNum * iFeaFiledNum����ҪԤ�ȷ���ռ�
	 * ����ֵ��
	 *		cw_errcode_t  - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 */
CW_FACE_API 
cw_errcode_t cwComputeMatchScore(void*       pRecogHandle, 
                                 const void* pFeaProbe, 
								 int         iFeaProbeDim, 
								 int         iFeaProbeNum,
								 const void* pFeaFiled, 
								 int         iFeaFiledDim, 
								 int         iFeaFiledNum,
								 float*      pScores);


    /**
	 * ���ܣ������ںϣ���ഫ��ʮ������
	 * ���룺
	 *		pRecogHandle     - ʶ����
	 *      pFeatureAll      - ��Ҫ�ںϵ������������ʮ����������װ��һ��
	 *      fWeightAr        - ��������Ȩ��
	 *      iCount           - ʵ�ʴ�����������
	 * �����
	 *      pFeatureOut      - �ںϺ������
	 * ����ֵ��
	 *		cw_errcode_t     - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 */
CW_FACE_API 
cw_errcode_t cwMergeFeature(void *pRecogHandle, char* pFeatureAll, float fWeightAr[10], int iCount, char *pFeatureOut);



#if defined (__cplusplus)
}
#endif



#endif

