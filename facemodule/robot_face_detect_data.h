#pragma once
#include <string>
#include <vector>
class robot_face_detect_data
{
public:
	robot_face_detect_data();
	bool getDetectData(std::string needtype, std::vector<char> &out);
	void setDetectData(std::string inname,void *data,int len);
	virtual ~robot_face_detect_data();
private:
	std::vector<char> m_data;
	std::string    m_name;
};

