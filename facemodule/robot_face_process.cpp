#include "robot_face_process.h"
#include "robot_face_detect_libfacedetect.h"
#include "robot_face_detect_seetaface.h"
#include "memorySwaper.h"
#include "camerastreamstruct.h"
#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "robot_PostMessage.h"
#include "robot_face_msghandler.h"
#include "robot_face_features.h"
#include "robot_FaceDebug_Windows.h"
#include "robot_face_features_seetaface.h"
#include "robot_face_feature_interface.h"

#ifdef HAS_YUNCONG
#include "robot_face_detect_yuncong.h"
#include "robot_face_live_handle_yuncong.h"
#include "robot_face_features_yuncong.h"
#pragma comment( lib, "CWFaceSDK.lib" )
#pragma comment( lib, "opencv_world310.lib" )
#endif // HAS_YUNCONG
#ifdef HAS_ARCSOFT

#include "robot_face_detect_arcsoft.h"
#include "robot_face_features_arcsoft.h"
#pragma comment(lib, "libarcsoft_face_engine.lib") 


#endif // HAS_ARCSOFT
#ifdef HAS_READFACE
#include "robot_face_detect_readface.h"
#include "robot_face_features_readface.h"
#pragma comment(lib, "libReadFace2.lib") 

#endif // HAS_READFACE





robot_face_process::robot_face_process()
{
	m_minface_ntf_th = 5;
	m_show_face_debug = false;
	m_wait_face_image = false;
	m_waitface_endtime = 0;
	m_face_realtime_identify = false;
	m_face_realtime_info = false;
	m_face_no = 0;
	m_inntf_status = false;
	m_outntf_status = false;
	m_face_status = false;
	m_identify = 0;;
	m_shapshot = false;
	m_direction=0;//0 front 
	m_live = 0;
	m_live_history_state = false;
	m_error = 0;
}


robot_face_process::~robot_face_process()
{
	stop();
	SAFE_DELETE(m_featurelist.thread);
	SAFE_DELETE(m_detectlist.detect);
	SAFE_DELETE(m_livedetectlist.thread);
}

bool robot_face_process::init(int no)
{
	char name_buffer[1024] = { 0 };
	sprintf_s(name_buffer, 1024, "FaceConfig_%d", no);
	m_face_no = no;
	m_camera_readername = name_buffer;
	m_camera_readername.append("_reader");
	std::string temp;
	if (!common_config_get_field_string(name_buffer, "defaultdetectalg", &temp))
		temp = "libface";

	if (!set_detect_alg(temp))return false;

	if (!common_config_get_field_int(name_buffer, "minface", &m_minface_ntf_th))
		m_minface_ntf_th = 5;

	if (!common_config_get_field_string(name_buffer, "direction", &temp))
		temp = "front";

	if (temp.compare("front") == 0)
		m_direction = 0;
	else
		m_direction = 1;

	if (!common_config_get_field_int(name_buffer, "enableidentify", &m_identify))
		m_identify = 0;

	if (m_identify)
	{
		if (!common_config_get_field_string(name_buffer, "defaultfeaturealg", &temp))
			temp = "seetaface";

		if (!set_feature_alg(temp))return false;
	}
		

	if (!common_config_get_field_int(name_buffer, "record", &m_record))
		m_record = 0;

	if (!common_config_get_field_string(name_buffer, "streamname", &m_streams_name) || m_streams_name.empty())
	{
		LOGE("face config error :name empty");
		m_error = VIS_DEVICE_CONFIG_ERROR;
		return false;
	}

	if (!common_config_get_field_int(name_buffer, "live", &m_live))
		m_live = 0;

	if (m_live)
	{
		if (!common_config_get_field_string(name_buffer, "livestreamname", &m_livestreams_name) || m_livestreams_name.empty())
		{
			LOGE("face config error :name empty");
			m_error = NIS_DEVICE_CONFGI_ERROR;
			return false;
		}
		if (!common_config_get_field_string(name_buffer, "defaultlive", &temp))
			temp = "yuncong";

		if (!set_live_alg(temp))return false;

		int spot = 0;
		if (!common_config_get_field_int(name_buffer, "livesnapshot", &spot))
			spot = 0;

		if (spot&&m_live)
			m_shapshot = true;
		else
			m_shapshot = false;

	}


	m_name = name_buffer;
	m_error = 0;
	return true;
	
}

void robot_face_process::stop()
{
	stop_detect();
	if (m_featurelist.thread&&m_featurelist.thread->state())
		m_featurelist.thread->quit();
}

void robot_face_process::set_debug(bool flag)
{
	m_show_face_debug = flag;
}

void robot_face_process::set_identify(bool b)
{
	if (m_identify)
		m_face_realtime_identify = b;
}

void robot_face_process::set_faceinfo(bool b)
{
	m_face_realtime_info = b;
}

void  robot_face_process::get_streamname(std::vector<std::string>& listname)
{
	listname.clear();
	listname.push_back(m_streams_name);
	if (m_live)
		listname.push_back(m_livestreams_name);
}

int robot_face_process::start_detect_task(FaceMessage &msg)
{
	if (state())return 0;
	m_wait_face_image = false;
	m_waitface_endtime = 0;
	if (m_streams_name.empty() || !m_detectlist.detect)
		return VIS_DEVICE_CONFIG_ERROR;

	if (m_live)
	{
		if (m_livestreams_name.empty() || !m_livedetectlist.thread)
			return NIS_DEVICE_CONFGI_ERROR;
	}

	if (!m_message_wait.fromguid.empty())
		return WAIT_OTHER_MODULE;
	
	m_message_wait.error = 1;
	m_message_wait = msg;
	m_resultvis = 0; m_resultnis = 0;
	m_resultcount = 0;
	open_camera();
	return 2;
}

int robot_face_process::camera_message(int result, std::string name, FaceMessage &msg)
{
	msg = m_message_wait;
	

	if (name.compare(m_streams_name) == 0)
		m_resultvis = result;
	else
		m_resultnis = result;

	m_resultcount++;

	if (m_live)
	{
		if (m_resultcount == 2)
		{
			if (state())
				m_error = 0;
			else
			{
				if (m_resultvis == 0 && m_resultnis == 0)
					m_error = readly_start();
				else
				{
					if (m_resultvis != 0 && m_resultnis != 0)
						m_error = LIVE_ALL_DEVICE_OPEN_ERROR;
					else if (m_resultvis == 0)
						m_error = LIVE_VIS_DEVICE_OPEN_ERROR;
					else
						m_error = LIVE_NIS_DEVICE_OPEN_ERROR;
					close_camera();
				}
			}
			m_message_wait.fromguid.clear();
			return m_error;			
		}
		else
			return 2;
	
	}
	else
	{
		if (state())
			m_error = 0;
		else
		{
			if (m_resultvis == 0)
				m_error = readly_start();
			else
				m_error = DEVICE_OPEN_ERROR;
		}
		m_message_wait.fromguid.clear();
		return m_error;

	}
	m_error = 1;
	return m_error;
}

void robot_face_process::clean_message()
{
	while (1)
	{
		Message msg = getq(0);
		if (msg.msgType == -1)break;
		switch (msg.msgType)
		{
		case  FACE_SNAPSHOT_MESSAGE:
		{
			if (msg.msgObject == NULL)break;
			FaceMessage *fm = (FaceMessage *)msg.msgObject;
			fm->error = 90;
			fm->content = json_request.face_snapshot(*fm);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, fm->content.c_str(), fm->content.size(), fm->fromguid.c_str());
			SAFE_DELETE(fm);
			break;
		}
		case FACE_REGISTER_MESSAGE:
		{
			if (msg.msgObject == NULL)break;
			FaceMessage *msg_ptr = (FaceMessage*)msg.msgObject;
			SAFE_DELETE(msg_ptr);
		}
		break;
		case CAMERA_SNAPSHOT_MESSAGE:
		{
			if (msg.msgObject == NULL)break;
			FaceMessage *fm = (FaceMessage *)msg.msgObject;
			fm->error = 90;
			fm->content = json_request.live_snapshot(*fm);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, fm->content.c_str(), fm->content.size(), fm->fromguid.c_str());
			SAFE_DELETE(fm);
			break;
		}
		default:
			break;
		}

	}
}

int robot_face_process::readly_start()
{
	if (m_pm != NULL)
	{
		Processon_Memory_Close(m_pm);
		m_pm = NULL;
	}

	if (m_pm == NULL)
	{
		m_pm = Processon_Memory_Open(m_streams_name.c_str(), m_name.c_str());
		if (!m_pm)
		{
			LOGE("face process memory not create !!");
			m_error = FACE_MEMORY_OPEN_ERROR;
			return m_error;
		}
	}
	if (m_live&&m_livedetectlist.thread)
	{
		if (!m_livedetectlist.thread->open(m_livestreams_name))
		{
			LOGE("face live process memory not create !!");
			m_error = LIVE_CREATE_ERROR;
			return m_error;
		}
	}

	clean_message();
	return start() ? 0 : 1;
}

void robot_face_process::get_status(face_status_info &fd)
{
	fd.run_state = state();
	fd.error = m_error;
	fd.has_person =m_face_status;
}


void robot_face_process::open_camera()
{
	Json::Value   json_object;
	Json::Reader  json_reader;
	Json::FastWriter  json_writer;
	json_object["msg_type"] = Json::Value("CAMERA_START_STREAMNAME_REQUEST");
	json_object["reader"] = Json::Value(m_camera_readername);
	json_object["streamname"] = Json::Value(m_streams_name);
	std::string report = json_writer.write(json_object);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, report.c_str(), report.size(), "video_stream_module");
	if (m_live)
	{
		json_object["streamname"] = Json::Value(m_livestreams_name);
		report = json_writer.write(json_object);
		robot_PostMessage::getPostManager()->post(MODULE_NAME, report.c_str(), report.size(), "video_stream_module");
	}
}


void robot_face_process::close_camera()
{
	Json::Value   json_object;
	Json::Reader  json_reader;
	Json::FastWriter  json_writer;
	json_object["msg_type"] = Json::Value("CAMERA_STOP_STREAMNAME_REQUEST");
	json_object["streamname"] = Json::Value(m_streams_name);
	json_object["reader"] = Json::Value(m_camera_readername);
	std::string report = json_writer.write(json_object);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, report.c_str(), report.size(), "video_stream_module");
	if (m_live)
	{
		json_object["streamname"] = Json::Value(m_livestreams_name);
		report = json_writer.write(json_object);
		robot_PostMessage::getPostManager()->post(MODULE_NAME, report.c_str(), report.size(), "video_stream_module");
	}
}


void robot_face_process::stop_detect()
{
	if (!state())
		return;
	m_show_face_debug = false;
	quit();
	close_camera();
	if (m_live&&m_livedetectlist.thread)
		m_livedetectlist.thread->close_live();
}

void robot_face_process::start_wait_face_image(FaceMessage client_request)
{
	client_request.cameralist.clear();
	client_request.cameralist.push_back(m_face_no);
	if (state())
	{
		FaceMessage *fm = new FaceMessage;
		*fm = client_request;
		pushq(FACE_SNAPSHOT_MESSAGE, "", fm);
	}
	else
	{
		client_request.error = 96;
		client_request.content = json_request.face_snapshot(client_request);
		robot_PostMessage::getPostManager()->post(MODULE_NAME, client_request.content.c_str(), client_request.content.size(), client_request.fromguid.c_str());
	}
}

void robot_face_process::start_wait_normal_image(FaceMessage client_request)
{
	client_request.cameralist.clear();
	client_request.cameralist.push_back(m_face_no);
	if (state())
	{
		FaceMessage *fm = new FaceMessage;
		*fm = client_request;
		pushq(CAMERA_SNAPSHOT_MESSAGE, "", fm);
	}
	else
	{
		client_request.error = 96;
		client_request.content = json_request.live_snapshot(client_request);
		robot_PostMessage::getPostManager()->post(MODULE_NAME, client_request.content.c_str(), client_request.content.size(), client_request.fromguid.c_str());
	}
}

void robot_face_process::stop_wait_face_image()
{
	pushq(FACE_CANCELSNAPSHOT_MESSAGE);
}

bool robot_face_process::can_register(std::vector<FaceRect>  &img)
{
	for (int i = 0; i < img.size(); i++)
	{
		int _x = (img[i].width / 2 + img[i].x);
		int _y = (img[i].height / 2 + img[i].y);
		
		_x = abs((CANMERA_WIDTH / 2) - _x);
		_y = abs((CANMERA_HEIGHT / 2) - _y);

		int _dis = sqrt(_x*_x + _y*_y);

		if (200 > _dis)
			return true;
	}
	return false;
}

void robot_face_process::get_processer_content(face_process_content& content)
{
	content.cameraid = m_face_no;
	content.direction = m_direction;
	content.enable_identify = m_identify;
	content.camera_readername = m_camera_readername;
}

void robot_face_process::run()
{

	m_live_history_state = false;

	std::vector<unsigned char> buffer;	
	buffer.resize(256);
	if (!malloc_buffer(buffer))
	{
		LOGE("camera data empty");
		return;
	}


	LOGI("ID : %d face detect start", m_face_no);

	Timer_helper fps_timer, hasperson_timer,faceinfo_timeout_check;

	int fps=0;

	FaceRect minface;
	minface.width =  10;
	minface.height =  10;

	bool  history_state = false;

	bool loop = true;

	bool livestate[2] = { false };
	unsigned char indexlive = 0;
	bool livesnap = false;
	FaceMessage  *livetask = NULL;

	while (loop)
	{

		Message msg = getq(0);
		switch (msg.msgType)
		{
		case  FACE_SNAPSHOT_MESSAGE:
		{
			FaceMessage *fm = (FaceMessage *)msg.msgObject;
			if (fm == NULL)break;
			if (!m_wait_face_image)
			{
				m_waitface_endtime = common_get_cur_time_stampms();
				m_waitface_endtime += fm->timeout * 1000;
				m_wait_face_image = true;
				m_current_task.task_message = *fm;
			}
			else
			{
				fm->error = 99;
				fm->content = json_request.face_snapshot(*fm);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, fm->content.c_str(), fm->content.size(), fm->fromguid.c_str());
			}
			SAFE_DELETE(fm);
			break;
		}
		case FACE_CANCELSNAPSHOT_MESSAGE:
		{
			m_wait_face_image = false;
			break;
		}
		case FACE_REGISTER_MESSAGE:
		{
			if (msg.msgObject == NULL)break;
			FaceMessage *msg_ptr = (FaceMessage*)msg.msgObject;
			face_register(msg_ptr);
			SAFE_DELETE(msg_ptr);
		}
		break;
		case CAMERA_SNAPSHOT_MESSAGE:
		{
			FaceMessage *fm = (FaceMessage *)msg.msgObject;
			if (fm == NULL)break;
			if (livesnap == true)
			{
				fm->error = 99;
				fm->content = json_request.live_snapshot(*fm);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, fm->content.c_str(), fm->content.size(), fm->fromguid.c_str());
				SAFE_DELETE(fm);
			}
			else
			{
				livetask = fm;
				livesnap = true;
			}

			break;
		}
		case _EXIT_THREAD_:
			loop = false;
			break;
		default:;
		}

		unsigned int data_len = buffer.size();
		int pm_ret = Processon_Memory_Read(m_pm, buffer.data(), &data_len);
		if (pm_ret < 0)
		{
			m_error = 202;
			LOGE("processon memory occur error!!");
			common_thread_sleep(1000);
			continue;;

		}

		if (pm_ret==0)
		{
			VideoMemoryHeadFormat *head_info = (VideoMemoryHeadFormat *)buffer.data();

			if ((common_get_longcur_time_stampms() - head_info->stime)>(1000 / (head_info->fps)+5))
				continue;

			fps++;
			m_error = 0;
			if (fps_timer.isreach(1000))
			{
				printf("[%d] video fps :%d\n", m_face_no,fps );
				fps = 0;
				fps_timer.update();
			}
			
 			cv::Mat frame;
 
			get_stand_frame(buffer.data(), data_len, frame);

			if (frame.empty())
				continue;


			//////////////////////////////////////////////////////////////////////////
			if (livesnap)
			{
				save_temp_live_image(livetask, frame);
				livesnap = false;
				SAFE_DELETE(livetask);
			}
			/////////////////////////////////////////////
			std::vector <FaceRect> live_face_list;
			FaceDetectData liveinfo;

			bool live_ret = false;
 			/////////////////////////////////////////////
			std::vector<FaceRect> track_allface_list;
 
 			std::vector<FaceRect> nearest_filter_face_list;

			if (m_live&&m_livedetectlist.thread)
				live_ret = m_livedetectlist.thread->handle_image(frame, m_detectlist.detect);

 			m_detectlist.detect->DetectFace(frame, minface, track_allface_list);

			if (m_live&&live_ret&&m_livedetectlist.thread)
			{
				m_livedetectlist.thread->get_result(liveinfo);

				livestate[indexlive++ % 2] = liveinfo.status;
			}
			convert_traceface2liveface(liveinfo, track_allface_list, live_face_list);

			fix_sex(track_allface_list);

			if (m_live)
				show_debug_face_windows(m_livestreams_name, live_face_list, liveinfo.nis_img);

			show_debug_face_windows(m_streams_name, track_allface_list, frame);

			for (int i = 0; i < track_allface_list.size(); i++)
			{
				if (!history_state&&hasperson_timer.isreach(1000))
				{
					if (face_arae_map[m_minface_ntf_th] != 0)
					{
						if (track_allface_list[i].width*track_allface_list[i].height < face_arae_map[m_minface_ntf_th])
							continue;
						else
						{
							if (!history_state&&track_allface_list[i].score < 0.5)
								continue;
						}
						if (m_live)
						{
							if (livestate[0] == false || livestate[1] == false)
								continue;
						}

					}
				}
				else
				{
					if (m_minface_ntf_th>=2&&track_allface_list[i].width*track_allface_list[i].height < face_arae_map[m_minface_ntf_th - 2])
						continue;
				}	
				if (m_live)
				{
					if (!track_allface_list[i].isLive&&track_allface_list[i].valid == 1)
					{
						if (track_allface_list[i].trackId > 0)
						{
							if (m_sex_map[track_allface_list[i].trackId].livesum < 10)
								continue;
							if (m_sex_map[track_allface_list[i].trackId].livecount < (((double)m_sex_map[track_allface_list[i].trackId].livesum*0.1)))
								continue;
						}
					}
				}


				nearest_filter_face_list.push_back(track_allface_list[i]);
			}


			bool now_state = nearest_filter_face_list.size() == 0 ? false : true;


			input_identify(live_face_list, track_allface_list, frame);

			if (m_face_realtime_info )
			{
				FacesPositionInfo *ftemp = new FacesPositionInfo();
				ftemp->cameraid = m_face_no;
				ftemp->faces = track_allface_list;
				robot_face_msghandler::getPtr()->pushq(FACES_POSITION_INFO,"" , ftemp);
			}

			snapshot(live_face_list, track_allface_list, frame);


			if (now_state)hasperson_timer.update();

			if (now_state)
			{
				m_face_in.face_img = frame;
				m_face_in.face_list = nearest_filter_face_list;
			}

			if (now_state != history_state)
			{
				if (now_state)
				{
					m_face_in.detect_time = common_get_cur_time_stampms();
					m_face_in.status = true;
					m_face_in.cameraid = m_face_no;

				}
				else
				{
					m_face_out.detect_time = common_get_cur_time_stampms();
					m_face_out.status = false;
					m_face_out.cameraid = m_face_no;
				}

			}

			history_state = now_state;
			m_face_status = now_state;
		}

		face_state_ntf();
	}
	if (m_wait_face_image)
	{
		m_current_task.task_message.error = 101;
		m_current_task.task_message.content = json_request.face_snapshot(m_current_task.task_message);
		robot_PostMessage::getPostManager()->post(MODULE_NAME, m_current_task.task_message.content.c_str(), m_current_task.task_message.content.size(), m_current_task.task_message.fromguid.c_str());
		m_wait_face_image = false;
	}
	if (m_show_face_debug)
	{
		cv::destroyAllWindows();
		if (m_windowsdbg[0].state())m_windowsdbg[0].exitWindows();
		if (m_windowsdbg[1].state())m_windowsdbg[1].exitWindows();
	}
		

	if (m_pm != NULL)
	{
		Processon_Memory_Close(m_pm);
		m_pm = NULL;
	}
	LOGW("ID : %d face detect exit",m_face_no);
}

void robot_face_process::input_identify(std::vector<FaceRect> &live_face_list, std::vector<FaceRect> &track_face_list, cv::Mat &frame)
{
	if (m_face_realtime_identify &&
		m_featurelist.thread)

	{
		std::vector<FaceRect> face_source;
		if (m_shapshot)
			face_source = live_face_list;
		else
			face_source = track_face_list;

		bool live_face_allvaild = true;

		for (int i = 0; i < face_source.size(); i++)
		{
			if (!face_source[i].valid)live_face_allvaild = false;

			if (m_detectlist.facetrack&& face_source[i].trackId < 0)live_face_allvaild = false;

		}
		if (face_source.size() == 0)live_face_allvaild = false;

		bool live_now_state = face_source.size() == 0 ? false : true;


		if (((live_now_state != m_live_history_state) ||
			((identify_time_interval.isreach(m_detectlist.identifyfps) && live_now_state&&live_face_allvaild)))
			&& m_featurelist.thread->can_idenify())
		{
			FaceDetectData *identifyframe = new FaceDetectData;
			identifyframe->cameraid = m_face_no;
			identifyframe->detect_time = common_get_cur_time_stampms();
			identifyframe->face_img = frame;
			identifyframe->face_list = face_source;
			identifyframe->status = true;
			m_featurelist.thread->pushq(FACE_IDENTIFY_HANDLE_TASK, "", identifyframe);
			identify_time_interval.update();
		}
		m_live_history_state = live_now_state;
	}

}

void robot_face_process::snapshot(std::vector<FaceRect> &live_face_list, std::vector<FaceRect> &track_face_list, cv::Mat &frame)
{
	if (m_wait_face_image)
	{
		std::vector<FaceRect> face_snapshot;
		if (m_shapshot)
			face_snapshot = live_face_list;
		else
			face_snapshot = track_face_list;

		bool can_photo = false;
		bool snapshot_face_allvaild = true;

		for (int i = 0; i < face_snapshot.size(); i++)
		{
			if (!face_snapshot[i].valid)snapshot_face_allvaild = false;

			if (m_detectlist.facetrack&& face_snapshot[i].trackId < 0)snapshot_face_allvaild = false;

			if (face_snapshot[i].score > 0.65&&face_snapshot[i].takephoto)can_photo = true;
		}
		if (face_snapshot.size() == 0)snapshot_face_allvaild = false;


		time_t   nowtime = common_get_cur_time_stampms();
		if (can_photo&&face_snapshot.size() > 0 && snapshot_face_allvaild)
		{
			m_current_task.realtime_image_cache_info.face_img = frame;
			m_current_task.realtime_image_cache_info.cameraid = m_face_no;
			m_current_task.realtime_image_cache_info.face_list = face_snapshot;
			m_current_task.realtime_image_cache_info.detect_time = common_get_cur_time_stampms();
			m_current_task.realtime_image_cache_info.status = true;
			save_temp_face_image();
			m_wait_face_image = false;
		}
		else
		{
			if (nowtime > m_waitface_endtime)
			{
				m_current_task.task_message.error = 101;
				m_current_task.task_message.content = json_request.face_snapshot(m_current_task.task_message);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, m_current_task.task_message.content.c_str(), m_current_task.task_message.content.size(), m_current_task.task_message.fromguid.c_str());
				m_wait_face_image = false;
			}
		}
	}
}

void robot_face_process::convert_traceface2liveface(FaceDetectData &liveinfo, std::vector<FaceRect> &track_allface_list, std::vector<FaceRect> &live_face_list)
{
	for (int i = 0; i < track_allface_list.size(); i++)
	{
		track_allface_list[i].trackId = track_allface_list[i].trackId >= 0 ? 10000 + track_allface_list[i].trackId : -1;
	}
	if (liveinfo.status)
	{
		for (int i = 0; i < track_allface_list.size(); i++)
		{
			if (track_allface_list[i].valid == 1)
			{
				for (int j = 0; j < liveinfo.face_list.size(); j++)
				{
					int xleft = MAX(track_allface_list[i].x, liveinfo.face_list[j].x);
					int xright = MIN(track_allface_list[i].width + track_allface_list[i].x, liveinfo.face_list[j].width + liveinfo.face_list[j].x);
					int ytop = MAX(track_allface_list[i].y, liveinfo.face_list[j].y);
					int ybottom = MIN(track_allface_list[i].height + track_allface_list[i].y, liveinfo.face_list[j].height + liveinfo.face_list[j].y);
					int totle = track_allface_list[i].width*track_allface_list[i].height + liveinfo.face_list[j].height*liveinfo.face_list[j].width;
					if (xleft < xright&&ytop < ybottom)
					{
						int aero = (xright - xleft)*(ybottom - ytop);
						if (((aero * 200) / totle)>40)
							track_allface_list[i].isLive = liveinfo.face_list[j].isLive;
					}

				}
			}
		}
		for (int i = 0; i < track_allface_list.size(); i++)
		{
			if (track_allface_list[i].isLive)
				live_face_list.push_back(track_allface_list[i]);
		}
	}
	else
	{
		if (!m_live)
		{
			for (int i = 0; i < track_allface_list.size(); i++)
			{
				live_face_list.push_back(track_allface_list[i]);
			}
		}

	}
}

bool robot_face_process::malloc_buffer(std::vector<unsigned char> &buffer)
{
	int pm_ret = -1;
	int outt = 60;
	while (pm_ret)
	{
		unsigned int data_len = buffer.size();
		pm_ret = Processon_Memory_Read(m_pm, buffer.data(), &data_len);
		if (pm_ret < 0)
		{
			m_error = 202;
			LOGE("processon memory occur error %d!!", pm_ret);
			return false;
		}
		if (pm_ret > 0)
		{
			switch (pm_ret)
			{
			case 1001:
				if (outt-- <= 0)return false;
				break;
			case 1002:
				buffer.resize(data_len);
				break;
			default:
				break;
			}
			continue;

		}
		else
		{
			if (face_arae_map.empty())
			{
				VideoMemoryHeadFormat *head_info = (VideoMemoryHeadFormat *)buffer.data();

				for (int i = 0; i < 50; i++)
				{
					face_arae_map[i] = (head_info->height + head_info->width)*(0.5*i + (i*i)*0.02);
					LOGD("face notify %d ------%0.5f", i, face_arae_map[i]);
				}


			}
		}
	}
	m_error = 0;
	return true;
}

void robot_face_process::fix_sex(std::vector<FaceRect> &face_list)
{
	for (int i = 0; i < face_list.size(); i++)
	{
		if (m_detectlist.facetrack&&face_list[i].trackId > 0)
		{
			if (face_list[i].valid == 1)
			{
				if (face_list[i].isLive)
					m_sex_map[face_list[i].trackId].livecount++;
				m_sex_map[face_list[i].trackId].livesum++;
			}
		}
		if (m_detectlist.facetrack&&face_list[i].sex&&face_list[i].trackId>0)
		{
			m_sex_map[face_list[i].trackId].sum += face_list[i].sex;
			m_sex_map[face_list[i].trackId].count++;

			if (m_sex_map[face_list[i].trackId].count != 0)
			{
				double sex_a = ((double)(m_sex_map[face_list[i].trackId].sum)) / m_sex_map[face_list[i].trackId].count;
				if (sex_a >= 1.0)
				{
					if (sex_a >= 1.5)
						face_list[i].sex = 2;
					if (sex_a < 1.5)
						face_list[i].sex = 1;
				}
			}
			else
			{
				m_sex_map[face_list[i].trackId].sum = 0;//long long ago
			}

		}

	}


	auto it = m_sex_map.begin();
	for (; it != m_sex_map.end(); )
	{
		bool remove_id = true;;
		for (int i = 0; i < face_list.size(); i++)
		{
			if (it->first == face_list[i].trackId)
				remove_id = false;
		}
		if (remove_id)
			it=m_sex_map.erase(it);
		else
			it++;
	}

}

void robot_face_process::show_debug_face_windows(std::string streamname, std::vector<FaceRect> &face_list, cv::Mat &temp_face)
{
	if (temp_face.empty())return;
	std::string windowname = streamname;
	windowname.append("-»À¡≥º‡≤‚");

	int optindex = 0;
	if (streamname.compare(m_streams_name) == 0)
		optindex = 0;
	else
		optindex = 1;

	if (m_last_show_state != m_show_face_debug)
	{
		if (m_show_face_debug)
			m_last_show_state = m_show_face_debug;
		else
		{
			if (m_windows_namelist.find(windowname) != m_windows_namelist.end())
			{
				m_windows_namelist.erase(windowname);
				m_windowsdbg[optindex].exitWindows();
			}

			if (m_windows_namelist.size() == 0)
				m_last_show_state = m_show_face_debug;
		}

	}


	if (m_show_face_debug)
	{
		cv::Mat frame;
		temp_face.copyTo(frame);
		for (unsigned int i = 0; i < face_list.size(); i++)
		{
			cv::Rect temprec;
			CvScalar color;
			char textidbuffer[1024] = { 0 };
			temprec.x = face_list[i].x;
			temprec.y = face_list[i].y;
			temprec.width = face_list[i].width;
			temprec.height = face_list[i].height;
			
			if (face_list[i].isLive)
			{
				if (face_list[i].sex == 0)
					color = CV_RGB(130, 230, 130);
				else if (face_list[i].sex == 1)
					color = CV_RGB(30, 230, 0);
				else
					color = CV_RGB(30, 230, 100);
			}
			else
			{
				if (face_list[i].sex == 0)
					color = CV_RGB(220, 130, 130);
				else if (face_list[i].sex == 1)
					color = CV_RGB(220, 30, 0);
				else
					color = CV_RGB(220, 30, 100);
			}
			rectangle(frame, temprec, color, 2);
			sprintf_s(textidbuffer, 1024, "%d(%.2f)", face_list[i].trackId,face_list[i].score);
			cv::putText(frame, textidbuffer, cv::Point(temprec.x + temprec.width, temprec.y), CV_FONT_HERSHEY_SIMPLEX, 0.5, color, 1, 2);
		}
		if (m_windows_namelist.find(windowname) == m_windows_namelist.end())
			m_windowsdbg[optindex].Init(windowname);
		m_windows_namelist.insert(windowname);
		m_windowsdbg[optindex].ShowMat(frame);
	}




}

void robot_face_process::face_state_ntf()
{
	if (m_face_status)
	{
		int time_dtel = common_get_cur_time_stampms() - m_face_in.detect_time;
		if (!m_inntf_status&&time_dtel>FACE_IN_PULLUP)
		{
			m_inntf_status = true;
			m_outntf_status = false;



			FaceDetectData *ftemp = new FaceDetectData();
			*ftemp = m_face_in;

			robot_face_msghandler::getPtr()->pushq(FACE_DETECT_NTF, "", ftemp);

		}

	}
	else
	{
		if (m_inntf_status)
		{
			int time_dtel = common_get_cur_time_stampms() - m_face_out.detect_time; 
			if (!m_outntf_status&&time_dtel>m_detectlist.down_face_pull)
			{
				m_outntf_status = true;
				m_inntf_status = false;

				FaceDetectData *ftemp = new FaceDetectData();
				ftemp->cameraid = m_face_no;
				ftemp->status = false;
				ftemp->detect_time = common_get_cur_time_stampms();

				robot_face_msghandler::getPtr()->pushq(FACE_DETECT_NTF, "", ftemp);

			}
		}

	}
}


void robot_face_process::get_stand_frame(unsigned char * buffer, unsigned int data_len, cv::Mat &frame)
{
	int type = CV_8UC3;

	frame.release();

	VideoMemoryHeadFormat * head_info = (VideoMemoryHeadFormat *)buffer;

	frame.create(head_info->height, head_info->width, type);

	memcpy(frame.data, buffer + sizeof(VideoMemoryHeadFormat), data_len - sizeof(VideoMemoryHeadFormat));

	if (head_info->color_type == VF_RGB)
	{
		cvtColor(frame, frame, CV_RGB2BGR);
	
	}

	if (head_info->color_type == VF_BGR)
	{
		;
	}
}

bool robot_face_process::set_detect_alg(std::string name)
{
	if (state())return false;
	m_detectlist.algname = name;
	m_detectlist.m_outntf_status = false;
	m_detectlist.m_face_status = false;
	m_detectlist.m_inntf_status = false;
	SAFE_DELETE(m_detectlist.detect);
	face_arae_map.clear();
	int default_facepuldown = FACE_IN_PULLDOWN;
	std::string alg_name = "libface";

	if (name.compare("seetaface") == 0)
	{
		m_detectlist.detect = new robot_face_detect_seetaface;	
		alg_name = "seetaface";
		default_facepuldown = 2000;		
	}	
#ifdef HAS_YUNCONG
	else if(name.compare("yuncong") == 0)
	{
		m_detectlist.detect = new robot_face_detect_yuncong;
		alg_name = "yuncong";
		default_facepuldown = 1300;
	}
#endif // HAS_YUNCONG
#ifdef HAS_ARCSOFT
	else if (name.compare("arcsoft") == 0)
	{
		m_detectlist.detect = new robot_face_detect_arcsoft;
		alg_name = "arcsoft";
		default_facepuldown = 1500;
	}
#endif // HAS_ARCSOFT
#ifdef HAS_READFACE
	else if (name.compare("readface") == 0)
	{
		m_detectlist.detect = new robot_face_detect_readface;
		alg_name = "readface";
		default_facepuldown = 1500;
	}
#endif // HAS_READFACE
	else
	{
		m_detectlist.detect = new robot_face_detect_libfacedetect;
		alg_name = "libface";
		default_facepuldown = 7000;
	}
	
	m_detectlist.detect->setMode();

	if (!common_config_get_field_int(alg_name.c_str(), "identifyfps", &m_detectlist.identifyfps))
		m_detectlist.identifyfps = 100;

	if (!common_config_get_field_int(alg_name.c_str(), "facepulldown", &m_detectlist.down_face_pull))
		m_detectlist.down_face_pull = default_facepuldown;

	if (!common_config_get_field_int(alg_name.c_str(), "facetrack", &m_detectlist.facetrack))
		m_detectlist.facetrack = 0;

	LOGI("[%d] face detect by %s", m_face_no, alg_name.c_str());

	return true;

}

bool robot_face_process::set_feature_alg(std::string name)
{
	if (state())return false;

	if (m_featurelist.thread&&m_featurelist.thread->state())
		m_featurelist.thread->quit();
	SAFE_DELETE(m_featurelist.thread);

	if (name.empty())
		name = "seetaface";

	robot_face_feature_interface * featureinterface;
#ifdef HAS_YUNCONG
	if (name.compare("yuncong") == 0)
		featureinterface = new robot_face_features_yuncong();
	else
#endif // HAS_YUNCONG
#ifdef HAS_ARCSOFT
	if (name.compare("arcsoft") == 0)
		featureinterface = new robot_face_features_arcsoft();
	else
#endif // HAS_ARCSOFT
#ifdef HAS_READFACE
	if (name.compare("readface") == 0)
		featureinterface = new robot_face_features_readface();
	else
#endif // HAS_READFACE
	{
		featureinterface = new robot_face_features_seetaface();
		name = "seetaface";
	}


	m_featurelist.algname = name;
	m_featurelist.thread = new robot_face_features;
	if (m_featurelist.thread == NULL)return false;
	if (m_featurelist.thread->init(m_face_no, name, featureinterface) && m_featurelist.thread->start())
	{
		LOGI("[%d] face identify by %s", m_face_no, name.c_str());
		return true;
	}
	else
	{
		m_error = 204;
		SAFE_DELETE(m_featurelist.thread);
		return false;
	}
	

}

bool robot_face_process::set_live_alg(std::string name)
{
	if (state())return false;

	if (m_livedetectlist.thread)
	{
		if (m_livedetectlist.thread->state())
			m_livedetectlist.thread->close_live();
	}
	SAFE_DELETE(m_livedetectlist.thread);
	std::string alg_name;
#ifdef HAS_YUNCONG
	alg_name = "yuncong";
	if (name.compare("yuncong") == 0)
		m_livedetectlist.thread = new robot_face_live_handle_yuncong;
	else
	{
		m_livedetectlist.thread = new robot_face_live_handle_yuncong;
		alg_name = "yuncong";
	}
#endif // HAS_YUNCONG

	m_livedetectlist.algname = alg_name;
	if (m_livedetectlist.thread == NULL)
		return false;

	if (m_livedetectlist.thread->init() == 0)
	{
		LOGI("[%d] face live detect by %s", m_face_no, alg_name.c_str());
		return true;
	}
	else
	{
		m_error = 205;
		m_livedetectlist.thread->close_live();
		SAFE_DELETE(m_livedetectlist.thread);
		return false;
	}
}

void robot_face_process::check_image_save_path()
{
	std::string path;
	common_get_current_dir(&path);
	common_fix_path(&path);
	path.append(IMAGE_SAVE_PATH);//sql

	if (!common_isDirectory(path.c_str()))
		common_create_dir(path.c_str());

	m_image_save_path = path;
}

void robot_face_process::save_temp_live_image(FaceMessage * taskinfo,cv::Mat &image)
{
	RealtimeIdentifyFaceInfo temp;
	if (taskinfo == NULL) return;

	if (taskinfo->savetype == 1)
	{
		check_image_save_path();
		std::string all_path = m_image_save_path;
		all_path.append("live.jpg");
		temp.data = all_path;
		if (imwrite(all_path, image))
			taskinfo->list.push_back(temp);
		else
			LOGE("write live image failed ");
	}
	else if (taskinfo->savetype == 2)
	{
		std::vector<unsigned char > out;
		if (imencode(".jpg", image, out))
		{
			std::string str;
			common_base64_encode_string(out.data(), out.size(), &str);
			if (str.size() != 0)
			{
				temp.data = str;
				taskinfo->list.push_back(temp);
			}
			else
				LOGE("base64_encode_string live image failed ");
		}
		else
			LOGE("imencode live image failed ");

	}
	else
	{

	}
	if (taskinfo->list.size() == 0 && taskinfo->savetype != 0)
		taskinfo->error = 90;
	else
		taskinfo->error = 0;

	taskinfo->content = json_request.live_snapshot(*taskinfo);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, taskinfo->content.c_str(), taskinfo->content.size(), taskinfo->fromguid.c_str());
}


void robot_face_process::save_temp_face_image()
{

	for (int i = 0; i < m_current_task.realtime_image_cache_info.face_list.size(); i++)
	{
		RealtimeIdentifyFaceInfo temp;

		if (!m_current_task.realtime_image_cache_info.face_list[i].takephoto)continue;

		int range_add = 30;


		cv::Rect rect;


		rect.x = m_current_task.realtime_image_cache_info.face_list[i].x - range_add < 0 ? 0 : m_current_task.realtime_image_cache_info.face_list[i].x - range_add;
		rect.y = m_current_task.realtime_image_cache_info.face_list[i].y - range_add < 0 ? 0 : m_current_task.realtime_image_cache_info.face_list[i].y - range_add;

		rect.width = ((rect.x + m_current_task.realtime_image_cache_info.face_list[i].width) + 2 * range_add) >= m_current_task.realtime_image_cache_info.face_img.cols ?
			(m_current_task.realtime_image_cache_info.face_img.cols - rect.x - 1) : ((m_current_task.realtime_image_cache_info.face_list[i].width) + 2 * range_add);

		rect.height = ((rect.y + m_current_task.realtime_image_cache_info.face_list[i].height) + 2 * range_add) >= m_current_task.realtime_image_cache_info.face_img.rows ?
			(m_current_task.realtime_image_cache_info.face_img.rows - rect.y - 1) : ((m_current_task.realtime_image_cache_info.face_list[i].height) + 2 * range_add);


		cv::Mat image_roi;
		m_current_task.realtime_image_cache_info.face_img(rect).copyTo(image_roi);

		if (m_current_task.task_message.savetype == 1)
		{
			check_image_save_path();
			std::string all_path = m_image_save_path;
			//std::string filename = get_guid();
			std::string temmp_filename;

			if (m_current_task.realtime_image_cache_info.face_list[i].trackId == -1)
				m_current_task.realtime_image_cache_info.face_list[i].trackId = i;

			common_itoa_x(m_current_task.realtime_image_cache_info.face_list[i].trackId, &temmp_filename);
			std::string filename = temmp_filename;
			filename.append(".bmp");
			all_path.append(filename);

			
			temp.id = m_current_task.realtime_image_cache_info.face_list[i].trackId;
			temp.data = all_path;
			if (imwrite(all_path, image_roi))
				m_current_task.task_message.list.push_back(temp);
			else
				LOGE("write face image failed ");
		}
		else if (m_current_task.task_message.savetype == 2)
		{
			std::vector<unsigned char > out;
			if (imencode(".jpg", image_roi, out))
			{
				std::string str;
				common_base64_encode_string(out.data(), out.size(), &str);
				if (str.size() != 0)
				{
					temp.data = str;
					if (m_current_task.realtime_image_cache_info.face_list[i].trackId == -1)
						m_current_task.realtime_image_cache_info.face_list[i].trackId = i;
					temp.id = m_current_task.realtime_image_cache_info.face_list[i].trackId;

					m_current_task.task_message.list.push_back(temp);
				}
				else
					LOGE("base64_encode_string face image failed ");
			}
			else
				LOGE("imencode face image failed ");

		}
		else
		{

		}
	}
	if (m_current_task.task_message.list.size() == 0 && m_current_task.task_message.savetype!=0)
		m_current_task.task_message.error = 90;
	else
		m_current_task.task_message.error = 0;

	m_current_task.task_message.content = json_request.face_snapshot(m_current_task.task_message);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, m_current_task.task_message.content.c_str(), m_current_task.task_message.content.size(), m_current_task.task_message.fromguid.c_str());
}

void robot_face_process::face_register(FaceMessage *fmsgptr)
{

	m_current_task.input_image_list.clear();
	m_current_task.task_message = *fmsgptr;
	if (m_featurelist.thread == NULL)
		m_current_task.task_message.error = SERVICE_DISABLE_IDENTIFY;
	else if (fmsgptr->list.size() == 0)
		m_current_task.task_message.error = INPUT_PARA_IVAILD;
	else
	{

		switch (fmsgptr->reg_type)
		{
		case MEMORY_FACE_REGISTER:
		{
			if (m_current_task.realtime_image_cache_info.face_list.size() == 0)
				m_current_task.task_message.error = CACHE_NOFACE;
			else
			{
				FaceTask *face_task_ptr = new FaceTask;
				face_task_ptr->task_message = *fmsgptr;
				face_task_ptr->input_image_list.push_back(m_current_task.realtime_image_cache_info);
// 
// 
// 				face_task_ptr->realtime_image_cache_info = m_current_task.realtime_image_cache_info;
				m_featurelist.thread->pushq(FACE_REGISTER_HANDLE_TASK, "", face_task_ptr);
				m_current_task.realtime_image_cache_info.face_list.clear();
				m_current_task.realtime_image_cache_info.status = false;
				return;
			}
		}
		break;
		case FILE_FACE_REGISTER:
		{
			FaceTask *face_task_ptr = new FaceTask;
			face_task_ptr->task_message = *fmsgptr;

			for (int i = 0; i < fmsgptr->list.size(); i++)
			{
				FaceDetectData  image;
				std::string path;
				common_utf8_2gbk(&fmsgptr->list[i].data, &path);
				if (common_check_file_exist(path.c_str()))
				{
					image.face_img = cv::imread(path.c_str());
					if (!image.face_img.empty())
						package_faceimage(image);
					else
						LOGW("file open failed %s.", path.c_str());
				}
				else
					LOGW("file not exist %s.", path.c_str());
				face_task_ptr->input_image_list.push_back(image);
			}
			m_featurelist.thread->pushq(FACE_REGISTER_HANDLE_TASK, "", face_task_ptr);
			return ;
		}
		break;
		case BASE64_FACE_REGISTER:
		{
			FaceTask *face_task_ptr = new FaceTask;
			face_task_ptr->task_message = *fmsgptr;
			for (int i = 0; i < fmsgptr->list.size(); i++)
			{
				FaceDetectData    image;
				std::vector<unsigned char>  buffer_iamge;
				if (common_base64_decode_string(&fmsgptr->list[i].data, &buffer_iamge) != 0)
				{
					image.face_img = cv::imdecode(buffer_iamge, CV_LOAD_IMAGE_ANYCOLOR);

					if (!image.face_img.empty())
						package_faceimage(image);
					else
						LOGW("cant imdecode image");
				}
				face_task_ptr->input_image_list.push_back(image);
			}
			m_featurelist.thread->pushq(FACE_REGISTER_HANDLE_TASK, "", face_task_ptr);
			return ;
		}
		break;
		default:
			m_current_task.task_message.error = 104;
			LOGE("unknow face register method %d", fmsgptr->reg_type);
		}
	}

	m_current_task.task_message.content = json_request.reg_face_begin(m_current_task.task_message);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, m_current_task.task_message.content.c_str(), m_current_task.task_message.content.size(), m_current_task.task_message.fromguid.c_str());
	m_current_task.realtime_image_cache_info.face_list.clear();
	m_current_task.realtime_image_cache_info.status = false;
}

void robot_face_process::package_faceimage(FaceDetectData &image)
{
	FaceRect minface;
	minface.width = 10;
	minface.height = 10;

	if (image.face_img.cols > CANMERA_WIDTH || image.face_img.rows > CANMERA_WIDTH)
	{
		cv::Mat temp_scalar_frame;

		cv::Size  des_size;

		double scalar = (double)image.face_img.cols / image.face_img.rows;
		double rate = 0;


		if (image.face_img.cols > image.face_img.rows)
		{
			rate = (double)CANMERA_WIDTH / image.face_img.cols;
			des_size.width = CANMERA_WIDTH;
			des_size.height = (int)(rate*image.face_img.rows);
		}
		else
		{
			des_size.height = CANMERA_WIDTH;
			rate = (double)CANMERA_WIDTH / image.face_img.rows;
			des_size.width = (int)(rate*image.face_img.cols);
		}

		cv::resize(image.face_img, temp_scalar_frame, des_size, 0, 0, CV_INTER_LINEAR);

		temp_scalar_frame.copyTo(image.face_img);
	}


	image.cameraid = m_face_no;
	image.detect_time = common_get_cur_time_stampms();
	if (m_detectlist.detect->DetectFace(image.face_img, minface, image.face_list, "image"))
		image.status = true;
}
