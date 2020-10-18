#include "robot_face_msghandler.h"
#include "robot_face_struct.h"
#include "robot_json_rsp.h"
#include "robot_PostMessage.h"
#include "robot_face_manager.h"

static 	robot_face_msghandler* prfmsghandler = NULL;
robot_face_msghandler::robot_face_msghandler()
{
	m_interval_timer = 500;
	m_client_face_state = false;
}


robot_face_msghandler* robot_face_msghandler::getPtr()
{
	if (prfmsghandler == NULL)
		prfmsghandler = new robot_face_msghandler;
	return prfmsghandler;
}

void robot_face_msghandler::init()
{
	if (!common_config_get_field_int("Face", "intervalinfo", &m_interval_timer))
		m_interval_timer = 500;
}


robot_face_msghandler::~robot_face_msghandler()
{
	if (state())quit();
}

void robot_face_msghandler::run()
{
	std::vector<face_process_content> face_process_con;
	robot_face_manager::getPtr()->get_allprocesser(face_process_con);

	m_faces_info_cache.clear();
	for (int i = 0; i < face_process_con.size();i++)
		m_faces_info_cache[face_process_con[i].cameraid].direction = face_process_con[i].direction;


	bool loop = true;
	Timer_helper faceinfo_timer;

	robot_json_rsp json_helper;

	bool    invaild_info = false;

	while (loop)
	{
		Message msg = getq();
		switch (msg.msgType)
		{
		case FACES_POSITION_INFO:
		{
			FacesPositionInfo *ftemp = (FacesPositionInfo *)msg.msgObject;
			if (ftemp == NULL)break;
			
			invaild_info = false;
			bool need_notify = false;

			bool last_state = m_faces_info_cache[ftemp->cameraid].realtime_position.size() == 0 ? false : true;

			m_faces_info_cache[ftemp->cameraid].realtime_position = ftemp->faces;

			bool now_state = m_faces_info_cache[ftemp->cameraid].realtime_position.size() == 0 ? false : true;

			if (m_faces_info_cache[ftemp->cameraid].direction == 0)
			{
				for (int i = 0; i < m_faces_info_cache[ftemp->cameraid].realtime_position.size(); i++)
				{
					//if (m_faces_info_cache[ftemp->cameraid].realtime_position[i].age != 0)
					if (m_faces_info_cache[ftemp->cameraid].realtime_position[i].valid)
						invaild_info = true;
				}

				if (invaild_info)
				{
					if (m_faces_info_cache[ftemp->cameraid].vaild_times.isreach(m_interval_timer))
						need_notify = true;
					m_faces_info_cache[ftemp->cameraid].vaild_times.update();
				}
				else
				{
					if (last_state != now_state&&now_state)
						need_notify = true;

				}


			}
			if (m_faces_info_cache[ftemp->cameraid].direction == 1)
			{
				if (last_state != now_state&&now_state)
					need_notify = true;
			}

			if (need_notify || now_state&&faceinfo_timer.isreach(m_interval_timer))
			{
				std::string out = json_helper.face_info_notify(0, m_faces_info_cache);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length());
				faceinfo_timer.update();
			}
			SAFE_DELETE(ftemp);



		}
		break;
		case FACE_DETECT_NTF:
		{
			FaceDetectData *ftemp = (FaceDetectData *)msg.msgObject;
			if (ftemp == NULL)break;
			

			m_faces_info_cache[ftemp->cameraid].face_status = ftemp->status;

			SAFE_DELETE(ftemp);
			m_client_face_state = false;
			auto it = m_faces_info_cache.begin();
			for (; it != m_faces_info_cache.end(); it++)
			{
				if (it->second.direction == 0 && it->second.face_status)
				{
					m_client_face_state = true;
				}
			}
			LOGI("[face detect] person %s", m_client_face_state?"enter":"leave");
			std::string out = json_helper.face_detect_ntf(m_faces_info_cache,m_client_face_state);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length());

		}
			break;
		case FACE_DETECT_STRONG_NTF:
		{
			std::string out = json_helper.face_detect_ntf(m_faces_info_cache, m_client_face_state);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.length(), msg.message.c_str());
		}
		break;
		case _EXIT_THREAD_:
			loop = false;
			break;
		default:;
		}
	}
}



