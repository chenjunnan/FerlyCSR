#ifndef CWFaceNationalityAgeGroup_H
#define CWFaceNationalityAgeGroup_H

#include "CWFaceConfig.h"


#if defined (__cplusplus)
extern "C" {
#endif



	/**
	 * 功能：创建国籍年龄段识别句柄
	 * 输入：
	 *		pConfigurePath - 模型配置文件
	 *      pLicence       - 授权码（仅用于安卓平台，PC端传NULL即可）
	 *		bGPU           - 是否使用GPU
	 * 输出：
	 *      errCode        - 成功返回CW_OK，失败返回其他
	 * 返回值：
	 *		void*          - 国籍年龄段句柄: 成功返回句柄，失败返回0
	 */
CW_FACE_API 
void* cwCreateNationalityAgeGroupHandle(cw_errcode_t* errCode, const char* pConfigurePath, const char* pLicence);


    /**
	 * 功能：释放国籍年龄段识别句柄
	 * 输入：
	 *		pHandle - 国籍年龄段识别句柄
	 * 输出：
	 *      无
	 * 返回值：
	 *		无
	 */
CW_FACE_API 
void cwReleaseNationalityAgeGroupHandle(void* pHandle);


    /**
	 * 功能：获得国籍年龄段的估计值
	 * 输入：
	 *		pRecogHandle      - 国籍年龄段识别句柄
	 *      pAlignedFace      - 对齐人脸数据
	 * 输出：
	 *      pAgeGroup         - 年龄段估计值， 0为小孩，1为成年人，2为老人
	 *      pNationality      - 国籍估计值，0为中国人，1为外国人
	 * 返回值：
	 *		错误码
	 */
CW_FACE_API
cw_errcode_t cwGetNationalityAgeGroupEval(void* pHandle, cw_aligned_face_t* pAlignedFace, int* pAgeGroup, int* pNationality);



#if defined (__cplusplus)
}
#endif


#endif

