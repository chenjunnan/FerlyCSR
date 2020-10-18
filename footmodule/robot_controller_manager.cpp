#include "robot_controller_manager.h"
#include "xCommon.h"
#include "robot_PostMessage.h"
#include "robot_controller_struct.h"


robot_controller_manager::robot_controller_manager()
{
}


robot_controller_manager::~robot_controller_manager()
{
}


robot_controller_manager * robot_controller_manager::getInstance()
{
	static robot_controller_manager _s;
	return &_s;
}

bool robot_controller_manager::init()
{
	std::string  undptype = "";
	int  type = 0;
	if (!common_config_get_field_string("robotunderpan", "type", &undptype))
		undptype = "gaussian";
	if (undptype.compare("gaussian") == 0)
		type = 0;
	if (undptype.compare("ferly") == 0)
		type = 1;
	if (undptype.compare("slamware") == 0)
		type = 2;
	m_controller_handler[0] = new robot_controller_moduel();
	if (!m_controller_handler[0])
	{
		m_controller_handler.clear();
		return false;
	}	
	m_controller_handler[0]->init(0,type);//only one now 
	return true;
}

bool robot_controller_manager::start()
{
	bool readly = true;
	for (auto it = m_controller_handler.begin(); it != m_controller_handler.end(); it++)
	{
		if (!it->second->start())readly = false;
	}
		
	return readly;
}

void robot_controller_manager::stop()
{
	for (auto it = m_controller_handler.begin(); it != m_controller_handler.end(); it++)
		it->second->stop();
	return;
}

void robot_controller_manager::append_message_type(const char * message)
{
	m_message_list.push_back(message);
}

int robot_controller_manager::message_handler(int dAppid, int dMessageType, const char *strFrom, const char *strMessage, unsigned long dMessagelen, const unsigned char *strExtdata /*= NULL*/, unsigned long dExtlen /*= 0*/)
{
	FootCtrlMessage   fcmsg;
	fcmsg.appid = dAppid;
	fcmsg.fromguid = strFrom;
	fcmsg.content = strMessage;
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool ret_value = false;

	bool b_json_read = json_reader.parse(strMessage, json_object);
	if (b_json_read)
	{
		std::string gbkstring;
		//common_utf8_2gbk(&fcmsg.content, &gbkstring);
		//LOGD("Receive controller message request %s", gbkstring.c_str());
		if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
		{
			std::string msgtype = json_object["msg_type"].asString();
			common_trim(&msgtype);
			int dmtype = -1;
			for (int i = 0; i < m_message_list.size(); i++)
			{
				if (msgtype.compare(m_message_list[i]) == 0)
				{
					dmtype = i;
					break;
				}
			}

			int controllerid = 0;
			if (json_object.isMember("controllerid") && json_object["controllerid"].isInt())
				controllerid = json_object["controllerid"].asInt();

			parse_jsonmessage(json_object, fcmsg);

			if (m_controller_handler.find(controllerid) == m_controller_handler.end())
				return 1;

			fcmsg.messagetype = (MESSAGE_TASK_NAME)dmtype;
			
			if (fcmsg.messagetype<ROBOT_CONTROLLRT_END&&fcmsg.messagetype>ROBOT_UNKNOW)
				ret_value = m_controller_handler[controllerid]->message_handler(fcmsg);

			if (ret_value)
			{

				if (!fcmsg.response.empty())post_report(fcmsg);
			}

			if (fcmsg.messagetype == NAVIGATE_SHUTDOWN_REQUEST&&fcmsg.error==0)
			{
				system("taskkill /F /im RobotGuard.exe");
				Json::Value   json_object;
				Json::FastWriter json_writer;
				json_object["msg_type"] = Json::Value("DISPATCH_SHUTDOWN_REQUEST");
				std::string regmessage = json_writer.write(json_object);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, regmessage.c_str(), regmessage.length(), "dispath_service");
			}
		}
	}
	return ret_value;

}

void robot_controller_manager::build_report(FootCtrlMessage &rcs,std::string &msgtype)
{

}

void robot_controller_manager::parse_jsonmessage(Json::Value &json_object, FootCtrlMessage &fcmsg)
{
	if (json_object.isMember("x") && json_object["x"].isInt())
		fcmsg.detail.x = json_object["x"].asInt();
	if (json_object.isMember("y") && json_object["y"].isInt())
		fcmsg.detail.y = json_object["y"].asInt();
	if (json_object.isMember("r") && json_object["r"].isInt())
		fcmsg.detail.r = json_object["r"].asInt();
	if (json_object.isMember("mapname") && json_object["mapname"].isString())
		fcmsg.detail.mapname = json_object["mapname"].asString();
	if (json_object.isMember("linear_speed") && json_object["linear_speed"].isDouble())
		fcmsg.detail.ls = json_object["linear_speed"].asDouble();
	if (json_object.isMember("angular_speed") && json_object["angular_speed"].isDouble())
		fcmsg.detail.as = json_object["angular_speed"].asDouble();
	if (json_object.isMember("angle") && json_object["angle"].isInt())
		fcmsg.detail.r = json_object["angle"].asInt();
	if (json_object.isMember("name") && json_object["name"].isString())
		fcmsg.detail.name = json_object["name"].asString();
	if (json_object.isMember("level") && json_object["level"].isInt())
		fcmsg.detail.x = json_object["level"].asInt();
	if (json_object.isMember("task_type") && json_object["task_type"].isString())
		fcmsg.detail.name = json_object["task_type"].asString();
	if (json_object.isMember("greeter_name") && json_object["greeter_name"].isString())
		fcmsg.detail.greeter_name = json_object["greeter_name"].asString();
	if (json_object.isMember("distance") && json_object["distance"].isDouble())
		fcmsg.detail.as= json_object["distance"].asDouble();
	if (json_object.isMember("on") && json_object["on"].isBool())
		fcmsg.detail.fstat = json_object["on"].asBool();
	if (json_object.isMember("start") && json_object["start"].isObject())
	{
		special_point_data point;
		Json::Value grid = json_object["start"];
		if (grid.isMember("x") && grid["x"].isInt())
			point.postion.x = grid["x"].asInt();
		if (grid.isMember("y") && grid["y"].isInt())
			point.postion.y = grid["y"].asInt();
		if (grid.isMember("r") && grid["r"].isInt())
			point.postion.angle = grid["r"].asInt();
		fcmsg.detail.point_list.push_back(point);
	}
	if (json_object.isMember("end") && json_object["end"].isObject())
	{
		special_point_data point;
		Json::Value grid = json_object["end"];
		if (grid.isMember("x") && grid["x"].isInt())
			point.postion.x = grid["x"].asInt();
		if (grid.isMember("y") && grid["y"].isInt())
			point.postion.y = grid["y"].asInt();
		if (grid.isMember("r") && grid["r"].isInt())
			point.postion.angle = grid["r"].asInt();
		fcmsg.detail.point_list.push_back(point);
	}
	if (json_object.isMember("virtual_wall") && json_object["virtual_wall"].isObject())
	{
		Json::FastWriter write_j;
		Json::Value vw = json_object["virtual_wall"];
		std::string out = write_j.write(vw);
		fcmsg.detail.greeter_name = out;
	}
	if (json_object.isMember("type") && json_object["type"].isInt())
		fcmsg.detail.type = json_object["type"].asInt();


	if (json_object.isMember("worldx") && json_object["worldx"].isDouble())
		fcmsg.detail.ls = json_object["worldx"].asDouble();
	if (json_object.isMember("worldy") && json_object["worldy"].isDouble())
		fcmsg.detail.as = json_object["worldy"].asDouble();
	if (json_object.isMember("worldr") && json_object["worldr"].isDouble())
		fcmsg.detail.rs = json_object["worldr"].asDouble();

	if (json_object.isMember("mapdatatype") && json_object["mapdatatype"].isInt())
		fcmsg.detail.state = json_object["mapdatatype"].asInt();

	if (json_object.isMember("taskqueue") && json_object["taskqueue"].isObject())
	{
		Json::Value queuevalue = json_object["taskqueue"];
		if (queuevalue.isMember("loop") && queuevalue["loop"].isBool())
			fcmsg.detail.taskcall.loop = queuevalue["loop"].asBool();
		if (queuevalue.isMember("loop_count") && queuevalue["loop_count"].isInt())
			fcmsg.detail.taskcall.loop_count = queuevalue["loop_count"].asInt();
		if (queuevalue.isMember("mapname") && queuevalue["mapname"].isString())
			fcmsg.detail.taskcall.mapname = queuevalue["mapname"].asString();
		if (queuevalue.isMember("queuename") && queuevalue["queuename"].isString())
			fcmsg.detail.taskcall.queuename = queuevalue["queuename"].asString();
		if (queuevalue.isMember("tasks") && queuevalue["tasks"].isArray())
		{			
			for (int i = 0; i < queuevalue["tasks"].size();i++)
			{
				Json::Value onetask = queuevalue["tasks"][i];
				if (onetask.isObject())
				{
					NaviTask temptask;
					if (onetask.isMember("taskname") && onetask["taskname"].isString())
						temptask.taskname = onetask["taskname"].asString();
					if (onetask.isMember("mapname") && onetask["mapname"].isString())
						temptask.mapname = onetask["mapname"].asString();
					if (onetask.isMember("path_name") && onetask["path_name"].isString())
						temptask.path_name= onetask["path_name"].asString();
					if (onetask.isMember("graph_name") && onetask["graph_name"].isString())
						temptask.graph_name = onetask["graph_name"].asString();
					if (onetask.isMember("graph_path_name") && onetask["graph_path_name"].isString())
						temptask.graph_path_name = onetask["graph_path_name"].asString();
					if (onetask.isMember("position_name") && onetask["position_name"].isString())
						temptask.pointname = onetask["position_name"].asString();
					if (onetask.isMember("graph_path_group_name") && onetask["graph_path_group_name"].isString())
						temptask.pointname = onetask["graph_path_group_name"].asString();
					if (onetask.isMember("x") && onetask["x"].isInt())
						temptask.position.x = onetask["x"].asInt();
					if (onetask.isMember("y") && onetask["y"].isInt())
						temptask.position.y = onetask["y"].asInt();
					if (onetask.isMember("r") && onetask["r"].isInt())
						temptask.position.angle = onetask["r"].asInt();
					fcmsg.detail.taskcall.queueinfo.push_back(temptask);
				}
			}
		}


	}


}

void robot_controller_manager::post_report(FootCtrlMessage &vmsg)
{
	robot_PostMessage::getPostManager()->post(MODULE_NAME, vmsg.response.c_str(), vmsg.response.size(), vmsg.fromguid.c_str());
}
