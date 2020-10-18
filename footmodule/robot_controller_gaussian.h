#pragma once
#include "robot_controller_interface.h"
#include "libwebsockets.h"
#include "lws_config.h"
#include "common_helper.h"

#define  GS_IP  "10.7.5.88"
//#define  GS_IP  "127.0.0.1"
#define  GS_PORT  "8080" //read rfrom sql
#define  GS_WS_PORT  "8089" //read rfrom sql
//#define  GS_WS_PORT  "7681" //read rfrom sql


struct gaussian_buffer_data {
	size_t rx, tx;
	unsigned char buf[LWS_PRE + 40960];
	unsigned int len;
	unsigned int index;
	int final;
	int continuation;
	int binary;
	int connect_state = 0;
	int save_state = 0;
	bool in_stop=false;
	int battery = 0;
	bool charge=true;
	int greeter_state = 0;
};

class robot_controller_gaussian;

class gaussian_thread :public Thread_helper
{
public:
	gaussian_thread(robot_controller_gaussian *p){ m_ws_context = NULL; parent = p; }
	virtual ~gaussian_thread(){ if (state())quit();  }

private:
	struct lws_context_creation_info m_ws_info;
	struct lws_client_connect_info m_ws_client_info[4];
	struct lws_context *m_ws_context;
	virtual void run();
	bool create_one_url(std::string url, gaussian_websocket_info *gwi, lws_client_connect_info* lcci);
	void create_websocket_url();
	bool handle_ws_client_info(char *buffer, char *path, std::string url, lws_client_connect_info *lccinfo);
	robot_controller_gaussian *parent;
};


class robot_controller_gaussian :
	public robot_controller_interface
{
public:
	robot_controller_gaussian();
	virtual int  init() ;
	virtual int check_init_finish();
	virtual bool is_navi();
	virtual bool is_standby();
	virtual void release();
	virtual int  updata_ultrasonic_data();
	virtual int get_laser_data(std::vector<grid_point_data>&,int &) ;

	virtual int get_device_status(device_status &) ;
	virtual int get_current_point(grid_point_data &) ;
	virtual int get_twopoint_navigate_path(std::string mapname, grid_point_data beginpoint, grid_point_data endpoint, navigation_path_data &data);

	virtual int get_navigate_path(navigation_path_data &data) ;
	virtual int navigate_to(grid_point_data&) ;
	virtual int navigate_to(std::string) ;

	virtual int navigate_pause();
	virtual int navigate_cancel() ;
	virtual int navigate_resume() ;

	virtual int get_special_point_list(std::string mapname, std::vector<special_point_data>&, int type = -1);
	virtual int remember_point(std::string mapname, special_point_data&);
	virtual int delete_point(std::string mapname, std::string name);

	virtual int get_map_picture(std::string mapname, std::vector<char> &mapdata) ;
	virtual int get_map_list(std::vector<map_data>&);

	virtual int get_current_map_name(std::string &mapname);
	virtual int get_current_speed(double &ls, double &as);
	virtual int get_protector(std::vector<int >& frame);

	virtual int load_map(std::string name) ;
	virtual int initialize_robot(std::string mapname, std::string name) ;
	virtual int initialize_robot(special_point_data) ;
	virtual int initialize_cancel_robot() ;

	virtual std::string get_health_report() ;

	virtual int scan_map(std::string name) ;
	virtual int save_map();
	virtual int cancel_scan();
	virtual int get_scan_map(std::vector<char> &mapdata);
	virtual int del_map(std::string name);

	virtual int get_virtual_wall(std::string name, std::string &data);
	virtual int  update_virtual_wall(std::string name, std::string json) ;

	virtual std::string get_slopes(std::string name) ;
	virtual int  update_slopes(std::string name, std::string json) ;


	virtual int move(double lspeed, double aspeed) ;
	
	virtual int moveto(double lspeed, double distance) ;
	virtual int check_moveto_finish();
	virtual int stop_moveto();
	virtual int rotate(double angle, double aspeed);
	virtual int check_rotate_finish();
	virtual int stop_rotate();


	virtual std::string get_version() ;
	virtual int set_navigation_speed(int level) ;
	virtual int detect_move_data(std::vector<move_object_detect>& object_detect);
	virtual int start_charge() ;
	virtual int stop_charge() ;
	virtual int get_charge_state(int &state);
	virtual int shutdown() ;
	virtual int soft_reset_reboot();
	virtual int resume_motor(int type);
	virtual int motor(bool type);

	//gaussian private interface
	int start_greeter(std::string mapname,std::string name);
	int stop_greeter();
	int pause_greeter();
	int resume_greeter();

	int		start_queue(TaskQueue &taskQueue);
	int		stop_queue();
	int		pause_queue();
	int		resume_queue();
	int 	isfinish_queue(bool &st);
	int     get_graphlist(std::string mapname, std::string &data);
	int		get_tasklist(std::string mapname, std::string &data);

	virtual ~robot_controller_gaussian();
	friend class gaussian_thread;
private:
	int gs_version();
	gaussian_thread m_ws_th;
	int get_number_version(std::string &version);
	int simple_get(std::string url);
	std::string imp_get(std::string url,int t=10);
	std::string imp_post(std::string json, std::string url,int t=10);
	bool simple_msg(std::string &out, std::string &data_compare);
	int try_initself();
	int wait_init();

	void append_url_head(std::string &url);
	std::string m_ip;
	std::string m_port;
	int inti_count;

};


