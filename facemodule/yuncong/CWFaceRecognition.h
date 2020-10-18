#ifndef CWFaceRecognition_H
#define CWFaceRecognition_H


#include "CWFaceConfig.h"



#if defined (__cplusplus)
extern "C" {
#endif



	/**
	 * 功能：创建识别句柄
	 * 输入：
	 *		pConfigurePath - 模型配置文件
	 *      pLicence       - 授权码（仅用于安卓平台，PC端传NULL即可）
	 *		emRecogPattern - 创建的句柄类型
	 * 输出：
	 *      errCode        - 错误码：成功返回CW_SDKLIT_OK，失败返回其他
	 * 返回值：
	 *		void*          - 识别句柄：成功返回句柄，失败返回0
	 */
CW_FACE_API 
void* cwCreateRecogHandle(cw_errcode_t* errCode, 
                          const char* pConfigurePath,
						  const char* pLicence, 
						  cw_recog_pattern_t emRecogPattern);


    /**
	 * 功能：释放识别句柄
	 * 输入：
	 *		pRecogHandle - 识别句柄
	 * 输出：
	 *      无
	 * 返回值：
	 *		无
	 */
CW_FACE_API 
void cwReleaseRecogHandle(void* pRecogHandle);


    /**
	 * 功能：获取特征长度
	 * 输入：
	 *		pRecogHandle - 识别句柄
	 * 输出：
	 *      无
	 * 返回值：
	 *		int          - 特征长度
	 */
CW_FACE_API 
int cwGetFeatureLength(void* pRecogHandle);


    /**
	 * 功能：提取用于注册的人脸特征，一次只提取一个特征
	 * 输入：
	 *		pRecogHandle  - 识别句柄
	 *      alignedFace   - 对齐的人脸数据指针
	 * 输出：
	 *      pFeatueData    - 返回的特征数据，需要预先分配足够空间
	 * 返回值：
	 *		cw_errcode_t   - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API 
cw_errcode_t cwGetFiledFeature(void*              pRecogHandle, 
                               cw_aligned_face_t* alignedFace, 
							   void*              pFeatueData);


    /**
	 * 功能：提取待检索人脸的特征，一次只提取一个特征
	 * 输入：
	 *		pRecogHandle  - 识别句柄
	 *      alignedFace   - 对齐的人脸数据指针
	 * 输出：
	 *      pFeatueData    - 返回的特征数据，需要预先分配足够空间
	 * 返回值：
	 *		cw_errcode_t   - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API
cw_errcode_t cwGetProbeFeature(void*              pRecogHandle, 
                               cw_aligned_face_t* alignedFace, 
							   void*              pFeatueData);



    /**
	 * 功能：将用于注册的特征转化为用于检索的特征
	 * 输入：
	 *		pRecogHandle  - 识别句柄
	 *      pFeaFiled     - 用于注册的特征
	 *      iFeaFiledDim  - 用于注册的特征长度
	 *      iFeaFiledNum  - 用于注册的特征数量
	 * 输出：
	 *      pFeaProbe     - 返回用于检索的人脸特征，需要预先分配空间
	 * 返回值：
	 *		cw_errcode_t  - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API 
cw_errcode_t cwConvertFiledFeatureToProbeFeature(void*       pRecogHandle, 
                                                 const void* pFeaFiled, 
												 int         iFeaFiledDim, 
												 int         iFeaFiledNum, 
												 void*       pFeaProbe);


    /**
	 * 功能：计算n个特征与m个特征的相似度，返回的相似度pScores的个数为m * n
	 * 输入：
	 *		pRecogHandle  - 识别句柄
	 *      ppFeaProbe    - 用于检索的特征
	 *      iFeaProbeDim  - 用于检索的特征长度
	 *      iFeaProbeNum  - 用于检索的特征数量
	 *      pFeaFiled     - 用于注册的特征
	 *      iFeaFiledDim  - 用于注册的特征长度
	 *      iFeaFiledNum  - 用于注册的特征数量
	 * 输出：
	 *      pScores       - 返回的相似度分数数组，长度为iFeaProbeNum * iFeaFiledNum，需要预先分配空间
	 * 返回值：
	 *		cw_errcode_t  - 成功返回CW_SDKLIT_OK，失败返回其他
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
	 * 功能：特征融合，最多传入十个特征
	 * 输入：
	 *		pRecogHandle     - 识别句柄
	 *      pFeatureAll      - 需要融合的特征集，最多十个，连续组装在一起
	 *      fWeightAr        - 特征集的权重
	 *      iCount           - 实际传入特征个数
	 * 输出：
	 *      pFeatureOut      - 融合后的特征
	 * 返回值：
	 *		cw_errcode_t     - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API 
cw_errcode_t cwMergeFeature(void *pRecogHandle, char* pFeatureAll, float fWeightAr[10], int iCount, char *pFeatureOut);



#if defined (__cplusplus)
}
#endif



#endif

