#pragma  once
#include <string>
#include <vector>

extern const char *MODULE_NAME;

#define ULTRASONIC_COUNT 7
#define BOOLTOINT(x) x?0:1




struct world_point_data
{
	double x;
	double y;
	double w;
	double z;
	world_point_data()
	{
		x = 0; y = 0; w = 0; z = 0;
	}
};

struct grid_point_data
{
	int x;
	int y;
	double angle;
	world_point_data world;
	grid_point_data()
	{
		x = 0;
		y = 0; angle = 0;
	}
};

struct ultrasonic_data
{
	double min_range;
	double range;
	grid_point_data origin;
	grid_point_data object_postion;
};

struct move_object_detect
{
	double distance;
	double object_angle;
	double object_speed;
	double possibility;
	int    is_human;
	grid_point_data object_position;
	/*	std::vector<ultrasonic_data> ultr_data;*/
	move_object_detect()
	{
		distance = 0;
		possibility = 0;
		object_angle = 0;
		object_speed = 0;
		is_human = 0;
	}
};

struct controller_status
{
	double speed;
	double battery;
	bool   isStop;
	bool charge;
};

struct special_point_data
{
	int type=0;
	std::string name;
	grid_point_data postion;
};
struct device_status
{
	double speed = 0;
	bool emergencyStop = false;
	int battery = 0;
	bool charge = false;
	int  charger=0;
	int speedlevel=0;
	int startuptime=0;
	bool navigation_status = false;
};


struct map_data
{
	std::string map_name;
	int   map_w=0;
	int   map_h=0;
	double originX = 0;
	double originY = 0;
	double rate = 0;
	std::vector<special_point_data> pos_list;
};

struct navigation_path_data
{
	map_data  map_info;
	std::vector<grid_point_data> path_pos_list;
};

struct move_message
{
	double as=0.0;
	double ls=0.0;
	unsigned long long tims=0;
	std::string guid;
};


enum ULTRASONIC_TYPE
{
	ULTR_BOTTOM,
	ULTR_FRONT,
	ULTR_BACK,
};

#define CONTROLL_OK   0
#define CONTROLL_FAILED   1



struct NaviTask
{
	std::string pointname;
	std::string taskname;
	std::string mapname;
	std::string path_name;
	std::string graph_name;
	std::string graph_path_name;
	std::string graph_path_group_name;
	grid_point_data  position;
	int type = 0;
};

struct TaskQueue
{
	std::string mapname;
	std::string queuename;
	bool		loop = false;
	int			loop_count = 0;
	std::vector<NaviTask > queueinfo;
};


struct  NavigationMessageDetail
{
	std::string        mapname;
	std::string        name;
	std::string		   greeter_name;
	int				   x;
	int				   y;
	int				   r;
	int				   state;
	double             ls;
	double             as;
	double			   rs;
	bool			   fstat;
	int				   type;
	TaskQueue						taskcall;
	std::vector<move_object_detect>  move_data;
	std::vector<special_point_data>  point_list;
	std::vector<map_data>		     map_dat;
	std::vector<int>				protector;;
	device_status     ds;
	NavigationMessageDetail()
	{
		x = 0;
		y = 0;
		r = 0;
		state = 0;
		ls = 0;
		fstat = false;
		as = 0;
		type = -1;
		rs = 0;
	}


};
enum MESSAGE_TASK_NAME
{
	ROBOT_UNKNOW = -1,
	NAVIGATE_TO_POSTION_REQUEST,
	NAVIGATE_TO_CANCEL_REQUEST,
	NAVIGATE_GET_CURRENT_POSTION_REQUEST,
	NAVIGATE_INITIALIZE_REQUEST,
	NAVIGATE_MOVE_REQUEST,
	NAVIGATE_ROTATE_REQUEST,
	NAVIGATE_DEVICE_STATUS_REQUEST,
	NAVIGATE_START_MOVEDATA_REQUEST,
	NAVIGATE_STOP_MOVEDATA_REQUEST,
	NAVIGATE_GET_MAP_REQUEST,
	NAVIGATE_MAP_LIST_REQUEST,
	NAVIGATE_APPEND_CHARGEPOINT_REQUEST,
	NAVIGATE_TO_CHARGE_REQUEST,
	NAVIGATE_GET_CHARGE_LIST_REQUEST,
	NAVIGATE_DEL_CHARGEPOINT_REQUEST,
	NAVIGATE_START_CHARGE_REQUEST,
	NAVIGATE_STOP_CHARGE_REQUEST,
	NAVIGATE_GET_STATE_CHARGE_REQUEST,
	NAVIGATE_SHUTDOWN_REQUEST,
	NAVIGATE_CHANGE_SPEED_REQUEST,
	NAVIGATE_GET_PATH_REQUEST,
	NAVIGATE_START_DANCE,
	NAVIGATE_STOP_DANCE,
	NAVIGATE_GREETER_REQUEST,
	NAVIGATE_HEALTH_REQUEST,
	NAVIGATE_RESUME_MOTOR_REQUEST,
	NAVIGATE_MOVETO_REQUEST,
	NAVIGATE_STOPMOVETO_REQUEST,
	NAVIGATE_STOPROTATE_REQUEST,
	NAVIGATE_MOTOR_REQUEST,
	NAVIGATE_GET_SPEED_REQUEST,
	NAVIGATE_START_SPEED_REQUEST,
	NAVIGATE_STOP_SPEED_REQUEST,
	NAVIGATE_SCAN_MAP_REQUEST,
	NAVIGATE_SCAN_SAVEMAP_REQUEST,
	NAVIGATE_SCAN_CANCEL_REQUEST,
	NAVIGATE_SCAN_GET_MAP_REQUEST,
	NAVIGATE_DEL_MAP_REQUEST,
	NAVIGATE_GET_VIRTUAL_WALL_REQUEST,
	NAVIGATE_SET_VIRTUAL_WALL_REQUEST,
	NAVIGATE_GET_PROTECT_REQUEST,
	NAVIGATE_APPEND_SPECIALPOINT_REQUEST,
	NAVIGATE_TO_SPECIALPOINT_REQUEST,
	NAVIGATE_GET_SPECIALPOINT_LIST_REQUEST,
	NAVIGATE_DEL_SPECIALPOINT_REQUEST,
	NAVIGATE_START_MASE_REQUEST,
	NAVIGATE_STOP_MASE_REQUEST,
	NAVIGATE_GET_GRAPHPATH_LIST_REQUEST,
	NAVIGATE_GET_TASK_STATUS_REQUEST,
	NAVIGATE_APPEND_TASKQUEUE_REQUEST,
	NAVIGATE_GET_TASKQUEUE_REQUEST,
	NAVIGATE_DEL_TASKQUEUE_REQUEST,
	NAVIGATE_START_TASKQUEUE_REQUEST,
	NAVIGATE_STOP_ALLTASKQUEUE_REQUEST,
	NAVIGATE_PAUSE_TASKQUEUE_REQUEST,
	NAVIGATE_RESUME_TASKQUEUE_REQUEST,
	NAVIGATE_STOP_CURRENTTASK_REQUEST,
	NAVIGATE_IS_TASKQUEUE_FINISH_REQUEST,
	NAVIGATE_MOVE_CHECK_RANGE_REQUEST,
	NAVIGATE_UPDATE_MOVE_WALL_REQUEST,
	NAVIGATE_GET_LASER_RAW_REQUEST,
	NAVIGATE_CANCEL_INITIALIZE_REQUEST,
	NAVIGATE_GET_TASK_LIST_REQUEST,
	NAVIGATE_RESET_ROBOT_REQUEST,
	ROBOT_CONTROLLRT_END
};


struct  FootCtrlMessage
{
	int appid;
	MESSAGE_TASK_NAME messagetype;
	std::string fromguid;
	std::string response;
	std::string content;
	int error;
	NavigationMessageDetail detail;
	FootCtrlMessage()
	{
		messagetype = ROBOT_UNKNOW;
		appid = 0;
		error = 0;
	}
};

class robot_controller_interface;

struct gaussian_websocket_info
{
	struct lws *wsi;
	std::string url;
	bool   success;
	bool   returnresult;
	robot_controller_interface *cinterface;
	char buffer[1024];
	char path[1024];
	gaussian_websocket_info()
	{
		wsi = NULL; success = false; returnresult = false; cinterface = NULL;
	}

};