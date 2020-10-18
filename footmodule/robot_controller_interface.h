#pragma once
#include "common_helper.h"
#include "robot_controller_struct.h"
#include "robot_json_rsp.h"

class robot_controller_interface
{
public:
	robot_controller_interface(){ m_ultrasonic.resize(3);};

	virtual int init()=0;
	virtual int check_init_finish()=0;
	virtual bool is_navi()=0;
	virtual bool is_standby() = 0;
	virtual void release() = 0;

	virtual int get_device_status(device_status &) = 0;
	virtual int updata_ultrasonic_data() = 0;
	virtual int get_laser_data(std::vector<grid_point_data>&,int&) = 0;
	
	virtual int get_current_point(grid_point_data &)=0;
	virtual int get_twopoint_navigate_path(std::string mapname,grid_point_data beginpoint, grid_point_data endpoint,navigation_path_data &data) = 0;

	virtual int get_navigate_path(navigation_path_data &data)=0;
	virtual int navigate_to(grid_point_data&) = 0;
	virtual int navigate_to(std::string) = 0;
	virtual int navigate_pause() = 0;
	virtual int navigate_cancel() = 0;
	virtual int navigate_resume() = 0;

	virtual int get_special_point_list(std::string mapname,std::vector<special_point_data>&,int type=-1) = 0;
	virtual int remember_point(std::string mapname, special_point_data&) = 0;
	virtual int delete_point(std::string mapname, std::string name) = 0;

	virtual int get_current_map_name(std::string &mapname) = 0;
	virtual int get_current_speed(double &ls,double &as) = 0;
	virtual int get_protector(std::vector<int >& frame) = 0;
	virtual int get_map_picture(std::string mapname, std::vector<char> &mapdata) = 0;
	virtual int get_map_list(std::vector<map_data>&) = 0;

	virtual int load_map(std::string name) = 0;
	virtual int initialize_robot(std::string mapname, std::string name) = 0;
	virtual int initialize_robot(special_point_data)=0;
	virtual int initialize_cancel_robot() = 0;

	virtual int scan_map(std::string name) = 0;
	virtual int save_map() = 0;
	virtual int cancel_scan() = 0;
	virtual int get_scan_map(std::vector<char> &mapdata) = 0;
	virtual int del_map(std::string name) = 0;
	virtual int get_virtual_wall(std::string name, std::string &data) = 0;
	virtual int  update_virtual_wall(std::string name, std::string json) = 0;

	virtual std::string get_slopes(std::string name) = 0;
	virtual int  update_slopes(std::string name, std::string json) = 0;

	virtual int detect_move_data(std::vector<move_object_detect>& object_detect)=0;
	virtual int move(double lspeed, double aspeed) = 0;

	virtual int moveto(double lspeed, double distance) = 0;
	virtual int check_moveto_finish() = 0;
	virtual int stop_moveto() = 0;

	virtual int rotate(double angle, double aspeed) = 0;
	virtual int check_rotate_finish()=0;
	virtual int stop_rotate() = 0;

	virtual int start_charge()=0;
	virtual int stop_charge()=0;
	virtual int get_charge_state(int &state)=0;
	virtual int shutdown()=0;
	virtual int soft_reset_reboot() = 0;
	virtual std::string get_health_report()=0;

	virtual int resume_motor(int type) = 0;
	virtual int motor(bool type)=0;

	virtual std::string get_version() = 0;
	virtual int set_navigation_speed(int level) = 0;
	virtual ~robot_controller_interface(){  };
	std::vector<std::vector<ultrasonic_data>>  m_ultrasonic;
protected:
	map_data      m_map_data;
	robot_json_rsp		m_json;
};


class single_handle_thread_base :public Thread_helper
{
public:
	single_handle_thread_base(){ m_controler = NULL; }
	virtual ~single_handle_thread_base(){  }
	virtual void set_handler(robot_controller_interface* p){ m_controler = p; }
protected:
	robot_controller_interface *m_controler;
	robot_json_rsp		m_json;
};