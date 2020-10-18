#pragma once
#include "robot_controller_interface.h"
class robot_controller_ferly :
	public robot_controller_interface
{
public:
	robot_controller_ferly();
	virtual ~robot_controller_ferly();
	virtual int  init();
	virtual int check_init_finish();
	virtual bool is_navi();
	virtual bool is_standby();
	virtual void release();
	virtual int  updata_ultrasonic_data();
	virtual int get_laser_data(std::vector<grid_point_data>&, int &);

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
	virtual int soft_reset_reboot();
	virtual int resume_motor(int type);
	virtual int motor(bool type);
};

