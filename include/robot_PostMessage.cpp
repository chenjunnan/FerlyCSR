#include "robot_PostMessage.h"


robot_PostMessage::robot_PostMessage()
{
	m_post_message = NULL;
}


robot_PostMessage * robot_PostMessage::getPostManager()
{
	static robot_PostMessage __ptr;
	return &__ptr;
}

void robot_PostMessage::set(fPostDataCB fun)
{
	m_post_message = fun;
}

void robot_PostMessage::post(const char *strModuleName, const char *strResult, unsigned long dResultLen, const char *desModuleName /*= NULL*/, const unsigned char *strExtdata /*= NULL*/, unsigned long dExtlen/*=0*/)
{
	if (m_post_message)
	{
		m_post_message(strModuleName, strResult, dResultLen, desModuleName, strExtdata, dExtlen);
	}
}

robot_PostMessage::~robot_PostMessage()
{
}

