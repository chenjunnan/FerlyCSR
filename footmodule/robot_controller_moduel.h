#pragma once
#include "common_helper.h"
#include "robot_controller_interface.h"
#include "robot_controller_struct.h"
#include "opencv2\core\core.hpp"
#include "MagneticTrack_ctrol.h"


class dance_thread :public single_handle_thread_base
{
public:
	dance_thread(){ }
	virtual ~dance_thread(){ if (state())quit(); }
private:
	virtual void run();
	void updata_stop_postion(std::vector<grid_point_data> &stop_pos, std::vector<grid_point_data> &laser_list, grid_point_data &current_point);
};

class movedetect_thread :public single_handle_thread_base
{
public:
	movedetect_thread(){  }
	virtual ~movedetect_thread(){ if (state())quit(); }
private:
	virtual void run();
	void move_data_handler();
	double angle_sub(const int x1, const int y1, const int x2, const int y2);
	void get_near_laser_data(std::vector<move_object_detect> &move_object);
	std::vector<move_object_detect> m_last_detectobject;

};

class Slammovedetect_thread :public single_handle_thread_base
{
public:
	Slammovedetect_thread(){  }
	virtual ~Slammovedetect_thread(){ if (state())quit(); }
private:
	virtual void run();
	void move_data_handler();
};
class detectspeed_thread :public single_handle_thread_base
{
public:
	detectspeed_thread(){ m_controler = NULL; }
	std::string client_name;
	virtual ~detectspeed_thread(){ if (state())quit(); }
private:
	virtual void run();

};

class move_ctrol_thread :public single_handle_thread_base
{
public:
	move_ctrol_thread(){ m_controler = NULL; }
	virtual ~move_ctrol_thread(){ if (state())quit(); }
private:
	virtual void run();
};

class move_check_thread :public single_handle_thread_base
{
public:
	move_check_thread();
	int updateVirualMap();
	virtual ~move_check_thread();
private:
	virtual void run();
	cv::Mat      _datamap;
	bool		m_finish_update;
};


class wait_interfce
{
public:
	wait_interfce(robot_controller_interface *p){ m_p = p; }
	virtual ~wait_interfce(){}
	virtual std::string onSuccess() = 0;
	virtual std::string onFailed() = 0;
	virtual std::string onTimeout(){ return ""; }
	virtual bool check() = 0;
	int waittime = -1;
	int sleeptime = 0;
	robot_controller_interface *m_p;
	robot_json_rsp m_rsp;
};

class wait_rotate :public wait_interfce
{
public:
	wait_rotate(robot_controller_interface *p) :wait_interfce(p){ waittime = 120000; sleeptime = 200; }
	virtual ~wait_rotate(){ }
	virtual std::string onSuccess()
	{
		return  m_rsp.navigate_rotate_finish_ntf(0, "success");
	}
	virtual std::string onFailed()
	{
		LOGW("wait failed for  rotate");
		return  m_rsp.navigate_rotate_finish_ntf(2, "failed");	
	}
	virtual std::string onTimeout()
	{
		LOGW("wait timeout for  rotate");
		return m_rsp.navigate_rotate_finish_ntf(1, "timeout");
	}
	virtual bool check()
	{
		return m_p->check_rotate_finish() ? false : true;
	}
};

class wait_moveto :public wait_interfce
{
public:
	wait_moveto(robot_controller_interface *p) :wait_interfce(p){ sleeptime = 200; }
	virtual ~wait_moveto(){ }
	virtual std::string onSuccess()
	{
		return  m_rsp.navigate_moveto_finish_ntf(0, "success");
	}
	virtual std::string onFailed()
	{
		LOGW("wait failed for  moveto");
		return  m_rsp.navigate_moveto_finish_ntf(2, "failed");
	}
	virtual std::string onTimeout()
	{
		LOGW("wait timeout for  moveto");
		return m_rsp.navigate_moveto_finish_ntf(1, "timeout");
	}
	virtual bool check()
	{
		return m_p->check_moveto_finish() ? false : true;
	}
};


class wait_thread :public Thread_helper
{
public:
	wait_thread(){ m_runfun = NULL; }
	void setFromer(std::string name){ clinet = name; }
	void setWaitHandle(wait_interfce *wi){
		if (!wi)return;
		if (m_runfun)release(); m_runfun = wi;
	}
	virtual ~wait_thread(){ if (state())quit(); release(); }
private:
	void release(){ if (m_runfun) delete m_runfun; m_runfun = NULL; };
	virtual void run();
	std::string clinet;
	wait_interfce *m_runfun;
};

#define  MAX_WAITTHREAD  2
#define  ROTATE_THREAD_ID  0
#define  MOVETO_THREAD_ID  1

class robot_controller_moduel :
	public Thread_helper
{
public:
	robot_controller_moduel();
	virtual ~robot_controller_moduel();
	int init(int id, int type);
	bool message_handler(FootCtrlMessage &rcs);
	void stop();

private:
	void get_path(FootCtrlMessage &rcs);
	void get_map(FootCtrlMessage &rcs);
	void task_handler(FootCtrlMessage *rbsm);
	int  encode_map(int mode, std::vector<char >&imgdata, std::string &dataorname);
	virtual void run();
	robot_controller_interface  *m_rci;
	dance_thread   m_dance_thread;
	bool  m_init_ok;
	robot_json_rsp		m_json;
	movedetect_thread  m_detect_moveobject;
	Slammovedetect_thread m_detect_slam_moveobject;
	detectspeed_thread m_speed_detect;
	move_ctrol_thread  m_move_ctrl;
	move_check_thread  m_checkmove_ctrl;
	MagneticTrack_ctrol_thread m_mtc;
	int				control_id;
	int				m_type;
	CHANDLE			m_ml;
	wait_thread 	m_wait_th[MAX_WAITTHREAD];
};

