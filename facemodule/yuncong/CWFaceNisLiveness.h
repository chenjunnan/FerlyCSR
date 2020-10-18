#ifndef CW_NISLIVENESS_DET_SDK_H
#define CW_NISLIVENESS_DET_SDK_H


#include "CWFaceConfig.h"



#if defined (__cplusplus)
extern "C" {
#endif



	/**
	 * ���������������
	 * ���룺
	 *      sNirModelPath		        - �����������ģ���ļ�
	 *		pMatrixParaDir				- ƥ���ļ�·��
	 *		pLogDir						- ��־����Ŀ¼���������
	 *      fSkinThresh					- ��ɫ��ֵ�����ݲ�ͬ��ǰ�˰汾���ã�
	 *      camType                     - �������������ͷ��Ŀ���ͣ���Ŀ����˫Ŀ��Ŀǰ˫ĿЧ������
	 *		detModel					- ���������ģʽ����������������/���˼��
	 *		pLicence					- ��Ȩ�루�����ڰ�׿ƽ̨��PC�˴�NULL���ɣ�
	 * �����
	 *      errCode                     - ������������
	 * ����ֵ��
	 *		���������                - �ɹ����ؾ����ʧ�ܷ���0
	 */
CW_FACE_API
void* cwCreateNirLivenessHandle(cw_nirliveness_err_t	*errCode,
                                const char				*pNirModelPath,
								const char				*pPairFilePath,
								const char				*pLogDir,
								const float				fSkinThresh,
								cw_nirliv_camera_type_t camType, 
								cw_nirliv_det_type_t    detModel,
								const char				*pLicence);		


	/**
	 * �ͷź��������
	 * ���룺
	 *      pHandle    - ���������
	 * �����
	 *		��
	 * ����ֵ��
	 *		��
	 */
CW_FACE_API
void cwReleaseNirLivenessHandle(void *pHandle);


	/**
	 * ���������ӿ�
	 * ���룺
	 *      pHandle                  - ���������
	 *		pNirLivDetInfo           - ����ĺ���Ϳɼ���ͼƬ���ؼ������Ϣ
	 * �����
	 *		pNirLivRes               - ��ź���������������飬�����ȷ����ڴ�
	 * ����ֵ��
	 *		cw_nirliveness_err_t     - ������
	 */
CW_FACE_API
cw_nirliveness_err_t cwFaceNirLivenessDet(void *pHandle, cw_nirliv_detinfo_t *pNirLivDetInfo, cw_nirliv_res_t *pNirLivRes);



#if defined (__cplusplus)
}
#endif

#endif   // CW_NISLIVENESS_DET