#pragma once
#include <string>
#include "json\json.h"
struct headcontrol_message
{
	int              appid;
	int              messagetype;
	int              tasktype;
	std::string      fromguid;
	std::string      content;
	int              return_state;
	int              timeout;
	std::string      ErrorInfo;

	int              opertype;
	std::string      luminance;
	int              anglelevel;
	int              anglevertical;
	int              speed;
	int              DevState;

	headcontrol_message()
	{
		appid = 0;
		messagetype = 0;
		return_state = 0;
		timeout = 60;
		tasktype = -1;
	}
};
class robot_json_rsp
{
public:
	robot_json_rsp(){}
	~robot_json_rsp(){}

	//打开设备
	std::string robot_json_rsp::open_dev(headcontrol_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("OPEN_DEV_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		return json_writer.write(json_object);
	}
	//打开屏幕亮度自动调节
	std::string robot_json_rsp::auto_open(headcontrol_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("AUTOMATIC_BACKLIGHT_OPEN_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		return json_writer.write(json_object);
	}
	//关闭屏幕亮度自动调节
	std::string robot_json_rsp::auto_close(headcontrol_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("AUTOMATIC_BACKLIGHT_CLOSE_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		return json_writer.write(json_object);
	}
	//设置屏幕亮度
	std::string robot_json_rsp::luminance_set(headcontrol_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("SCREEN_LUMINANCE_SET_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		return json_writer.write(json_object);
	}
	//获取当前屏幕亮度
	std::string robot_json_rsp::luminance_get(headcontrol_message &data)
	{
		Json::Value   json_luminanceinfo;
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("SCREEN_LUMINANCE_GET_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		else
		{
			json_luminanceinfo["luminance"] = Json::Value(data.luminance);
			json_object["result"] = Json::Value(json_luminanceinfo);
		}
		return json_writer.write(json_object);
	}

	//头部灯设置
	std::string robot_json_rsp::head_lightSet(headcontrol_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("HEAD_LIGHT_SET_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		return json_writer.write(json_object);
	}
	//头部灯关闭
	std::string robot_json_rsp::head_lightClose(headcontrol_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("HEAD_LIGHT_CLOSE_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		return json_writer.write(json_object);
	}
	//头部转动
	std::string robot_json_rsp::head_rotate(headcontrol_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("HEAD_CONTROL_ROTATE_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		return json_writer.write(json_object);
	}
	//头部旋转设定角度
	std::string robot_json_rsp::head_angle(headcontrol_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("HEAD_CONTROL_SET_ANGLE_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		return json_writer.write(json_object);
	}
	//获取当前角度
	std::string robot_json_rsp::angle_get(headcontrol_message &data)
	{
		Json::Value   json_angleinfo;
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("HEAD_CONTROL_GET_ANGLE_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		else
		{
			json_angleinfo["angle_level"] = Json::Value(data.anglelevel);
			json_angleinfo["angle_vertical"] = Json::Value(data.anglevertical);
			json_object["result"] = Json::Value(json_angleinfo);
		}
		return json_writer.write(json_object);
	}
	//头部复位
	std::string robot_json_rsp::head_init(headcontrol_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("HEAD_CONTROL_INIT_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		return json_writer.write(json_object);
	}
	//设置旋转速度
	std::string robot_json_rsp::head_speed(headcontrol_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("HEAD_CONTROL_SET_SPEED_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		return json_writer.write(json_object);
	}
	//获得旋转速度
	std::string robot_json_rsp::speed_get(headcontrol_message &data)
	{
		Json::Value   json_speedinfo;
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("HEAD_CONTROL_SET_SPEED_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		else
		{
			json_speedinfo["speed"] = Json::Value(data.speed);
			json_object["result"] = Json::Value(json_speedinfo);
		}
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::stateinfo_result(headcontrol_message &data)
	{
		Json::Value   json_stateinfo;
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("HEAD_DEVSTATE_INFO_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.ErrorInfo);

		else
		{
			json_stateinfo["DevState"] = Json::Value(data.DevState);
			json_object["result"] = Json::Value(json_stateinfo);
		}

		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::head_rotate2(int error)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_HEAD_ROTATE_RESPONSE");
		json_object["error"] = Json::Value(error);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::arm_rotate(int error)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_ARM_ROTATE_RESPONSE");
		json_object["error"] = Json::Value(error);
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::head_getrotate(int error, int angle)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_HEAD_GETANGLE_RESPONSE");
		json_object["error"] = Json::Value(error);
		if (error==0)
			json_object["angle"] = Json::Value(angle);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::arm_getrotate(int error, int id, int angle)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_ARM_GETANGLE_RESPONSE");
		json_object["error"] = Json::Value(error);
		json_object["id"] = Json::Value(id);
		if (error == 0)
			json_object["angle"] = Json::Value(angle);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::do_defaultaction(int error)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_ROBOT_DO_ACTION_RESPONSE");
		json_object["error"] = Json::Value(error);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::do_headstop(int error)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_HEAD_STOP_RESPONSE");
		json_object["error"] = Json::Value(error);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::do_armstop(int error,int id)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_ARM_STOP_RESPONSE");
		json_object["error"] = Json::Value(error);
		json_object["id"] = Json::Value(id);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::change_speed(int error, int id)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_CHANGESPEED_RESPONSE");
		json_object["error"] = Json::Value(error);
		json_object["id"] = Json::Value(id);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::rotatefinish(int id,int endpos)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_ROTATE_FINISH_NOTIFY");
		json_object["id"] = Json::Value(id);
		json_object["finishstate"] = Json::Value(endpos);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::reinitfinish()
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_REINIT_FINISH_NOTIFY");
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::stop_action(int error)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_ROBOT_STOP_ACTION_RESPONSE");
		json_object["error"] = Json::Value(error);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::setswing(int error)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_SET_SWINGANGLE_RESPONSE");
		json_object["error"] = Json::Value(error);
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::reinit(int error)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("MOTION_REINIT_RESPONSE");
		json_object["error"] = Json::Value(error);
		return json_writer.write(json_object);
	}
}; 