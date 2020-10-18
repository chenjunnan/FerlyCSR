#pragma once
#include <string>
#include "json/json.h"
#include "robot_controller_struct.h"

class robot_json_rsp
{
public:
	robot_json_rsp(){};
	~robot_json_rsp(){};

	std::string robot_json_rsp::navigate_to(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_TO_POSTION_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_cancel(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_TO_CANCEL_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_curpostion(FootCtrlMessage *data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_GET_CURRENT_POSTION_RESPONSE");
		json_object["error"] = Json::Value(data->error);
		if (!data->error&&data->detail.point_list.size()==1)
		{
			if (!data->detail.mapname.empty())json_object["map"] = Json::Value(data->detail.mapname);
			json_object["x"] = Json::Value(data->detail.point_list[0].postion.x);
			json_object["y"] = Json::Value(data->detail.point_list[0].postion.y);
			json_object["r"] = Json::Value((int)data->detail.point_list[0].postion.angle);
			json_object["worldx"] = Json::Value(data->detail.point_list[0].postion.world.x);
			json_object["worldy"] = Json::Value(data->detail.point_list[0].postion.world.y);
			json_object["worldr"] = Json::Value(data->detail.point_list[0].postion.angle);
		}

		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_state_ntf(FootCtrlMessage *data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_CURRENT_STATE_NOTIFY");

		json_object["navigate_state"] = Json::Value(data->detail.state);

		if (data->detail.x != 0 && data->detail.y != 0)
		{
			json_object["x"] = Json::Value(data->detail.x);
			json_object["y"] = Json::Value(data->detail.y);
			json_object["r"] = Json::Value(data->detail.r);
		}

		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_neterror_ntf()
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_NET_ERROR_NOTIFY");
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_init(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_INITIALIZE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_move(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_MOVE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_move_check(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_MOVE_CHECK_RANGE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_move_check_update(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_UPDATE_MOVE_WALL_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_rotate(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_ROTATE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::navigate_device_status(int errorcode, device_status ds)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_DEVICE_STATUS_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		if (!errorcode)
		{
			json_object["speed"] = Json::Value(ds.speed);
			json_object["emergencyStop"] = Json::Value(ds.emergencyStop);
			json_object["battery"] = Json::Value(ds.battery);
			json_object["charge"] = Json::Value(ds.charge);
			json_object["charge_type"] = Json::Value(ds.charger);
			json_object["speedlevel"] = Json::Value(ds.speedlevel);
			json_object["startuptime"] = Json::Value(ds.startuptime);
			json_object["navigation_status"] = Json::Value(ds.navigation_status);
		}
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::navigate_movedata_start(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_START_MOVEDATA_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::navigate_movedata_stop(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_STOP_MOVEDATA_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::low_battery_ntf(int errorcode, int value, bool charge, bool stop,int charge_type)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_BATTERY_STATE_NOTIFY");
		json_object["error"] = Json::Value(errorcode);
		if (!errorcode)
		{
			json_object["battery"] = Json::Value(value);
			json_object["charge"] = Json::Value(charge);
			json_object["charge_type"] = Json::Value(charge_type);
			json_object["emergencyStop"] = Json::Value(stop);
		}

		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_maplist(FootCtrlMessage *data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_MAP_LIST_RESPONSE");
		json_object["error"] = Json::Value(data->error);
		if (data->error == 0)
		{
			for (unsigned int i = 0; i < data->detail.map_dat.size(); i++)
			{
				Json::Value   map_dat;
				map_dat["name"] = Json::Value(data->detail.map_dat[i].map_name);
				map_dat["width"] = Json::Value(data->detail.map_dat[i].map_w);
				map_dat["height"] = Json::Value(data->detail.map_dat[i].map_h);
				map_dat["originX"] = Json::Value(data->detail.map_dat[i].originX);
				map_dat["originY"] = Json::Value(data->detail.map_dat[i].originY);
				map_dat["rate"] = Json::Value(data->detail.map_dat[i].rate);
				json_object["maplist"].append(map_dat);
			}
		}
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_change_speed(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_CHANGE_SPEED_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_getmap(FootCtrlMessage *data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_GET_MAP_RESPONSE");
		json_object["error"] = Json::Value(data->error);
		if (!data->detail.name.empty())
		{
			if (data->detail.state==0)
				json_object["path"] = Json::Value(data->detail.name);
			else
			{
				json_object["filetype"] = Json::Value(data->detail.greeter_name);
				json_object["data"] = Json::Value(data->detail.name);
			}
		}

		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_append_chargepoint(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_APPEND_CHARGEPOINT_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_to_charge(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_TO_CHARGE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_charge_point_list(FootCtrlMessage *data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_GET_CHARGE_LIST_RESPONSE");
		json_object["error"] = Json::Value(data->error);
		for (unsigned int i = 0; i < data->detail.point_list.size(); i++)
		{
			Json::Value   one;
			one["name"] = Json::Value(data->detail.point_list[i].name);
			one["x"] = Json::Value(data->detail.point_list[i].postion.x);
			one["y"] = Json::Value(data->detail.point_list[i].postion.y);
			one["angle"] = Json::Value((int)data->detail.point_list[i].postion.angle);
			json_object["charge_point"].append(one);
		}
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_charge_state(FootCtrlMessage *data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_GET_STATE_CHARGE_RESPONSE");
		json_object["error"] = Json::Value(data->error);
		if (data->error == 0)
			json_object["charge_state"] = Json::Value(data->detail.state);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_del_charge_point(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_DEL_CHARGEPOINT_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_start_charge(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_START_CHARGE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_stop_charge(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_STOP_CHARGE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_shutdown(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_SHUTDOWN_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_reset(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_RESET_ROBOT_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_detectmove_ntf(FootCtrlMessage *data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_MOVEDATA_STATE_NOTIFY");
		json_object["error"] = Json::Value(data->error);
		for (unsigned int i = 0; i < data->detail.move_data.size(); i++)
		{
			Json::Value  json_face;
			json_face["distance"] = Json::Value(data->detail.move_data[i].distance);
			json_face["angle"] = Json::Value(data->detail.move_data[i].object_angle);
			json_face["x"] = Json::Value(data->detail.move_data[i].object_position.x);
			json_face["y"] = Json::Value(data->detail.move_data[i].object_position.y);
			json_object["move_list"].append(json_face);
		}
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_mappathlist(FootCtrlMessage *data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_GET_PATH_RESPONSE");
		json_object["error"] = Json::Value(data->error);
		if (data->error == 0)
		{
			for (unsigned int i = 0; i < data->detail.point_list.size(); i++)
			{
				Json::Value   point_json;
				point_json["x"] = Json::Value(data->detail.point_list[i].postion.x);
				point_json["y"] = Json::Value(data->detail.point_list[i].postion.y);
				json_object["path"].append(point_json);
			}
			if (data->detail.point_list.size()>0 && data->detail.map_dat.size() > 0)
			{
				json_object["width"] = Json::Value(data->detail.map_dat[0].map_w);
				json_object["height"] = Json::Value(data->detail.map_dat[0].map_h);
				json_object["originX"] = Json::Value(data->detail.map_dat[0].originX);
				json_object["originY"] = Json::Value(data->detail.map_dat[0].originY);
				json_object["rate"] = Json::Value(data->detail.map_dat[0].rate);
			}
		}
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_greeter(FootCtrlMessage *data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_GREETER_RESPONSE");
		json_object["error"] = Json::Value(data->error);
		json_object["task_type"] = Json::Value(data->detail.name);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_system_state_ntf(int state,std::string msg)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_COMMAND_STATE_NOTIFY");
		json_object["cmd_state"] = Json::Value(state);
		json_object["cmd_msg"] = Json::Value(msg);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_system_state_ntf(int state, std::string msg, Json::Value &jsonobject)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_COMMAND_STATE_NOTIFY");
		json_object["cmd_state"] = Json::Value(state);
		json_object["cmd_msg"] = Json::Value(msg);
		json_object["cmd_data"] = jsonobject;
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_rotate_finish_ntf(int error,std::string msg)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_ROTATE_FINISHED_NOTIFY");
		json_object["error"] = Json::Value(error);
		json_object["msg"] = Json::Value(msg.c_str());
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_moveto_finish_ntf(int error, std::string msg)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_MOVETO_FINISHED_NOTIFY");
		json_object["error"] = Json::Value(error);
		json_object["msg"] = Json::Value(msg.c_str());
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::navigate_moveto(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_MOVETO_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_stop_moveto(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_STOPMOVETO_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_stop_rotate(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_STOPROTATE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_resume_motor(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_RESUME_MOTOR_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_motor(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_MOTOR_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_speed(int errorcode, double as, double ls)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_GET_SPEED_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		if (errorcode == 0)
		{
			json_object["as"] = Json::Value(as);
			json_object["ls"] = Json::Value(ls);

		}
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_scan(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_SCAN_MAP_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_save_map(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_SCAN_SAVEMAP_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_cancel_scanmap(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_SCAN_CANCEL_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_get_scanmap(FootCtrlMessage *data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_SCAN_GET_MAP_RESPONSE");
		json_object["error"] = Json::Value(data->error);
		if (!data->detail.name.empty())
		{
			if (data->detail.state == 0)
				json_object["path"] = Json::Value(data->detail.name);
			else
			{
				json_object["filetype"] = Json::Value(data->detail.greeter_name);
				json_object["data"] = Json::Value(data->detail.name);
			}
		}
		return json_writer.write(json_object);
	}


	std::string robot_json_rsp::navigate_del_map(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_DEL_MAP_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_get_vwall(int errorcode,std::string data)
	{
		Json::Value   json_object,json_data;
		Json::FastWriter json_writer;
		Json::Reader  json_read;
		if (!data.empty())
		{
			if (json_read.parse(data, json_data))
				json_object["virtual_wall"] = Json::Value(json_data);
		}

		json_object["msg_type"] = Json::Value("NAVIGATE_GET_VIRTUAL_WALL_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_set_vwall(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_SET_VIRTUAL_WALL_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_get_protector(int errorcode,std::vector<int> protect_list)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_GET_PROTECT_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		for (unsigned int i = 0; i < protect_list.size();i++)
			json_object["protector"].append(Json::Value(protect_list[i]==0x30?false:true));
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_append_specialpoint(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_APPEND_SPECIALPOINT_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_to_special(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_TO_SPECIALPOINT_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_special_point_list(FootCtrlMessage *data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_GET_SPECIALPOINT_LIST_RESPONSE");
		json_object["error"] = Json::Value(data->error);
		for (unsigned int i = 0; i < data->detail.point_list.size(); i++)
		{
			Json::Value   one;
			one["type"] = Json::Value(data->detail.point_list[i].type);
			one["name"] = Json::Value(data->detail.point_list[i].name);
			one["x"] = Json::Value(data->detail.point_list[i].postion.x);
			one["y"] = Json::Value(data->detail.point_list[i].postion.y);
			one["angle"] = Json::Value((int)data->detail.point_list[i].postion.angle);
			json_object["charge_point"].append(one);
		}
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_del_special_point(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_DEL_SPECIALPOINT_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::startmase(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_START_MASE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::stopmase(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_STOP_MASE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::memsnotify(int errorcode, const char * con=NULL)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_MASE_NOTIFY");
		json_object["error"] = Json::Value(errorcode);
		if (con)json_object["errormsg"] = Json::Value(con);
		return json_writer.write(json_object);
	}


	std::string robot_json_rsp::SimpleResponse(std::string reponse,int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value(reponse);
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::istaskqueuefinsih(bool b, int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_IS_TASKQUEUE_FINISH_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		if (errorcode==0)
			json_object["finished"] = Json::Value(b);
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::navigate_laser_raw(std::vector<grid_point_data>&move_data,int err)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_GET_LASER_RAW_RESPONSE");
		json_object["error"] = Json::Value(err);
		for (unsigned int i = 0; i < move_data.size(); i++)
		{
			Json::Value  json_face;
			json_face["x"] = Json::Value(move_data[i].x);
			json_face["y"] = Json::Value(move_data[i].y);
			json_object["raw"].append(json_face);
		}
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::navigate_init_cancel(int err)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("NAVIGATE_CANCEL_INITIALIZE_RESPONSE");
		json_object["error"] = Json::Value(err);
		return json_writer.write(json_object);
	}
};