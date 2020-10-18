#pragma once
#include "robot_face_struct.h"

#ifdef HAS_READFACE
#include "RSCommon.h"

class robot_face_readface_singleton
{
public:
	static robot_face_readface_singleton *getInstance();
	bool   isLicense();
	RSHandle getRSHandle();
	virtual ~robot_face_readface_singleton();
private:
	bool GetFileContent(std::string filePath, std::string& initInfo);

	robot_face_readface_singleton();
	// Ö¤Êé¾ä±ú
	RSHandle m_Plicense;
	bool		iniok;
};

#endif // HAS_READFACE
