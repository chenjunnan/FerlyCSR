#include "robot_face_readface_singleton.h"
#include <iostream>
#include <fstream> 

#ifdef HAS_READFACE
#include "RSFaceSDK.h"

robot_face_readface_singleton::robot_face_readface_singleton()
{
	iniok = false;
	m_Plicense = NULL;
}


robot_face_readface_singleton * robot_face_readface_singleton::getInstance()
{
	static robot_face_readface_singleton m_gaf;
	return &m_gaf;
}
bool robot_face_readface_singleton::GetFileContent(std::string filePath, std::string& initInfo)
{
	std::ifstream filestream(filePath.c_str());
	if (!filestream.is_open())
	{
		return false;
	}
	std::stringstream streambuf;
	streambuf << filestream.rdbuf();
	initInfo = streambuf.str();
	return true;
}

bool robot_face_readface_singleton::isLicense()
{
	if (iniok)return iniok;
	std::string strLicenseContent("");
	std::string filepath;
	common_get_current_dir(&filepath);
	common_fix_path(&filepath);

	std::string path = filepath + READFACE_CFGPATH;
	common_fix_path(&path);
	if (!common_isDirectory(path.c_str()))
		common_create_dir(path.c_str());

	std::string lis = filepath;
	lis.append(READFACE_CFG_INIT_LICENSE_PATH);

	if (!GetFileContent(lis, strLicenseContent))
	{
		LOGE("missing initlicense file");
		return false;
	}
	int rsInitLicenseflag = rsInitLicenseManagerV2(&m_Plicense, strLicenseContent.c_str());
	if (rsInitLicenseflag)
	{
		LOGE("readface init failed  ,error license file %d", rsInitLicenseflag);
		return false;
	}

	std::string strAuthContent("");
	std::string authfile = filepath;
	authfile.append(READFACE_CFG_INIT_AUTH_CONTENT_PATH);

	if (!GetFileContent(authfile, strAuthContent))
	{
		LOGE("missing initAuthContent file");
		return false;
	}
	int activateFlag = rsActivateLicenseManagerV2(m_Plicense, strAuthContent.c_str());

	if (activateFlag != 0)
	{
		char buffer[4096] = { 0 };
		int requestFlag = rsRunLicenseManagerV2AuthRequest(m_Plicense, (char**)&buffer);
		printf("rsRunLicenseManagerV2AuthRequest requestFlag = %d \n", requestFlag);
		LOGE("rsActivateLicenseManagerV2 failed = %d ", activateFlag);
		return false;
	}
	const char *sdkdevice = rsGetSDKLicenseDeviceKey();
	LOGI("ReadFace DeviceKey %s .", sdkdevice);
	const char *sdkversion = rsGetRSFaceSDKVersion();
	LOGI("ReadFace Version: %s.", sdkversion);
	iniok = true;
	return iniok;
}


RSHandle robot_face_readface_singleton::getRSHandle()
{
	return m_Plicense;
}

robot_face_readface_singleton::~robot_face_readface_singleton()
{
	if (m_Plicense != NULL)
	{
		rsUnInitLicenseManagerV2(&m_Plicense);
		m_Plicense = NULL;
	}
}
#endif // HAS_READFACE
