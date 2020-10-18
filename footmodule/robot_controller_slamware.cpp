#include "robot_controller_slamware.h"
#include "SlamWareRobotControlModule.h"
#include <io.h>
#include <fstream>
#include <iostream>
#include <fstream>
#include <string>
#include <iosfwd>
#include <sys\stat.h>
#include "robot_PostMessage.h"
#include <set>


robot_controller_slamware::robot_controller_slamware() :m_waitthread(this), m_notifythread(this), m_objdetect(this)
{
	isinit = false;
	isconect = false;
	m_ml = common_mutex_create_threadlock();
	m_laser = common_mutex_create_threadlock();
}


robot_controller_slamware::~robot_controller_slamware()
{
	common_mutex_threadlock_release(m_ml);
	common_mutex_threadlock_release(m_laser);
}


std::vector<std::string> robot_controller_slamware::get_map_file(std::string path)
{
	long hFile = 0;
	struct _finddata_t fileInfo;
	std::string pathName, exdName;
	char pixformat[1024] = { 0 };
	std::vector<std::string> filelist;
	if ((hFile = _findfirst(pathName.assign(path).append("\\slammap\\*.stcm").c_str(), &fileInfo)) == -1)  {
		return std::vector<std::string>();
	}
	do
	{
		if (!(fileInfo.attrib&_A_SUBDIR))
		{
			std::string path_file;
			path_file = path;
			path_file.append("slammap\\");
			path_file.append(fileInfo.name);
			filelist.push_back(path_file);
		}

	} while (_findnext(hFile, &fileInfo) == 0);
	_findclose(hFile);
	return filelist;
}

int robot_controller_slamware::init()
{
	if (isinit)
		return 0;
	if (!m_waitthread.state())
		m_waitthread.start();
	if (!m_notifythread.state())
		m_notifythread.start();
	if (!m_objdetect.state())
		m_objdetect.start();
	std::string ip;
	if (!common_config_get_field_string("robotunderpan", "ip", &ip))
		ip = "192.168.11.1";

	if (!isconect&&InitSlamWare(ip.c_str(), NULL) != 0)
	{
		return 2;
	}
	isconect = true;
	std::string dirpath;
	common_get_current_dir(&dirpath);
	common_fix_path(&dirpath);
	dirpath.append("slammap\\");//sql

	if (!common_isDirectory(dirpath.c_str()))
		common_create_dir(dirpath.c_str());

	std::string path;
	common_get_current_dir(&path);
	std::vector<std::string>  maplist = get_map_file(path);

	if (maplist.size() == 0)
		return 3;

	std::string mapname;
	if (!common_config_get_field_string("robotunderpan", "mapname", &mapname))
	{
		return 0;
	}

	bool hasfile = false;
	int index = 0;
	for (int i = 0; i < maplist.size();i++)
	{
		int begin_pos = maplist[i].find_last_of('\\');
		int endpos = maplist[i].find_last_of('.');

		if (maplist[i].substr(maplist[i].find_last_of('\\')+1, maplist[i].find_last_of('.') - maplist[i].find_last_of('\\')-1) == mapname)
		{
			hasfile = true;
			break;

		}
	}

	if (hasfile)
	{
		if (SetMap(0, 0, 0, maplist[index].c_str()) == 0 && ReLocalization(0) == 0)
		{
			if (WaitNavigation() == STATUS_ActionStatusFinished)
			{
				isinit = true;
				return CONTROLL_OK;
			}			
			currentmap = mapname;
			LoadPoint();	
			return 1;
		}
		else
			return 1;
	}

	
	return 3;

}


int robot_controller_slamware::check_init_finish()
{
	if (!isconect)
		return 1;
	if (isinit)
		return 0;
	if (m_waitthread.state())
	{
		if (m_waitthread.getWaitType() == 2 && m_waitthread.isFinish())
		{
			isinit = true;
			return 0;
		}
		else
			return 1;
	}
	return 0;
}

bool robot_controller_slamware::is_navi()
{
	return 0;
}

bool robot_controller_slamware::is_standby()
{
	return 0;
}

void robot_controller_slamware::release()
{
	Bye();
	if (m_waitthread.state())m_waitthread.quit();
	if (m_notifythread.state())m_notifythread.quit();
	if (m_objdetect.state())m_objdetect.quit();
}

int robot_controller_slamware::updata_ultrasonic_data()
{
	return 0;
}

int robot_controller_slamware::get_laser_data(std::vector<grid_point_data>&data, int &maxsu)
{
	LOCK_HELPER_AUTO_HANDLER(m_laser);
	maxsu = 0;
	tLASERPOINT *dataptr = nullptr;
	data.clear();
	int LaserCount = GetLaserData(&dataptr);
	if (LaserCount < 0)
		return LaserCount;
	if (dataptr)
	{
		for (int i = 0; i < LaserCount;i++)
		{
			if (dataptr[i].value)
			{
				grid_point_data gdata;
				gdata.angle = (dataptr[i].angle * 180 )/ 3.1415925;
				gdata.world.w = dataptr[i].distance;//ÔÝÊ±ÓÃW
				data.push_back(gdata);
			}
	
		}
	}

	return 0;
}

int robot_controller_slamware::get_device_status(device_status &ds)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml); 
	tSlamRobotChargeState srcs;
	if (GetChargeStatus(&srcs))
		 return -1;
	ds.battery = GetBattery();
	ds.charge = srcs.isCharging;
	if (srcs.isDCConnect)
		ds.charger = 4;
	else if (srcs.onDock==1)
		ds.charger = 3;
	else if (ds.battery == 100)
		ds.charger = 5;
	else
		ds.charger = 0;
// 	if (srcs.PowerStageState == 1 || srcs.PowerStageState == 2 || srcs.PowerStageState == 3)
// 		ds.charge = true;
	ds.emergencyStop = srcs.emergencystop;
	return 0;
}

int robot_controller_slamware::get_device_status(device_status &ds,bool &isconnect)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	tSlamRobotChargeState srcs;
	if (GetChargeStatus(&srcs))
		return -1;
	ds.battery = GetBattery();
	ds.charge = srcs.isCharging;
	if (srcs.isDCConnect)
		ds.charger = 4;
	else if (srcs.onDock == 1)
		ds.charger = 3;
	else if (ds.battery == 100)
		ds.charger = 5;
	else
		ds.charger = 0;
	// 	if (srcs.PowerStageState == 1 || srcs.PowerStageState == 2 || srcs.PowerStageState == 3)
	// 		ds.charge = true;
	isconnect = srcs.sdpconnected;
	ds.emergencyStop = srcs.emergencystop;
	return 0;
}


int robot_controller_slamware::get_current_point(grid_point_data &data)
{
	double x, y, yam;
	if (GetCurrentPosition(&x,&y,&yam))
		return -1;
	data.x = x * 1000;
	data.y = y * 1000;
	data.angle = yam;
	return 0;
}

int robot_controller_slamware::get_twopoint_navigate_path(std::string mapname, grid_point_data beginpoint, grid_point_data endpoint, navigation_path_data &data)
{
	return 0;
}

int robot_controller_slamware::get_navigate_path(navigation_path_data &data)
{
	return 0;
}

int robot_controller_slamware::navigate_to(grid_point_data&data)
{
	m_waitthread.CancelMessage();
	if (NavigationTo(((double)data.x )/ 1000.0, ((double)data.y )/ 1000.0, ((double)data.angle), 1))
		return -1;
	m_waitthread.pushq(1);
	return 0;
}

int robot_controller_slamware::navigate_to(std::string name)
{
	auto it = point_list.begin();
	for (; it != point_list.end(); it++)
	{
		if (it->name == name)
			break;
	}
	if (it == point_list.end())
		return -1;
	m_waitthread.CancelMessage();
	if (!SetHome(((double)it->postion.x) / 1000.0, ((double)it->postion.y) / 1000.0, ((double)it->postion.angle)))
		return -2;
	int ret= 	GoHome();
	if (ret==0)
		m_waitthread.pushq(1);
	return ret;
}

int robot_controller_slamware::navigate_pause()
{
	return 0;
}

int robot_controller_slamware::navigate_cancel()
{
	m_waitthread.CancelMessage();
	CancelCurrentAction();
	return 0;
}

int robot_controller_slamware::navigate_resume()
{
	return 0;
}



void robot_controller_slamware::LoadPoint()
{
	if (currentmap.empty())
		return;
	point_list.clear();
	std::ifstream ifi;

	std::string path;
	common_get_current_dir(&path);
	common_fix_path(&path);
	std::string pddbpath = path;
	pddbpath.append("slammap\\");
	pddbpath.append(currentmap);
	pddbpath.append(".points");

	ifi.open(pddbpath, std::ios::in);
	if (ifi.fail())
		return;
	std::string readInfo;
	while (std::getline(ifi, readInfo))
	{
		Json::Value   json_object;
		Json::Reader  json_reader;
		std::string guid;

		bool b_json_read = json_reader.parse(readInfo.c_str(), json_object);
		if (b_json_read)
		{
			special_point_data point_data;
			if (json_object.isMember("name") && json_object["name"].isString())
				point_data.name = json_object["name"].asString();
			if (json_object.isMember("x") && json_object["x"].isInt())
				point_data.postion.x = json_object["x"].asInt();
			if (json_object.isMember("y") && json_object["y"].isInt())
				point_data.postion.y = json_object["y"].asInt();
			if (json_object.isMember("angle") && json_object["angle"].isInt())
				point_data.postion.angle = json_object["angle"].asInt();
			if (json_object.isMember("type") && json_object["type"].isInt())
				point_data.type = json_object["type"].asInt();

			point_list.push_back(point_data);
		}

	}
	ifi.close();
}


void robot_controller_slamware::SavePoint()
{
	if (currentmap.empty())
		return;
	std::string path;
	common_get_current_dir(&path);
	common_fix_path(&path);
	std::string pddbpath = path;
	pddbpath.append("slammap\\");
	pddbpath.append(currentmap);
	pddbpath.append(".points");

	std::ofstream ifi;
	ifi.open(pddbpath, std::ios::out);
	if (ifi.fail())
		return;

	auto it = point_list.begin();

	for (; it != point_list.end(); it++)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;

		json_object["name"] = Json::Value(it->name);
		json_object["x"] = Json::Value(it->postion.x);
		json_object["y "] = Json::Value(it->postion.y);
		json_object["angle"] = Json::Value(it->postion.angle);
		json_object["type"] = Json::Value(it->type);

		std::string out = json_writer.write(json_object).c_str();
		out.append("\n");

		ifi << out;
	}
	ifi.close();
}


int robot_controller_slamware::get_special_point_list(std::string mapname, std::vector<special_point_data>& out, int type /*= -1*/)
{
	auto it = point_list.begin();
	out.clear();
	for (; it != point_list.end(); it++)
	{
		if (type != -1 && it->type != type)
			continue;
		out.push_back(*it);
	}
	return 0;
}

int robot_controller_slamware::remember_point(std::string mapname, special_point_data& point)
{
	if (currentmap.empty())
		return 1;
	point_list.push_back(point);
	SavePoint();
	return 0;
}

int robot_controller_slamware::delete_point(std::string mapname, std::string name)
{
// 	if (mapname != currentmap)
// 		return 1;
	if (currentmap.empty())
		return 1;
	auto it = point_list.begin();
	for (; it != point_list.end(); it++)
	{
		if (it->name == name)
		{
			point_list.erase(it);
			return 0;
		}
	}
	SavePoint();
	return 0;
}

int robot_controller_slamware::get_map_picture(std::string mapname, std::vector<char> &mapdata)
{
	std::string path;
	common_get_current_dir(&path);
	std::vector<std::string>  maplist = get_map_file(path);
	if (maplist.size() == 0)
		return 3;

	bool hasfile = false;
	int index = 0;
	for (int i = 0; i < maplist.size(); i++)
	{
		if (maplist[i].substr(maplist[i].find_last_of('\\') + 1, maplist[i].find_last_of('.') - maplist[i].find_last_of('\\') - 1) == mapname)
		{
			hasfile = true;
			index = i;
			break;

		}
	}

	if (hasfile)
	{
		std::string pathbmp ;
		pathbmp.assign(mapdata.begin(), mapdata.end());
		return ConvertMap2Bmp(maplist[index].c_str(), pathbmp.c_str());
	}
	return 1;
}

int robot_controller_slamware::get_map_list(std::vector<map_data>&  vecmaplist)
{
	std::string path;
	common_get_current_dir(&path);
	std::vector<std::string>  maplist = get_map_file(path);


	for (int i = 0; i < maplist.size();i++)
	{
		map_data temp;
		temp.map_name = maplist[i].substr(maplist[i].find_last_of('\\') + 1, maplist[i].find_last_of('.') - maplist[i].find_last_of('\\') - 1);
		vecmaplist.push_back(temp);
	}
	return 0;
}

int robot_controller_slamware::get_current_map_name(std::string &mapname)
{
	if (!isconect)
		return 2;
	if (currentmap.empty())
		return 1;
	mapname = currentmap;
	return 0;
}

int robot_controller_slamware::get_current_speed(double &ls, double &as)
{
	return 0;
}

int robot_controller_slamware::get_protector(std::vector<int >& frame)
{
	return 0;
}

int robot_controller_slamware::load_map(std::string name)
{
	std::string path;
	common_get_current_dir(&path);
	std::vector<std::string>  maplist = get_map_file(path);
	printf("map size :%d\n", maplist.size());
	if (maplist.size() == 0)
		return 3;

	bool hasfile = false;
	int index = 0;
	for (int i = 0; i < maplist.size(); i++)
	{
		if (maplist[i].substr(maplist[i].find_last_of('\\') + 1, maplist[i].find_last_of('.') - maplist[i].find_last_of('\\') - 1) == name)
		{
			hasfile = true;
			index = i;
			break;

		}
	}

	if (hasfile)
	{
		if (SetMap(0, 0, 0, maplist[index].c_str()) == 0)
		{
			currentmap = name;
			LoadPoint();
			return CONTROLL_OK;
		}
		else
			return 2;
			
	}
	return 1;
}

int robot_controller_slamware::initialize_robot(std::string mapname, std::string name)
{
	return 0;
}

int robot_controller_slamware::initialize_robot(special_point_data data)
{
	m_waitthread.CancelMessage();
	CancelCurrentAction();
	isinit = false;
	if (load_map(data.name))
		return 109;
	if (ReLocalization(0)==0)
	{
		m_waitthread.pushq(2);
		return 0;
	}
	return 1;

}

int robot_controller_slamware::initialize_cancel_robot()
{
	m_waitthread.CancelMessage();
	CancelCurrentAction();
	return 0;
}

std::string robot_controller_slamware::get_health_report()
{
	return "";
}

int robot_controller_slamware::scan_map(std::string name)
{
	scanmapname = name;
	if (ScanMap() != 0)
		return 1;
	return 0;
}

int robot_controller_slamware::save_map()
{
	std::string path;
	common_get_current_dir(&path);
	common_fix_path(&path);
	path.append("slammap\\");
	path.append(scanmapname);
	path.append(".stcm");

	if (SaveMap(path.c_str()) != 0)//ERRROR RET
		return 0;
	return 0;
}

int robot_controller_slamware::cancel_scan()
{
	if (CancelScanMap() != 0)
		return 1;
	return 0;

}

int robot_controller_slamware::get_scan_map(std::vector<char> &mapdata)
{
	std::string pathbmp;
	pathbmp.assign(mapdata.begin(), mapdata.end());
	return GetMapBmp(pathbmp.c_str());
}

int robot_controller_slamware::del_map(std::string name)
{
	std::string path;
	common_get_current_dir(&path);
	std::vector<std::string>  maplist = get_map_file(path);

	if (maplist.size() == 0)
		return 3;

	bool hasfile = false;
	int index = 0;
	for (int i = 0; i < maplist.size(); i++)
	{
		int begin_pos = maplist[i].find_last_of('\\');
		int endpos = maplist[i].find_last_of('.');

		if (maplist[i].substr(maplist[i].find_last_of('\\') + 1, maplist[i].find_last_of('.') - maplist[i].find_last_of('\\') - 1) == name)
		{
			remove(maplist[i].c_str());
			break;

		}
	}


	return 0;
}



int robot_controller_slamware::get_virtual_wall(std::string name, std::string &data)
{
	return 0;
}

int robot_controller_slamware::update_virtual_wall(std::string name, std::string json)
{
	return 0;
}

std::string robot_controller_slamware::get_slopes(std::string name)
{
	return "";
}

int robot_controller_slamware::update_slopes(std::string name, std::string json)
{
	return 0;
}

int robot_controller_slamware::move(double lspeed, double aspeed)
{
	if (abs(aspeed) > 0.0001 && abs(lspeed) < 0.0001)
	{
		if (aspeed>0)
			Move(4, aspeed);
		else
			Move(0, aspeed);
	}
	else if (abs(lspeed) > 0.0001 && abs(aspeed) <0.0001)
	{
		if (lspeed > 0)
			Move(2, lspeed);
		else
			Move(6, lspeed);
	}
	else if (lspeed > 0.0001&&aspeed>0.0001)
		Move(1, lspeed);
	else  if (lspeed > 0.0001&& aspeed < -0.0001)
		Move(7, lspeed);
	else  if (lspeed < -0.0001 && aspeed > 0.0001)
		Move(3, lspeed);
	else if (lspeed < -0.0001 && aspeed <-0.0001)
		Move(5, lspeed);

	return 0;
}

int robot_controller_slamware::moveto(double lspeed, double distance)
{
	
	return 0;
}

int robot_controller_slamware::check_moveto_finish()
{
	return 0;
}

int robot_controller_slamware::stop_moveto()
{
	return 0;
}

int robot_controller_slamware::rotate(double angle, double aspeed)
{
	return RotateAngle(angle, aspeed);
}

int robot_controller_slamware::check_rotate_finish()
{
	enROBOTSTATE Status = GetRobotStatus();
	if (Status == STATUS_ActionStatusRunning || Status == STATUS_ActionStatusRunning)
		return 1;

	if (Status == STATUS_ActionStatusFinished || Status == STATUS_ActionStatusStopped)
		return 0;
	
	return -1;
}

int robot_controller_slamware::stop_rotate()
{
	CancelCurrentAction();
	return 0;
}

std::string robot_controller_slamware::get_version()
{
	if (isconect)
		return "SlamWare";
	return "";
}

int robot_controller_slamware::set_navigation_speed(int level)
{
	if (level==0)
		return SetSpeed(1)?0:1;
	if (level == 1)
		return SetSpeed(2) ? 0 : 1;
	if (level == 2)
		return SetSpeed(0) ? 0 : 1;
	return -1;
}

int robot_controller_slamware::detect_move_data(std::vector<move_object_detect>& object_detect)
{
	LOCK_HELPER_AUTO_HANDLER(m_laser);
	tLASERPOINT *dataptr = nullptr;
	object_detect.clear();
	int LaserCount = GetLaserData(&dataptr);
	if (LaserCount < 0)
		return -1;
	if (dataptr)
	{
		std::vector<move_object_detect> temp,filtobj;
		for (int i = 0; i < LaserCount; i++)
		{
			if (dataptr[i].value)
			{
				move_object_detect obj;
				obj.distance = dataptr[i].distance;
				obj.object_angle = (dataptr[i].angle * 180) / 3.1415925;
				temp.push_back(obj);
			}

		}

		for (int i = 0; i < temp.size(); i++)
		{
			if (temp[i].distance < 5&&abs(temp[i].object_angle)<75)
				filtobj.push_back(temp[i]);
		}
		if (filtobj.size() > 20)
		{
			double near_dis = 0.02,near_angle=1;
			int loop = 0;
			while (filtobj.size()>20)
			{
				temp = filtobj;
				std::set<int> remove_index;
				for (int i = 0; i < temp.size();i++)
				{
					for (int j = 0; j < filtobj.size(); j++)
					{
						if (i==j)
							continue;
						if ((filtobj[j].distance - temp[i].distance)>=0&&
							(filtobj[j].distance - temp[i].distance) <= near_dis &&
							(abs(filtobj[j].object_angle - temp[i].object_angle)) <= near_angle)
						{
							remove_index.insert(j);
						}
					}
				}
				near_dis = near_dis + 0.02;
				loop++;
				if (loop%3==0)
					near_angle = near_angle + 1;
				filtobj.clear();
				for (int j = 0; j < temp.size(); j++)
				{
					if (remove_index.find(j) == remove_index.end())
						filtobj.push_back(temp[j]);
				}
			}
		}
		else
		{
			filtobj = temp;
		}
		for (int i = 0; i < filtobj.size(); i++)
		{
			filtobj[i].distance = filtobj[i].distance * 20;
		}

		object_detect = filtobj;
		return 0;
	}
	return -1;
}

int robot_controller_slamware::start_charge()
{
	return 0;
}

int robot_controller_slamware::stop_charge()
{
	return 0;
}

int robot_controller_slamware::get_charge_state(int &state)
{
	tSlamRobotChargeState srcs;
	if (GetChargeStatus(&srcs))
		return -1;

	if (srcs.PowerStageState == 1 || srcs.PowerStageState == 3)
		state = 1;
	if (srcs.PowerStageState == 2)
		state = 2;
	if (srcs.PowerStageState == 4 || srcs.PowerStageState == 5 || srcs.PowerStageState == 0)
		state = 3;

	return 0;
}

int robot_controller_slamware::shutdown()
{	
	return ShutdownRobot(0, 0);
}

int robot_controller_slamware::soft_reset_reboot()
{
	return ResetRobot(true);
}

int robot_controller_slamware::resume_motor(int type)
{
	return ClearAllError();
}

int robot_controller_slamware::motor(bool type)
{
	return 0;
}

bool slam_thread::isFinish()
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	return finished;
}

void slam_thread::CancelMessage()
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	cancel = true;
}

int slam_thread::getWaitType()
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	return waittype;
}

int slam_thread::getResult()
{
	return result;
}

void slam_thread::run()
{
	while (1)
	{
		Message  msg = getq();
		switch (msg.msgType)
		{
		case 1:
		{
			common_mutex_threadlock_lock(m_ml);
			cancel = false;
			finished = false;
			waittype = 1;
			common_mutex_threadlock_unlock(m_ml);
			LOGI("wait navigation\n");
			enROBOTSTATE ret = WaitNavigation();
			LOGI("wait navigation done %d\n", ret);
			common_mutex_threadlock_lock(m_ml);
			if (!cancel)
			{
				FootCtrlMessage fcmsg;
				if (ret == STATUS_ActionStatusFinished)
					fcmsg.detail.state = 2005 ;
				else if (ret == STATUS_ActionStatusError)
				{
					const char *message_err = GetWaitErrorResult();
					LOGE("slam moveto error :%s", message_err);
					if (strcmp(message_err, "unreachable") == 0)
						fcmsg.detail.state = 2002;
					else
						fcmsg.detail.state = 2008;
				}
				else
					fcmsg.detail.state = 2004;
				fcmsg.response = m_json.navigate_state_ntf(&fcmsg);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, fcmsg.response.c_str(), fcmsg.response.size());
			}
			finished=true;
			common_mutex_threadlock_unlock(m_ml);

		}break;
		case 2:
		{
			common_mutex_threadlock_lock(m_ml);
			cancel = false;
			finished = false;
			waittype = 2;
			common_mutex_threadlock_unlock(m_ml);
			LOGI("wait init");
			enROBOTSTATE ret = WaitNavigation();
			LOGI("wait init done");
			common_mutex_threadlock_lock(m_ml);
			finished = true;
			parent->isinit = (ret == STATUS_ActionStatusFinished ? true : false);
			common_mutex_threadlock_unlock(m_ml);

		}break;
		case  _EXIT_THREAD_:
			return;
		}
	}
}

void slamrobotstate_thread::run()
{
	Timer_helper tick,detectobj;
	int		sleeptime = 1000;
	bool	laststop = false;
	int		lastbattery = -1;
	bool	lastcharger = false;
	bool	detect_status = false;

	device_status ds;
	bool isconnect = false;
	while (1)
	{
		Message  msg = getq(sleeptime);
		switch (msg.msgType)
		{
		case  _EXIT_THREAD_:
			return;
		}
		
		if (parent->get_device_status(ds, isconnect) == 0)
		{
			if (isconnect)
			{
				if (lastcharger ||
					laststop != ds.emergencyStop ||
					lastcharger != ds.charge ||
					lastbattery != ds.battery ||
					tick.isreach(2 * 60 * 1000))
				{
					tick.update();
					FootCtrlMessage fcmsg;
					fcmsg.response = m_json.low_battery_ntf(fcmsg.error, ds.battery, ds.charge, ds.emergencyStop, ds.charger);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, fcmsg.response.c_str(), fcmsg.response.size());

				}
				lastcharger = ds.charge;
				lastbattery = ds.battery;
				laststop = ds.emergencyStop;
			}
			else
			{
				std::string ip;
				if (!common_config_get_field_string("robotunderpan", "ip", &ip))
					ip = "192.168.11.1";
				parent->isconect = false;
				if (InitSlamWare(ip.c_str(), NULL) != 0)
				{
					parent->isconect = true;
				}
			}
			
		}
	}
}

void slamrobotobjdetect_thread::run()
{
	Timer_helper tick;
	int		sleeptime = 500;
	bool	detect_status = false;

	while (1)
	{
		Message  msg = getq(sleeptime);
		switch (msg.msgType)
		{
		case  _EXIT_THREAD_:
			return;
		}
	
		std::vector<grid_point_data> pointdata;

		int maxsu = 0;
		if (parent->m_waitthread.getWaitType() == 1 &&
			!parent->m_waitthread.isFinish())
		{
			int ret = parent->get_laser_data(pointdata, maxsu);
			if (ret == 0)
			{
				bool hasnear_object = false;
				for (int index=0; index < pointdata.size(); index++)
				{
					if (abs(pointdata[index].angle) < 45 && abs(pointdata[index].world.w) < 0.6)
						hasnear_object = true;
				}

				if (hasnear_object&&(detect_status != hasnear_object || tick.isreach(10000)))
				{
					FootCtrlMessage fcmsg;
					fcmsg.detail.state = 2003;
					fcmsg.response = m_json.navigate_state_ntf(&fcmsg);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, fcmsg.response.c_str(), fcmsg.response.size());
					tick.update();
				}
				detect_status = hasnear_object;
			}else
				detect_status = false;

		}
		else
		{
			detect_status = false;
		}

	}
}
