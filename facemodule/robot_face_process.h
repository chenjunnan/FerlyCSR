#pragma once
#include "robot_face_struct.h"
#include "common_helper.h"
#include "memorySwaper.h"
#include "robot_json_rsp.h"
#include <set>
#include "robot_FaceDebug_Windows.h"
#include "robot_face_live_interface.h"
#include "robot_face_process_interface.h"

struct FaceLiver
{
	std::string  algname;
	robot_face_live_interface* thread;
	FaceLiver()
	{
		thread = NULL;
	}
};


class robot_face_process :
	public robot_face_process_local_interface
{
public:
	robot_face_process();
	virtual ~robot_face_process();
	virtual bool init(int no);
	virtual void stop();
	virtual void set_debug(bool flag);
	virtual void set_identify(bool b);
	virtual void set_faceinfo(bool b);
	virtual void get_streamname(std::vector< std::string>&);

	virtual int  start_detect_task(FaceMessage &msg);
	virtual int  camera_message(int result, std::string name, FaceMessage &outmsg);

	virtual void get_status(face_status_info &fd);
	virtual void stop_detect();
	virtual void start_wait_face_image(FaceMessage);
	virtual void start_wait_normal_image(FaceMessage);
	virtual void stop_wait_face_image();
	//virtual bool face_register(FaceMessage registerinfo);
	virtual void get_processer_content(face_process_content& content);


private:
	virtual void run();
	int  readly_start();
	void get_stand_frame(unsigned char * buffer, unsigned int data_len, cv::Mat &frame);
	bool can_register(std::vector<FaceRect> &img);
	bool set_detect_alg(std::string);
	bool set_feature_alg(std::string);
	bool set_live_alg(std::string name);
	void check_image_save_path();
	void save_temp_live_image(FaceMessage * taskinfo, cv::Mat &image);
	void save_temp_face_image();
	void face_state_ntf();
	void show_debug_face_windows(std::string windowsname, std::vector<FaceRect> &face_list, cv::Mat &temp_face);
	void face_register(FaceMessage *rcs);
	void package_faceimage(FaceDetectData &image);
	void open_camera();
	void snapshot(std::vector<FaceRect> &live_face_list, std::vector<FaceRect> &track_face_list, cv::Mat &frame);
	void input_identify(std::vector<FaceRect> &live_face_list, std::vector<FaceRect> &track_face_list, cv::Mat &frame);
	void convert_traceface2liveface(FaceDetectData &liveinfo, std::vector<FaceRect> &track_allface_list, std::vector<FaceRect> &live_face_list);
	bool malloc_buffer(std::vector<unsigned char> &buffer);
	void fix_sex(std::vector<FaceRect> &face_list);
	void close_camera();
	void clean_message();
	FaceDetecter  m_detectlist;

	FaceFeaturer  m_featurelist;

	FaceLiver     m_livedetectlist;

	std::map<int, double>   face_arae_map;

	HANDLE       m_pm;
	int         m_resultvis;
	int         m_resultnis;
	int         m_resultcount;

	std::string  m_streams_name;
	std::string  m_livestreams_name;

	int     m_minface_ntf_th;

	bool	m_show_face_debug;
	bool    m_last_show_state;
	std::set<std::string> m_windows_namelist;

	int     m_identify;
	int     m_direction;
	int     m_record;
	
	int     m_live;

	bool	m_wait_face_image;
	time_t  m_waitface_endtime;

	bool    m_face_realtime_identify;
	bool    m_face_realtime_info;

	Timer_helper identify_time_interval;

	Timer_helper detect_time;

	std::string  m_name;

	int       m_face_no;

	robot_json_rsp   m_repjson;

	bool	   m_inntf_status;
	bool	   m_outntf_status;
	bool       m_face_status;
	bool	   m_shapshot;
	bool       m_live_history_state;

	FaceDetectData  m_face_in;
	FaceDetectData  m_face_out;
	std::string m_image_save_path;
	robot_json_rsp json_request;
	FaceTask  m_current_task;
	robot_FaceDebug_Windows m_windowsdbg[2];
	std::map<int, PersonStateCache> m_sex_map;
	FaceMessage m_message_wait;

	int m_error;
	std::string    m_camera_readername;

};

