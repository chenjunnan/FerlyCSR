#ifndef CWFaceVersion_H
#define CWFaceVersion_H


#include "CWFaceConfig.h"


#if defined (__cplusplus)
extern "C" {
#endif


	/**
	 * 功能：获取SDK版本信息
	 * 输入：
	 *		iBuffLen     - 输出buf分配字节长度
	 * 输出：
	 *      pVersion     - 版本信息，需事先分配内存
	 * 返回值：
	 *		cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API 
cw_errcode_t cwGetFaceSDKVersion(char* pVersion, int iBuffLen);


    /**
	 * 功能：通过网络授权安装licence，生成的V2C授权文件会保存在当前目录
	 * 输入：
	 *		sAuthorizedSerial  - 授权序列号，需从云从科技获取
	 *		iInstallFlag       - 安装授权标志，建议使用CW_RETERROR_IFEXIST，如果存在正式授权则不继续安装
	 *		iSaveFlag          - 安装授权时保存文件标志，建议使用CW_SAVE_ALL，将设备文件和授权文件都保存到当前路径
	 * 输出：
	 *      无
	 * 返回值：
	 *		cw_errcode_t       - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API 
cw_errcode_t cwInstallLicence(const char* sAuthorizedSerial, cw_install_key_flag_t iInstallFlag, cw_save_key_flag_t iSaveFlag);


    /**
	 * 功能：获取授权并发数
	 * 输入：
	 *      pLicence      - 授权码（仅用于安卓，PC传NULL即可）
	 * 输出：
	 *      无
	 * 返回值：
	 *		int   -  授权并发数
	 */
CW_FACE_API 
int cwGetMaxHandlesNum(const char* pLicence);


	/**
	 * 功能：获取设备唯一码，仅移动端有效
	 * 输入：
	 *		iBuffLen     - 输出buf分配字节长度，不低于160字节
	 * 输出：
	 *      pDeviceInfo  - 设备唯一码，需事先分配内存，不低于160字节
	 *      iUseLen      - 输出的设备码的长度
	 * 返回值：
	 *		cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API 
cw_errcode_t cwGetDeviceInfo(char* pDeviceInfo, int iBuffLen, int *iUseLen);


    /**
	 * 功能：获取移动端授权码licence，PC端无效
	 * 输入：
	 *		pCusName     - 客户名称，需从云从科技获取
	 *      pCusCode     - 客户账号，需从云从科技获取    
	 *      iBuffLen     - 分配字节长度，不低于300字节
	 * 输出：
	 *      pLicence     - 输出licence码，需事先分配内存，不低于300字节
	 *      iUseLen      - 输出的licence码的长度
	 * 返回值：
	 *		cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API 
cw_errcode_t cwGetLicence(const char* pCusName, const char* pCusCode, char* pLicence, int iBuffLen, int *iUseLen);


    /**
	 * 功能：获取移动端授权码licence，PC端无效，供集成商用
	 * 输入：
	 *		pCusName     - 客户名称，需从云从科技获取
	 *      pCusCode     - 客户账号，需从云从科技获取    
	 *      pDeviceInfo  - 设备码信息，由cwGetDeviceInfo接口获取
	 *      iBuffLen     - 分配字节长度，不低于300字节
	 * 输出：
	 *      pLicence     - 输出licence码，需事先分配内存，不低于300字节
	 *      iUseLen      - 输出的licence码的长度
	 * 返回值：
	 *		cw_errcode_t - 成功返回CW_SDKLIT_OK，失败返回其他
	 */
CW_FACE_API 
cw_errcode_t cwGetLicenceForCustom(const char* pCusName, const char* pCusCode, const char* pDeviceInfo, char* pLicence, int iBuffLen, int *iUseLen);


#if defined (__cplusplus)
}
#endif


#endif



