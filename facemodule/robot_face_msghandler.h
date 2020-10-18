#pragma once
#include "common_helper.h"
#include <map>
#include "robot_face_struct.h"
class robot_face_manager;

class robot_face_msghandler :public Thread_helper
{
public:
	static robot_face_msghandler* getPtr();

	void init();

	virtual ~robot_face_msghandler();

	virtual void run();

private:
	robot_face_msghandler();

	int    m_interval_timer;

	std::map<int, CameraCacheInfo> m_faces_info_cache;

	bool m_client_face_state;


};

