#include "robot_controller_ferly.h"


robot_controller_ferly::robot_controller_ferly()
{
}


robot_controller_ferly::~robot_controller_ferly()
{
}

int robot_controller_ferly::init()
{
	return 0;
}

int robot_controller_ferly::check_init_finish()
{
	return 0;
}

bool robot_controller_ferly::is_navi()
{
	return 0;
}

bool robot_controller_ferly::is_standby()
{
	return 0;
}

void robot_controller_ferly::release()
{

}

int robot_controller_ferly::updata_ultrasonic_data()
{
	return 0;
}

int robot_controller_ferly::get_laser_data(std::vector<grid_point_data>&, int &)
{
	return 0;
}

int robot_controller_ferly::get_device_status(device_status &)
{
	return 0;
}

int robot_controller_ferly::get_current_point(grid_point_data &)
{
	return 0;
}

int robot_controller_ferly::get_twopoint_navigate_path(std::string mapname, grid_point_data beginpoint, grid_point_data endpoint, navigation_path_data &data)
{
	return 0;
}

int robot_controller_ferly::get_navigate_path(navigation_path_data &data)
{
	return 0;
}

int robot_controller_ferly::navigate_to(grid_point_data&)
{
	return 0;
}

int robot_controller_ferly::navigate_to(std::string)
{
	return 0;
}

int robot_controller_ferly::navigate_pause()
{
	return 0;
}

int robot_controller_ferly::navigate_cancel()
{
	return 0;
}

int robot_controller_ferly::navigate_resume()
{
	return 0;
}

int robot_controller_ferly::get_special_point_list(std::string mapname, std::vector<special_point_data>&, int type /*= -1*/)
{
	return 0;
}

int robot_controller_ferly::remember_point(std::string mapname, special_point_data&)
{
	return 0;
}

int robot_controller_ferly::delete_point(std::string mapname, std::string name)
{
	return 0;
}

int robot_controller_ferly::get_map_picture(std::string mapname, std::vector<char> &mapdata)
{
	return 0;
}

int robot_controller_ferly::get_map_list(std::vector<map_data>&)
{
	return 0;
}

int robot_controller_ferly::get_current_map_name(std::string &mapname)
{
	return 0;
}

int robot_controller_ferly::get_current_speed(double &ls, double &as)
{
	return 0;
}

int robot_controller_ferly::get_protector(std::vector<int >& frame)
{
	return 0;
}

int robot_controller_ferly::load_map(std::string name)
{
	return 0;
}

int robot_controller_ferly::initialize_robot(std::string mapname, std::string name)
{
	return 0;
}

int robot_controller_ferly::initialize_robot(special_point_data)
{
	return 0;
}

int robot_controller_ferly::initialize_cancel_robot()
{
	return 0;
}

std::string robot_controller_ferly::get_health_report()
{
	return "";
}

int robot_controller_ferly::scan_map(std::string name)
{
	return 0;
}

int robot_controller_ferly::save_map()
{
	return 0;
}

int robot_controller_ferly::cancel_scan()
{
	return 0;

}

int robot_controller_ferly::get_scan_map(std::vector<char> &mapdata)
{
	return 0;
}

int robot_controller_ferly::del_map(std::string name)
{
	return 0;
}



int robot_controller_ferly::get_virtual_wall(std::string name, std::string &data)
{
	return 0;
}

int robot_controller_ferly::update_virtual_wall(std::string name, std::string json)
{
	return 0;
}

std::string robot_controller_ferly::get_slopes(std::string name)
{
	return "";
}

int robot_controller_ferly::update_slopes(std::string name, std::string json)
{
	return 0;
}

int robot_controller_ferly::move(double lspeed, double aspeed)
{
	return 0;
}

int robot_controller_ferly::moveto(double lspeed, double distance)
{
	return 0;
}

int robot_controller_ferly::check_moveto_finish()
{
	return 0;
}

int robot_controller_ferly::stop_moveto()
{
	return 0;
}

int robot_controller_ferly::rotate(double angle, double aspeed)
{
	return 0;
}

int robot_controller_ferly::check_rotate_finish()
{
	return 0;
}

int robot_controller_ferly::stop_rotate()
{
	return 0;
}

std::string robot_controller_ferly::get_version()
{
	return "";
}

int robot_controller_ferly::set_navigation_speed(int level)
{
	return 0;
}

int robot_controller_ferly::detect_move_data(std::vector<move_object_detect>& object_detect)
{
	return 0;
}

int robot_controller_ferly::start_charge()
{
	return 0;
}

int robot_controller_ferly::stop_charge()
{
	return 0;
}

int robot_controller_ferly::get_charge_state(int &state)
{
	return 0;
}

int robot_controller_ferly::shutdown()
{
	return 0;

}

int robot_controller_ferly::soft_reset_reboot()
{
	return 0;
}
int robot_controller_ferly::resume_motor(int type)
{
	return 0;
}

int robot_controller_ferly::motor(bool type)
{
	return 0;
}
