#pragma once
#include "robot_controller_interface.h"


class robot_controller_slamware;

class slam_thread :public Thread_helper
{
public:
	slam_thread(robot_controller_slamware *p){ parent = p; 	m_ml = common_mutex_create_threadlock(); result = -1; cancel = false; finished = true; waittype = -1; }
	virtual ~slam_thread(){ if (state())quit(); common_mutex_threadlock_release(m_ml); }

	bool isFinish();
	void CancelMessage();
	int getWaitType();
	int getResult();

private:
	virtual void run();
	robot_controller_slamware *parent;
	CHANDLE m_ml;
	bool cancel;
	bool finished;
	int waittype;
	int result;
	robot_json_rsp		m_json;
};

class slamrobotstate_thread :public Thread_helper
{
public:
	slamrobotstate_thread(robot_controller_slamware *p){ parent = p; }
	virtual ~slamrobotstate_thread(){ if (state())quit(); }

private:
	virtual void run();
	robot_controller_slamware *parent;
	robot_json_rsp		m_json;
};

class slamrobotobjdetect_thread :public Thread_helper
{
public:
	slamrobotobjdetect_thread(robot_controller_slamware *p){ parent = p; }
	virtual ~slamrobotobjdetect_thread(){ if (state())quit(); }

private:
	virtual void run();
	robot_controller_slamware *parent;
	robot_json_rsp		m_json;
};


class robot_controller_slamware :
	public robot_controller_interface
{
public:
	robot_controller_slamware();
	virtual ~robot_controller_slamware();
	std::vector<std::string> get_map_file(std::string path);
	virtual int  init();
	virtual int check_init_finish();
	virtual bool is_navi();
	virtual bool is_standby();
	virtual void release();
	virtual int  updata_ultrasonic_data();
	virtual int get_laser_data(std::vector<grid_point_data>&data, int &maxsu);

	virtual int get_device_status(device_status &);
	
	virtual int get_current_point(grid_point_data &);
	virtual int get_twopoint_navigate_path(std::string mapname, grid_point_data beginpoint, grid_point_data endpoint, navigation_path_data &data);

	virtual int get_navigate_path(navigation_path_data &data);
	virtual int navigate_to(grid_point_data&);
	virtual int navigate_to(std::string);

	virtual int navigate_pause();
	virtual int navigate_cancel();
	virtual int navigate_resume();


	virtual int get_special_point_list(std::string mapname, std::vector<special_point_data>&, int type = -1);
	virtual int remember_point(std::string mapname, special_point_data&);
	virtual int delete_point(std::string mapname, std::string name);


	virtual int get_map_picture(std::string mapname, std::vector<char> &mapdata);
	virtual int get_map_list(std::vector<map_data>&);

	virtual int get_current_map_name(std::string &mapname);
	virtual int get_current_speed(double &ls, double &as);
	virtual int get_protector(std::vector<int >& frame);

	virtual int load_map(std::string name);
	virtual int initialize_robot(std::string mapname, std::string name);
	virtual int initialize_robot(special_point_data);
	virtual int initialize_cancel_robot();

	virtual std::string get_health_report();

	virtual int scan_map(std::string name);
	virtual int save_map();
	virtual int cancel_scan();
	virtual int get_scan_map(std::vector<char> &mapdata);
	virtual int del_map(std::string name);

	virtual int get_virtual_wall(std::string name, std::string &data);
	virtual int  update_virtual_wall(std::string name, std::string json);

	virtual std::string get_slopes(std::string name);
	virtual int  update_slopes(std::string name, std::string json);


	virtual int move(double lspeed, double aspeed);

	virtual int moveto(double lspeed, double distance);
	virtual int check_moveto_finish();
	virtual int stop_moveto();
	virtual int rotate(double angle, double aspeed);
	virtual int check_rotate_finish();
	virtual int stop_rotate();


	virtual std::string get_version();
	virtual int set_navigation_speed(int level);
	virtual int detect_move_data(std::vector<move_object_detect>& object_detect);
	virtual int start_charge();
	virtual int stop_charge();
	virtual int get_charge_state(int &state);
	virtual int shutdown();
	virtual int soft_reset_reboot() ;
	virtual int resume_motor(int type);
	virtual int motor(bool type);

	void LoadPoint();
	void SavePoint();
	int get_device_status(device_status &ds, bool &isconnect);
	std::string scanmapname;
	std::string currentmap;
	slam_thread m_waitthread;
	slamrobotstate_thread m_notifythread;
	slamrobotobjdetect_thread m_objdetect;
	bool isconect;
	bool isinit;
	std::vector<special_point_data > point_list;
	CHANDLE m_ml;
	CHANDLE m_laser;
};

