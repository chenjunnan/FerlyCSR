#ifndef CWFaceServiceSDK_H
#define CWFaceServiceSDK_H

#include "CWFaceConfig.h"



#if defined (__cplusplus)
extern "C" {
#endif


   /**
    * 功能：SDK服务初始化
    * 输入：
    *      sDetConfigFile      - 检测模型配置文件路径
    *	   sRecogConfigPath    - 识别模型配置文件路径
	*      sLicense            - 移动端授权码，PC端填NULL
	*      iMinFace            - 最小人脸尺寸
	*      iMaxFace            - 最大人脸尺寸
	*      iHandleNum          - 创建的句柄个数，建议填1
    * 输出：
    *      无
    * 返回值：
    *	   cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他	
	*/
CW_FACE_API
cw_errcode_t cwInitSDKService(const char* sDetConfigFilePath, const char* sRecogConfigFilePath, const char *sLicense, int iMinFace, int iMaxFace, int iHandleNum);


   /**
    * 功能：关闭SDK服务，释放资源
    * 输入：
    *      无
    * 输出：
    *      无
    * 返回值：
    *	   cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他	
	*/
CW_FACE_API 
cw_errcode_t cwCloseSDKService();


   /**
    * 功能：通过图片数据进行人脸检测，获取人脸位置及质量分，最多可检测20个人脸
    * 输入：
    *      imgData      - 图片数据，可支持多种格式
    *	   iDataLen     - 图片数据长度，仅二进制CW_IMAGE_BINARY格式必填，其他格式可不填
	*	   iWidth       - 图片数据宽度，除了CW_IMAGE_BINARY，其他格式都需必填
	*	   iHeight      - 图片数据高度，除了CW_IMAGE_BINARY，其他格式都需必填
	*	   format       - 图片格式，详见cw_img_form_t枚举类型	
	*      iBufLen      - outResult分配的长度	
    * 输出：
    *      outResult    - 输出的人脸字符串，需分配足够字节长度，格式为：(left,top,right,bottom,quality)，用^分隔开，如x,x,x,x,x^x,x,x,x,x^...
	*      iOutLen      - 输出的outResult字符串实际长度
    * 返回值：
    *	   cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他	
	*/
CW_FACE_API 
cw_errcode_t cwFaceServiceDet(char* imgData, int iDataLen, int iWidth, int iHeight, cw_img_form_t format, char *outResult, int iBufLen, int *iOutLen);


   /**
    * 功能：获取特征长度，需先调用cwInitSDKService
    * 输入：
    *      无
    * 输出：
    *      无
    * 返回值：
    *	   int        - 成功返回特征长度，失败返回0	
	*/
CW_FACE_API 
int cwFaceServiceGetFeaLength();


   /**
    * 功能：提取图片中最大人脸特征
    * 输入：
    *      imgData      - 图片数据，可支持多种格式
    *	   iDataLen     - 图片数据长度，仅二进制CW_IMAGE_BINARY格式必填，其他格式可不填
	*	   iWidth       - 图片数据宽度，除了CW_IMAGE_BINARY，其他格式都需必填
	*	   iHeight      - 图片数据高度，除了CW_IMAGE_BINARY，其他格式都需必填
	*	   format       - 图片格式，详见cw_img_form_t枚举类型	
    * 输出：
    *      outFeature   - 输出的人脸特征，该参数需事先分配内存，分配内存大小为：cwGetServiceFeaLen
    * 返回值：
    *	   cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他	
	*/
CW_FACE_API 
cw_errcode_t cwFaceServiceGetFeature(char* imgData, int iDataLen, int iWidth, int iHeight, cw_img_form_t format, char *outFeature);


   /**
    * 功能：通过图片数据进行人脸比对，获取两个图片中最大人脸相似度分数
    * 输入：
    *      imgData1      - 图片1数据，可支持多种格式
    *	   iDataLen1     - 图片1数据长度，仅二进制CW_IMAGE_BINARY格式必填，其他格式可不填
	*	   iWidth1       - 图片1数据宽度，除了CW_IMAGE_BINARY，其他格式都需必填
	*	   iHeight1      - 图片1数据高度，除了CW_IMAGE_BINARY，其他格式都需必填
	*	   format1       - 图片1格式，详见cw_img_form_t枚举类型	
	*      imgData2      - 图片2数据，可支持多种格式
	*	   iDataLen2     - 图片2数据长度，仅二进制CW_IMAGE_BINARY格式必填，其他格式可不填
	*	   iWidth2       - 图片2数据宽度，除了CW_IMAGE_BINARY，其他格式都需必填
	*	   iHeight2      - 图片2数据高度，除了CW_IMAGE_BINARY，其他格式都需必填
	*	   format2       - 图片2格式，详见cw_img_form_t枚举类型		
    * 输出：
    *      fScore        - 输出的人脸相似度分数
    * 返回值：
    *	   cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他	
	*/
CW_FACE_API 
cw_errcode_t cwFaceVerify(char* imgData1, int iDataLen1, int iWidth1, int iHeight1, cw_img_form_t format1, 
                          char* imgData2, int iDataLen2, int iWidth2, int iHeight2, cw_img_form_t format2, float *fScore);


	/**
	 * 功能：注册图片中最大人脸到识别库，如果该id已经存在，返回错误码
	 * 输入：
	 *      id             - 人脸id
	 *      imgData        - 图片数据，可支持多种格式
	 *	    iDataLen       - 图片数据长度，仅二进制CW_IMAGE_BINARY格式必填，其他格式可不填
	 *	    iWidth         - 图片数据宽度，除了CW_IMAGE_BINARY，其他格式都需必填
	 *	    iHeight        - 图片数据高度，除了CW_IMAGE_BINARY，其他格式都需必填
	 *	    format         - 图片格式，详见cw_img_form_t枚举类型	
	 * 输出： 
	 *      无
	 * 返回值：
	 *		cw_errcode_t   - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API
cw_errcode_t cwRegistFace(char *id, char* imgData, int iDataLen, int iWidth, int iHeight, cw_img_form_t format);


	/**
	 * 功能：注册人脸特征到识别库，如果该id已经存在，返回错误码
	 * 输入：
	 *      id             - 人脸id
	 *      feature        - 人脸特征
	 * 输出： 
	 *      无
	 * 返回值：
	 *		cw_errcode_t   - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API
cw_errcode_t cwRegistFeature(char *id, char* feature);


	/**
	 * 功能：从识别库删除人脸
	 * 输入：
	 *      id             - 人脸id
	 * 输出：
	 *      无
	 * 返回值：
	 *		cw_errcode_t   - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API
cw_errcode_t cwUnRegistFace(char *id);


	/**
	 * 功能：通过图片中最大人脸在识别库中检索TopN
	 * 输入：
	 *      imgData        - 图片数据，可支持多种格式
	 *	    iDataLen       - 图片数据长度，仅二进制CW_IMAGE_BINARY格式必填，其他格式可不填
	 *	    iWidth         - 图片数据宽度，除了CW_IMAGE_BINARY，其他格式都需必填
	 *	    iHeight        - 图片数据高度，除了CW_IMAGE_BINARY，其他格式都需必填
	 *	    format         - 图片格式，详见cw_img_form_t枚举类型
	 *      iTopN          - 检索topN
	 *      iBufLen        - outResult分配的字节长度
	 * 输出：
	 *      outResult      - 输出的最相似的topN的人脸id及分数，格式为：userId1=score1^userId2=score2^userId3=score3^...
	 *      iOutLen        - 输出的outResult字符串实际长度
	 * 返回值：
	 *		cw_errcode_t   - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API
cw_errcode_t cwSearchFace(char* imgData, int iDataLen, int iWidth, int iHeight, cw_img_form_t format, int iTopN, char *outResult, int iBufLen, int *iOutLen);


#if defined (__cplusplus)
}
#endif


#endif


