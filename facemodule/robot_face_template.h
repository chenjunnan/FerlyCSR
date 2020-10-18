#pragma once
#include <map>
#include "robot_face_struct.h"
#include <list>


class robot_face_template
{
public:
	static robot_face_template *getInstance();

	void add_feature_handler(Thread_helper *);

	void remove_feature_handler(Thread_helper *);

	void UpdateTemplate(std::map<std::string, FaceDatabaseFormat>   &facelist);

	void setTemplate(std::string guid, FaceDatabaseFormat &data);

	void delTemplatebyName(std::string name);

	void clearTemplate();

	~robot_face_template();
private:
	robot_face_template();

	CHANDLE    m_l;

	std::map<std::string, FaceDatabaseFormat> m_facelist;
	void	readfacedb();
	void	writefacedb();

	std::list<Thread_helper*> m_cb_list;
};

