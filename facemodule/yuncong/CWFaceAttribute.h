#ifndef CWFaceAttribute_H
#define CWFaceAttribute_H

#include "CWFaceConfig.h"


#if defined (__cplusplus)
extern "C" {
#endif



	/**
	 * 功能：创建年龄性别属性句柄
	 * 输入：
	 *		pConfigurePath - 模型配置文件
	 *      pLicence       - 授权码（仅用于安卓平台，PC端传NULL即可）
	 * 输出：
	 *      errCode        - 成功返回CW_OK，失败返回其他
	 * 返回值：
	 *		void*          - 属性句柄: 成功返回句柄，失败返回0
	 */
CW_FACE_API 
void* cwCreateAttributeHandle(cw_errcode_t* errCode, const char* pConfigurePath, const char* pLicence);


    /**
	 * 功能：释放年龄性别句柄
	 * 输入：
	 *		pAttributeHandle - 属性句柄
	 * 输出：
	 *      无
	 * 返回值：
	 *		无
	 */
CW_FACE_API 
void cwReleaseAttributeHandle(void* pAttributeHandle);


    /**
	 * 功能：获得年龄性别估计
	 * 输入：
	 *		pAttributeHandle - 属性句柄
	 *      pAlignedFace     - 对齐人脸数据
	 * 输出：
	 *      pAge             - 年龄估计值
	 *      pGender          - 性别估计值，1为男士，2为女士
	 * 返回值：
	 *		错误码
	 */
CW_FACE_API
cw_errcode_t cwGetAgeGenderEval(void* pAttributeHandle, cw_aligned_face_t* pAlignedFace, int* pAge, int* pGender);



#if defined (__cplusplus)
}
#endif


#endif

