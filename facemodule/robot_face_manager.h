#pragma once
#include <list>
#include "common_helper.h"
#include "opencv2\core\core.hpp"
#include "robot_face_struct.h"
#include "robot_face_process.h"


class robot_face_manager
{
public:
	static robot_face_manager *getPtr();

	bool   init();


	bool   start();

	void   stop();

	void   add_message_type(const char * message);

	void   get_allprocesser(std::vector<face_process_content>&fc);

	int    message_handler(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata = NULL,
		unsigned long dExtlen = 0);

	int handler_register_message(std::string &msgtype, FaceMessage &fmsg, Json::Value &json_object);

	virtual ~robot_face_manager();
private:
	void post_report(FaceMessage &fmsg);
	void get_statusfilepath(std::string &exepath);
	void check_status();
	void write_status(int id, int pos, int state);
	robot_face_manager();

	std::vector<robot_face_process*>  m_face_handler;
	std::vector<face_process_content>  m_face_processer_content;
	robot_json_rsp      m_json;
	std::vector<std::string >  m_message_list;
	int       m_mainfaceid;
 };

