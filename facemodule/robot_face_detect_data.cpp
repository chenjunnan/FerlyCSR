#include "robot_face_detect_data.h"


robot_face_detect_data::robot_face_detect_data()
{
}


bool robot_face_detect_data::getDetectData(std::string needtype, std::vector<char> &out)
{
	if (needtype.compare(m_name) == 0)
	{
		out.clear();
		out = m_data;
		return true;
	}
	else
	{
		//TODO convert from one to other alg 
		return false;
	}
}

void robot_face_detect_data::setDetectData(std::string inname, void *data, int len)
{
	m_data.clear();
	m_data.resize(len);
	memcpy(m_data.data(),data,len);
	m_name = inname;
}

robot_face_detect_data::~robot_face_detect_data()
{
}
