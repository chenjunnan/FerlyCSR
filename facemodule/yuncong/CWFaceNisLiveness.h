#ifndef CW_NISLIVENESS_DET_SDK_H
#define CW_NISLIVENESS_DET_SDK_H


#include "CWFaceConfig.h"



#if defined (__cplusplus)
extern "C" {
#endif



	/**
	 * 创建红外活体检测器
	 * 输入：
	 *      sNirModelPath		        - 红外活体检测器模型文件
	 *		pMatrixParaDir				- 匹配文件路径
	 *		pLogDir						- 日志保存目录，不能填空
	 *      fSkinThresh					- 肤色阈值（根据不同的前端版本设置）
	 *      camType                     - 红外活体检测摄像头数目类型，单目或者双目，目前双目效果更好
	 *		detModel					- 红外活体检测模式：距离最近人脸检测/多人检测
	 *		pLicence					- 授权码（仅用于安卓平台，PC端传NULL即可）
	 * 输出：
	 *      errCode                     - 红外活体错误码
	 * 返回值：
	 *		红外活体句柄                - 成功返回句柄，失败返回0
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
	 * 释放红外活体句柄
	 * 输入：
	 *      pHandle    - 红外活体句柄
	 * 输出：
	 *		无
	 * 返回值：
	 *		无
	 */
CW_FACE_API
void cwReleaseNirLivenessHandle(void *pHandle);


	/**
	 * 红外活体检测接口
	 * 输入：
	 *      pHandle                  - 红外活体句柄
	 *		pNirLivDetInfo           - 输入的红外和可见光图片及关键点等信息
	 * 输出：
	 *		pNirLivRes               - 存放红外活体检测结果的数组，需事先分配内存
	 * 返回值：
	 *		cw_nirliveness_err_t     - 错误码
	 */
CW_FACE_API
cw_nirliveness_err_t cwFaceNirLivenessDet(void *pHandle, cw_nirliv_detinfo_t *pNirLivDetInfo, cw_nirliv_res_t *pNirLivRes);



#if defined (__cplusplus)
}
#endif

#endif   // CW_NISLIVENESS_DET