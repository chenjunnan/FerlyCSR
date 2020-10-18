#ifndef CWFaceNisLivService_H
#define CWFaceNisLivService_H

#include "CWFaceConfig.h"



#if defined (__cplusplus)
extern "C" {
#endif


   /**
    * 功能：SDK红外活体服务初始化，该服务只支持双目摄像头，单人脸模式
    * 输入：
    *      sCWModelsPath       - CWModles文件夹路径
	*      sLicense            - 移动端授权码，PC端填NULL
	*      fSkinThresh         - 肤色阈值，一般0.5
	*      iMinFace            - 最小人脸尺寸
	*      iMaxFace            - 最大人脸尺寸
	*      iHandleNum          - 创建的句柄个数，建议填1
    * 输出：
    *      无
    * 返回值：
    *	   cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他	
	*/
CW_FACE_API
cw_errcode_t cwInitNirLivService(const char* sCWModelsPath, const char *sLicense, float fSkinThresh, int iMinFace, int iMaxFace, int iHandleNum);


   /**
    * 功能：关闭SDK红外活体服务，释放资源
    * 输入：
    *      无
    * 输出：
    *      无
    * 返回值：
    *	   cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他	
	*/
CW_FACE_API 
cw_errcode_t cwCloseNirLivService();


   /**
    * 功能：通过见光图片数据与红外图片数据获取活体状态及分数
    * 输入：
	*      imgDataVis      - 可见光图片数据，可支持多种格式
	*	   iDataLenVis     - 可见光图片数据长度，仅二进制CW_IMAGE_BINARY格式必填，其他格式可不填
	*	   iWidthVis       - 可见光图片数据宽度，除了CW_IMAGE_BINARY，其他格式都需必填
	*	   iHeightVis      - 可见光图片数据高度，除了CW_IMAGE_BINARY，其他格式都需必填
	*	   formatVis       - 可见光图片格式，详见cw_img_form_t枚举类型	
	*      imgDataNir      - 红外图片数据，可支持多种格式
	*	   iDataLenNir     - 红外图片数据长度，仅二进制CW_IMAGE_BINARY格式必填，其他格式可不填
	*	   iWidthNir       - 红外图片数据宽度，除了CW_IMAGE_BINARY，其他格式都需必填
	*	   iHeightNir      - 红外图片数据高度，除了CW_IMAGE_BINARY，其他格式都需必填
	*	   formatNir       - 红外图片格式，详见cw_img_form_t枚举类型	
    * 输出：
    *      state           - 输出的红外活体状态值，详情参考枚举cw_nirliv_det_rst_t
	*      fScore          - 输出的红外活体分数，一般小于0.5为非活体，否则为活体
    * 返回值：
    *	   cw_errcode_t    - 成功返回CW_SDKLIT_OK，失败返回其他	
	*/
CW_FACE_API 
cw_errcode_t cwNirLivDetService(char* imgDataVis, int iDataLenVis, int iWidthVis, int iHeightVis, cw_img_form_t formatVis, 
	                            char* imgDataNir, int iDataLenNir, int iWidthNir, int iHeightNir, cw_img_form_t formatNir, 
								cw_nirliv_det_rst_t *state, float *fScore);


#if defined (__cplusplus)
}
#endif


#endif


