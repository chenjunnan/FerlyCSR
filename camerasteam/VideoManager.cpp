#include "VideoManager.h"
#include <assert.h>
#include "json\json.h"
#include "opencv2\core\core.hpp"
#include "robot_PostMessage.h"
#include <time.h>
extern const char *MODULE_NAME;
#define MAX_VIDEO 20

const char *FORMAT_PR[] = {
	"BGR", "RGB", "RBGA", "YUV420P", "YUY2"
};


VideoManager::VideoManager()
{
	m_ml = common_mutex_create_threadlock();
}

VideoManager * VideoManager::get()
{
	static VideoManager __v;
	return &__v;
}

bool VideoManager::Init()
{
	bool ret=m_dnotify.Init(GUID_CAPTURE, "视频设备监控窗口");
    update_device();
	return ret;
}

bool VideoManager::addvideo(VideoConfig vp)
{
	if (vp.video_logicID < 0 )
		return false;

	if (m_videomap.find(vp.video_logicID) != m_videomap.end() && m_videomap[vp.video_logicID])
		return true;

	m_videomap[vp.video_logicID] = new VideoStream(vp);
	if (m_videomap[vp.video_logicID]&&m_videomap[vp.video_logicID]->init_memory())
		return true;
	SAFE_DELETE(m_videomap[vp.video_logicID]);
	m_videomap.erase(vp.video_logicID);
	return false;
}

bool VideoManager::start()
{
	std::string request = "{\"msg_type\":\"DISPATCH_REGISTER_MESSAGE_MONITOR_REQUEST\",\"name\":\"client_service\"}";
	robot_PostMessage::getPostManager()->post(MODULE_NAME, request.c_str(), request.size(), "dispath_service");
	return true;
}

bool VideoManager::video_start(int index)
{
	if (index < 0 || m_videomap.find(index) == m_videomap.end() || m_videomap[index] == NULL)
		return false;
	if (m_videomap[index]->state())
		return true;
	if (m_videomap[index]->open())
		return true;
	return m_videomap[index]->state();
}

bool VideoManager::video_stop(int index)
{
	if (index < 0 || m_videomap.find(index) == m_videomap.end() || m_videomap[index] == NULL)
		return false;
	if (!m_videomap[index]->state())
		return true;
	m_videomap[index]->quit();
	return !m_videomap[index]->state();
}


bool VideoManager::video_set(int index, int t, int v, bool a)
{
	if (index < 0 || m_videomap.find(index) == m_videomap.end() || m_videomap[index] == NULL)
		return false;
	if (!m_videomap[index]->state())
		return false;
	return m_videomap[index]->set(t, v, a);
}

bool VideoManager::video_get(int index, int t, int &v, bool &a)
{
	if (index < 0 || m_videomap.find(index) == m_videomap.end() || m_videomap[index] == NULL)
		return false;
	if (!m_videomap[index]->state())
		return false;
	return m_videomap[index]->get(t, v, a);
}

void VideoManager::stop_all()
{
	if (m_dnotify.state())
		m_dnotify.quit();
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	auto it = m_videomap.begin();
	for (; it != m_videomap.end();it++)
		video_stop(it->first);
}

bool VideoManager::update_device()
{
	if (CameraDirect::CameraList(m_devicelist))
	{
		LOCK_HELPER_AUTO_HANDLER(m_ml);
		for (int i = 0; i < m_devicelist.size(); i++)
		{
			LOGI("[%d] %s .", m_devicelist[i].index, m_devicelist[i].name.c_str());
		}

		int video_count = 0;
		if (!common_config_get_field_int("Video", "videocount", &video_count))
		{
			LOGF("cant read video stream config");
			return false;
		}
		LOGI("video stream config count:%d ", video_count);
		while (video_count)
		{
			video_count--;
			VideoConfig ptemp_vp;
			char name_buffer[1024] = { 0 };
			sprintf_s(name_buffer, 1024, "VideoStreamConfig_%d", video_count);
			ptemp_vp.video_logicID = video_count;
			int enable = 0;
			if (!common_config_get_field_int(name_buffer, "enable", &enable))
			{
				LOGE("cant read video %d enable", video_count);
				continue;
			}
			if (!enable)continue;

			if (!common_config_get_field_string(name_buffer, "streamname", &ptemp_vp.stream_name))
			{
				LOGE("cant read video %d stream name", video_count);
				continue;
			}
			if (!common_config_get_field_string(name_buffer, "devicename", &ptemp_vp.device_name))
			{
				LOGE("cant read video %d device name", video_count);
				continue;
			}
			common_utf8_2gbk(&ptemp_vp.device_name, &ptemp_vp.device_name);

			if (!common_config_get_field_int(name_buffer, "deviceindex", &ptemp_vp.device_name_index))
			{
				LOGE("cant read video %d device index", video_count);
				continue;
			}
			if (!common_config_get_field_int(name_buffer, "channels", &ptemp_vp.channels))
			{
				LOGE("cant read video %d channels", video_count);
				continue;
			}
			if (!common_config_get_field_int(name_buffer, "width", &ptemp_vp.width))
			{
				LOGE("cant read video %d width", video_count);
				continue;
			}
			if (!common_config_get_field_int(name_buffer, "height", &ptemp_vp.height))
			{
				LOGE("cant read video %d height", video_count);
				continue;
			}
			if (!common_config_get_field_int(name_buffer, "fps", &ptemp_vp.fps))
			{
				LOGE("cant read video %d fps", video_count);
				continue;
			}
			if (!common_config_get_field_int(name_buffer, "angle", &ptemp_vp.angle))
			{
				LOGE("cant read video %d angle", video_count);
				continue;
			}
			if (!common_config_get_field_int(name_buffer, "mode", &ptemp_vp.mode))
			{
				LOGE("cant read video %d mode", video_count);
				continue;
			}
			std::string temp;
			if (!common_config_get_field_string(name_buffer, "format", &temp))
			{
				LOGE("cant read video %d format", video_count);
				continue;
			}
			std::transform(temp.begin(), temp.end(), temp.begin(), tolower);

			if (temp.compare("bgr") == 0)
				ptemp_vp.format = VF_BGR;
			if (temp.compare("rgb") == 0)
				ptemp_vp.format = VF_RGB;
			if (temp.compare("yuv420p") == 0)
				ptemp_vp.format = VF_YUV420P;

			int cnt = 0;
			bool found = false;
			for (int i = 0; i < m_devicelist.size(); i++)
			{
				if (m_devicelist[i].name.compare(ptemp_vp.device_name) == 0 && cnt++ == ptemp_vp.device_name_index)
				{
					found = true;
					cnt = i;
					break;
				}
			}
			if (found)
			{
				int opid = CameraDirect::GetOptimization(m_devicelist[cnt], ptemp_vp.width, ptemp_vp.height, ptemp_vp.fps);
				if (opid != -1)
				{
					ptemp_vp.width = m_devicelist[cnt].capablity[opid].min_width;
					ptemp_vp.height = m_devicelist[cnt].capablity[opid].min_height;
					ptemp_vp.fps = m_devicelist[cnt].capablity[opid].min_fps;
					ptemp_vp.cameracap = m_devicelist[cnt].capablity[opid];
				}
				else
					found = false;
			}
			
			bool online = false;
			if (found&&addvideo(ptemp_vp))
			{
				online = true;
				
				m_stream_name_id[ptemp_vp.stream_name] = ptemp_vp.video_logicID;;
			}
			else
			{
				LOGE("cant create video %d stream name :%s", video_count, ptemp_vp.stream_name.c_str());
			}

			if (m_videomap.find(ptemp_vp.video_logicID) != m_videomap.end())
			{
				m_videomap[ptemp_vp.video_logicID]->online = online;
			}

		}
		return true;
	}
	else
	{
		LOCK_HELPER_AUTO_HANDLER(m_ml);
		LOGE("cant find any live canmera");
		
		{
			for each (auto var in m_videomap)
				var.second->online = false;
			
		}
	}
	return false;
}

bool VideoManager::remove_device()
{
	update_device();
	//TODO 掉设备通知
// 	json_object["streamname"] = Json::Value(stream_name);
// 	json_object["error"] = Json::Value(error);
// 	std::string report = json_writer.write(json_object);
// 	robot_PostMessage::getPostManager()->post(MODULE_NAME, report.c_str(), report.size(), cm.from.c_str(), NULL, 0);
	return false;
}

int VideoManager::message_handler(CAMERA_MESSAGE &cm)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	Json::Value   json_object;
	Json::Reader  json_reader;
	Json::FastWriter  json_writer;
	bool ret_value = false;
	int error = 1;
	std::string stream_name,reader_name;
	bool b_json_read = json_reader.parse(cm.context, json_object);
	if (b_json_read)
	{
		if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
		{
			std::string msgtype = json_object["msg_type"].asString();
			common_trim(&msgtype);

			if (json_object.isMember("streamname") && json_object["streamname"].isString())
				stream_name = json_object["streamname"].asString();
			if (json_object.isMember("reader") && json_object["reader"].isString())
				reader_name = json_object["reader"].asString();
			if (reader_name.empty())reader_name = cm.from;

			if (msgtype.compare("CAMERA_START_STREAMNAME_REQUEST") == 0)
			{
				if (m_stream_name_id.find(stream_name) != m_stream_name_id.end())
				{
					if (m_listenlist.find(stream_name) != m_listenlist.end())
						error = 0;
					else
						error = video_start(m_stream_name_id[stream_name]) ? 0 : 1;
				}
				if (error == 0)
					m_listenlist[stream_name].insert(reader_name);
				json_object.clear();
				json_object["msg_type"] = Json::Value("CAMERA_START_STREAMNAME_RESPONSE");
			}
			if (msgtype.compare("CAMERA_STOP_STREAMNAME_REQUEST") == 0)
			{
				if (m_stream_name_id.find(stream_name) != m_stream_name_id.end())
				{
					if (m_listenlist.find(stream_name) != m_listenlist.end())
					{	
						if (m_listenlist[stream_name].find(reader_name) != m_listenlist[stream_name].end())
						{
							error = 0;
							m_listenlist[stream_name].erase(reader_name);
							if (m_listenlist[stream_name].size() == 0)
							{
								error = video_stop(m_stream_name_id[stream_name]) ? 0 : 1;
								m_listenlist.erase(stream_name);
							}
						}						
					}
				}
				json_object.clear();
				json_object["msg_type"] = Json::Value("CAMERA_STOP_STREAMNAME_RESPONSE");
			}
			if (msgtype.compare("CAMERA_CHANGE_USER_REQUEST") == 0)
			{
				if (m_stream_name_id.find(stream_name) != m_stream_name_id.end())
				{
					if (m_listenlist.find(stream_name) != m_listenlist.end())
						error = video_stop(m_stream_name_id[stream_name]) ? 0 : 1;
					else
						error = 0;
				}
				json_object.clear();
				json_object["msg_type"] = Json::Value("CAMERA_CHANGE_USER_RESPONSE");
			}
			if (msgtype.compare("CAMERA_RESUME_USER_REQUEST") == 0)
			{
				if (m_stream_name_id.find(stream_name) != m_stream_name_id.end())
				{
					if (m_listenlist.find(stream_name) != m_listenlist.end())
						error = video_start(m_stream_name_id[stream_name]) ? 0 : 1;
					else
						error = 0;
				}
				json_object.clear();
				json_object["msg_type"] = Json::Value("CAMERA_RESUME_USER_RESPONSE");
			}

			
			if (msgtype.compare("CAMERA_GETPARAMETER_STREAMNAME_REQUEST") == 0)
			{
				int value = 0;
				bool autype = false;
				int type = 0;
				if (json_object.isMember("parametertype") && json_object["parametertype"].isInt())
				{
					type = json_object["parametertype"].asInt();
					if (m_stream_name_id.find(stream_name) != m_stream_name_id.end())
						error = video_get(m_stream_name_id[stream_name],type, value, autype) ? 0 : 1;
				}			
				json_object.clear();
				if (!error)
				{
					json_object["parametertype"] = Json::Value(type);
					json_object["parametervalue"] = Json::Value(value);
					json_object["parameteram"] = Json::Value(autype);	
				}
				json_object["msg_type"] = Json::Value("CAMERA_GETPARAMETER_STREAMNAME_RESPONSE");
			}
			if (msgtype.compare("CAMERA_SETPARAMETER_STREAMNAME_REQUEST") == 0)
			{
				int value = 0;
				bool autype = true;
				int type = 0;
				
				if (json_object.isMember("parametervalue") && json_object["parametervalue"].isInt())
					value = json_object["parametervalue"].asInt();
				if (json_object.isMember("parameteram") && json_object["parameteram"].isBool())
					autype = json_object["parameteram"].asBool();
				if (json_object.isMember("parametertype") && json_object["parametertype"].isInt())
				{
					type = json_object["parametertype"].asInt();
					if (m_stream_name_id.find(stream_name) != m_stream_name_id.end())
						error = video_set(m_stream_name_id[stream_name], type, value, autype) ? 0 : 1;
				}
				json_object.clear();
				if (!error)
				{
					json_object["parametertype"] = Json::Value(type);	
				}
				json_object["msg_type"] = Json::Value("CAMERA_SETPARAMETER_STREAMNAME_RESPONSE");
			}
			if (msgtype.compare("CAMERA_LIST_STREAMNAME_REQUEST") == 0)
			{
				json_object.clear();
				error = 0;
				for each (auto var in m_stream_name_id)
				{
					if (m_videomap.find(var.second) != m_videomap.end())
					{
						
						Json::Value camera_info;
						camera_info["streamname"] = Json::Value(var.first);
						camera_info["state"] = Json::Value(m_videomap[var.second]->state());
						camera_info["online"] = Json::Value(m_videomap[var.second]->online);
						VideoConfig t_config;
						m_videomap[var.second]->getConfig(t_config);
						camera_info["devicename"] = Json::Value(t_config.device_name);
						camera_info["videoid"] = Json::Value(t_config.video_logicID);
						camera_info["width"] = Json::Value(t_config.width);
						camera_info["height"] = Json::Value(t_config.height);
						camera_info["channels"] = Json::Value(t_config.channels);
						camera_info["fps"] = Json::Value(t_config.fps);
						camera_info["format"] = Json::Value(FORMAT_PR[(int)t_config.format]);

						if (m_listenlist.find(var.first) != m_listenlist.end())
						{
							for (auto it = m_listenlist[var.first].begin(); it != m_listenlist[var.first].end();it++)
								camera_info["uselist"].append(*it);
						}
						json_object["cameralist"].append(camera_info);
					}
				}

				json_object["msg_type"] = Json::Value("CAMERA_LIST_STREAMNAME_RESPONSE");
			}
			if (cm.from.compare("dispath_service") == 0 && cm.appid == 16 && msgtype.compare("DISPATCH_CLIENT_DISCONNECT_NOTIFY") == 0)
			{
				if (json_object.isMember("guid") && json_object["guid"].isString())
				{
					std::string uid = json_object["guid"].asString();

					for (auto it = m_listenlist.begin(); it != m_listenlist.end(); )
					{
						if (it->second.find(uid) != it->second.end())
							it->second.erase(uid);

						if (it->second.size() == 0)
						{
							error = video_stop(m_stream_name_id[it->first]) ? 0 : 1;
							it=m_listenlist.erase(it);
						}
						else
							it++;
					}
				}
				json_object.clear();
			}
			if (cm.from.compare("dispath_service") == 0 && cm.appid == 16 && msgtype.compare("DISPATCH_CLIENT_CONNECT_NOTIFY") == 0)
			{
				json_object.clear();
			}
			
		}
	}
	
	if (!json_object.empty())
	{
		if (!reader_name.empty())json_object["reader"] = Json::Value(reader_name);
		if (!stream_name.empty())json_object["streamname"] = Json::Value(stream_name);
		json_object["error"] = Json::Value(error);
		std::string report = json_writer.write(json_object);
		robot_PostMessage::getPostManager()->post(MODULE_NAME, report.c_str(), report.size(), cm.from.c_str(), NULL, 0);
	}
	return 0;
}

VideoManager::~VideoManager()
{
	stop_all();
	common_mutex_threadlock_release(m_ml);
}
