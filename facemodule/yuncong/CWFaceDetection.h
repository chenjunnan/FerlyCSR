#ifndef CWFaceDetection_H
#define CWFaceDetection_H

#include "CWFaceConfig.h"



#if defined (__cplusplus)
extern "C" {
#endif




    /**
	 * 功能：创建检测器句柄
	 * 输入：
	 *      pConfigFile		    - 模型参数配置文件
	 *      pLicense            - 授权码（仅用于安卓平台，PC端传NULL即可）
	 * 输出：
	 *      errCode             - 成功返回CW_SDKLIT_OK，失败返回其他
	 * 返回值：
	 *		检测器句柄          - 成功返回句柄，失败返回0
	 */
CW_FACE_API
void* cwCreateDetHandle(cw_errcode_t* errCode, const char* pConfigFile, const char*	pLicense);


	/**
	 * 功能：释放检测器
	 * 输入：
	 *		pDetector - 检测器句柄
	 * 输出：
	 *      无
	 * 返回值：
	 *		无
	 */
CW_FACE_API 
void cwReleaseDetHandle(void* pDetector);


	/**
	 * 功能：获取检测器参数
	 * 输入：
	 *		pDetector    - 检测器句柄
	 * 输出：
	 *      param        - 检测器参数
	 * 返回值：
	 *		cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API
cw_errcode_t cwGetFaceParam(void* pDetector, cw_det_param_t* param);


	/**
	 * 功能：设置检测器参数（必须先调用cwGetFaceParam再使用此函数）
	 * 输入：
	 *      pDetector - 检测器句柄
	 *		param     - 参数
	 * 输出：
	 *      无
	 * 返回值：
	 *		cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他	
	 */
CW_FACE_API
cw_errcode_t cwSetFaceParam(void* pDetector, const cw_det_param_t* param);


	/**
	 * 功能：设置人脸输出顺序，默认0-不排序
	 * 输入：
	 *      pDetector - 检测器句柄
	 *		flag      - 0-不排序，1-按人脸大小从大到小排序，2-按id从小到大排序；3-从左到右排序；4-从上到下排序
	 * 输出：
	 *      无
	 * 返回值：
	 *		cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他	
	 */
CW_FACE_API
cw_errcode_t cwSetFaceBufOrder(void* pDetector, int flag);


	/**
	 * 功能：人脸检测跟踪接口
	 * 输入：
	 *      pDetector    - 检测器句柄
	 *		pFrameImg    - 被检测图像
	 *		iBuffLen     - 存放检测结果pFaceBuffer数组的元素个数
	 *		iOp          - 操作码
	 * 输出：
	 *		pFaceBuffer  - 存放检测结果的数组
	 *		nFaceNum     - 实际被检测到的人脸数
	 * 返回值：
	 *		cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他	
	 */
CW_FACE_API 
cw_errcode_t cwFaceDetection(void*          pDetector,			
						     cw_img_t*      pFrameImg,			
							 cw_face_res_t* pFaceBuffer,			
							 int            iBuffLen,					
							 int*           nFaceNum,					
							 int            iOp);


	/**
	 * 功能：清除检测跟踪状态信息函数
	 * 输入：
	 *		pDetector    - 检测器句柄
	 * 输出：
	 *      无
	 * 返回值：
	 *		cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API
cw_errcode_t cwResetDetTrackState(void* pDetector);



#if defined (__cplusplus)
}
#endif


#endif


