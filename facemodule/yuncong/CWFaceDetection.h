#ifndef CWFaceDetection_H
#define CWFaceDetection_H

#include "CWFaceConfig.h"



#if defined (__cplusplus)
extern "C" {
#endif




    /**
	 * ���ܣ�������������
	 * ���룺
	 *      pConfigFile		    - ģ�Ͳ��������ļ�
	 *      pLicense            - ��Ȩ�루�����ڰ�׿ƽ̨��PC�˴�NULL���ɣ�
	 * �����
	 *      errCode             - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 * ����ֵ��
	 *		��������          - �ɹ����ؾ����ʧ�ܷ���0
	 */
CW_FACE_API
void* cwCreateDetHandle(cw_errcode_t* errCode, const char* pConfigFile, const char*	pLicense);


	/**
	 * ���ܣ��ͷż����
	 * ���룺
	 *		pDetector - ��������
	 * �����
	 *      ��
	 * ����ֵ��
	 *		��
	 */
CW_FACE_API 
void cwReleaseDetHandle(void* pDetector);


	/**
	 * ���ܣ���ȡ���������
	 * ���룺
	 *		pDetector    - ��������
	 * �����
	 *      param        - ���������
	 * ����ֵ��
	 *		cw_errcode_t - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 */
CW_FACE_API
cw_errcode_t cwGetFaceParam(void* pDetector, cw_det_param_t* param);


	/**
	 * ���ܣ����ü���������������ȵ���cwGetFaceParam��ʹ�ô˺�����
	 * ���룺
	 *      pDetector - ��������
	 *		param     - ����
	 * �����
	 *      ��
	 * ����ֵ��
	 *		cw_errcode_t - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������	
	 */
CW_FACE_API
cw_errcode_t cwSetFaceParam(void* pDetector, const cw_det_param_t* param);


	/**
	 * ���ܣ������������˳��Ĭ��0-������
	 * ���룺
	 *      pDetector - ��������
	 *		flag      - 0-������1-��������С�Ӵ�С����2-��id��С��������3-����������4-���ϵ�������
	 * �����
	 *      ��
	 * ����ֵ��
	 *		cw_errcode_t - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������	
	 */
CW_FACE_API
cw_errcode_t cwSetFaceBufOrder(void* pDetector, int flag);


	/**
	 * ���ܣ����������ٽӿ�
	 * ���룺
	 *      pDetector    - ��������
	 *		pFrameImg    - �����ͼ��
	 *		iBuffLen     - ��ż����pFaceBuffer�����Ԫ�ظ���
	 *		iOp          - ������
	 * �����
	 *		pFaceBuffer  - ��ż����������
	 *		nFaceNum     - ʵ�ʱ���⵽��������
	 * ����ֵ��
	 *		cw_errcode_t - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������	
	 */
CW_FACE_API 
cw_errcode_t cwFaceDetection(void*          pDetector,			
						     cw_img_t*      pFrameImg,			
							 cw_face_res_t* pFaceBuffer,			
							 int            iBuffLen,					
							 int*           nFaceNum,					
							 int            iOp);


	/**
	 * ���ܣ����������״̬��Ϣ����
	 * ���룺
	 *		pDetector    - ��������
	 * �����
	 *      ��
	 * ����ֵ��
	 *		cw_errcode_t - �ɹ�����CW_SDKLIT_OK��ʧ�ܷ�������
	 */
CW_FACE_API
cw_errcode_t cwResetDetTrackState(void* pDetector);



#if defined (__cplusplus)
}
#endif


#endif


