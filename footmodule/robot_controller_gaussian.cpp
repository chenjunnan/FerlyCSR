#include "robot_controller_gaussian.h"
#include "common_helper.h"
#include "json/json.h"
#include "robot_PostMessage.h"
#include "robot_http_pool.h"


bool g_navi_state = false;
Timer_helper   ntf_time;
Timer_helper   healthntf_time;
int gaussian_version = 0;
gaussian_buffer_data  gaussian_ws_cache;
robot_json_rsp		g_json;
std::string health;
CHANDLE  m_ml = NULL;
gaussian_websocket_info  gwi[4];
robot_controller_gaussian::robot_controller_gaussian() :m_ws_th(this)
{
	m_ip = GS_IP;
	m_port = GS_PORT;
	m_ml = common_mutex_create_threadlock();
	inti_count = 0;
}

static void ifemergency_infomation(Json::Value &json_object)
{
	bool laststop   = gaussian_ws_cache.in_stop;
	int lastbattery = gaussian_ws_cache.battery;
	bool lastcharger = gaussian_ws_cache.charge;
	if (json_object.isMember("emergency") && json_object["emergency"].isBool())
	{
		bool stop1 = false, stop2 = false;
		stop1 = json_object["emergency"].asBool();
		int nowstate;
		if (json_object.isMember("emergencyStop") && json_object["emergencyStop"].isBool())
		{
			stop2 = (int)json_object["emergencyStop"].asBool();
			if (stop1 || stop2)gaussian_ws_cache.in_stop = true;
			else gaussian_ws_cache.in_stop = false;
		}
		else
			gaussian_ws_cache.in_stop = stop1;
		
		if (json_object.isMember("battery") && json_object["battery"].isDouble())
		{
			gaussian_ws_cache.battery = (int)json_object["battery"].asDouble();
		}
		if (json_object.isMember("chargerStatus") && json_object["chargerStatus"].isBool())
		{
			gaussian_ws_cache.charge = json_object["chargerStatus"].asBool();
		}
		if (json_object.isMember("charger") && json_object["charger"].isInt())
		{
			nowstate = json_object["charger"].asInt();
			if (nowstate != 0)
				gaussian_ws_cache.charge = true;
		}
		if (json_object.isMember("speed") && json_object["speed"].isDouble())
		{
			double speed = json_object["speed"].asDouble();
		}
		if (gaussian_ws_cache.charge || 
			laststop	!=gaussian_ws_cache.in_stop||
			lastcharger != gaussian_ws_cache.charge||
			lastbattery != gaussian_ws_cache.battery||
			ntf_time.isreach(1000*60*2)
			)

		{
			
			FootCtrlMessage fcmsg;
			fcmsg.response = g_json.low_battery_ntf(fcmsg.error, gaussian_ws_cache.battery, gaussian_ws_cache.charge, gaussian_ws_cache.in_stop, nowstate);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, fcmsg.response.c_str(), fcmsg.response.size());
			ntf_time.update();

		}
	}
}

static void ifnavigation_infomation(Json::Value &json_object)
{

	std::string notice_str;
	std::string notice_fields_str;
	grid_point_data postion;
	if (json_object.isMember("noticeType") && json_object["noticeType"].isString())
	{
		notice_str = json_object["noticeType"].asString();
		if (json_object.isMember("noticeDataFields") && json_object["noticeDataFields"].isString())
		{
			notice_fields_str = json_object["noticeDataFields"].asString();
		}

		if (notice_fields_str.size() != 0)
		{
			if (json_object.isMember("data") && json_object["data"].isObject())
			{
				Json::Value   json_data = json_object["data"];
				if (json_data.isMember(notice_fields_str.c_str()) && json_data[notice_fields_str.c_str()].isObject())
				{
					Json::Value   json_realDestination = json_data[notice_fields_str.c_str()];
					if (json_realDestination.isMember("angle") && json_realDestination["angle"].isDouble())
						postion.angle = json_realDestination["angle"].asDouble();
					if (json_realDestination.isMember("gridPosition") && json_realDestination["gridPosition"].isObject())
					{
						Json::Value   json_xy = json_realDestination["gridPosition"];
						if (json_xy.isMember("x") && json_xy["x"].isInt())
							postion.x = json_xy["x"].asDouble();
						if (json_xy.isMember("y") && json_xy["y"].isInt())
							postion.y = json_xy["y"].asDouble();
					}
				}

			}
		}
		int new_state = 0;
		if (notice_str.find("LOCALIZATION_FAILED") != -1)
		{
			g_navi_state = false;
			new_state = 2001;
		}

		if (notice_str.find("GOAL_NOT_SAFE") != -1)
			new_state = 2002;

		if (notice_str.find("TOO_CLOSE_TO_OBSTACLES") != -1)
			new_state = 2003;
		if (notice_str.find("UNREACHABLE") != -1)
		{	
			g_navi_state = false;
			new_state = 2004;
		}
		if (notice_str.find("REACHED") != -1)
		{
			g_navi_state = false;
			new_state = 2005;
		}

		if (notice_str.find("HEADING") != -1)
		{
			g_navi_state = true;
			new_state = 2006;
		}
		if (notice_str.find("PLANNING") != -1)
			new_state = 2007;
		if (notice_str.find("UNREACHED") != -1)
		{
			g_navi_state = false;
			new_state = 2008;
		}



		if (new_state != 0 && gaussian_ws_cache.save_state != new_state)
		{
			FootCtrlMessage fcmsg;
			fcmsg.detail.state = new_state;
			fcmsg.detail.x = postion.x;
			fcmsg.detail.y = postion.y;
			fcmsg.detail.r = postion.angle;
			fcmsg.response = g_json.navigate_state_ntf(&fcmsg);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, fcmsg.response.c_str(), fcmsg.response.size());
			if (gaussian_ws_cache.save_state == 2007 && new_state==2002)
				g_navi_state = false;
			gaussian_ws_cache.save_state = new_state;
			
			switch (new_state)
			{
			case 2001:
				LOGI("[robot navigation] inint error");
				break;
			case 2002:
				LOGI("[robot navigation] error position");
				break;
			case 2004:
				LOGI("[robot navigation] unreachable position");
				break;
			case 2005:
				LOGI("[robot navigation] reached position");
				break;
			case 2006:
				LOGI("[robot navigation] start navigation to position");
				break;
			case 2008:
				LOGI("[robot navigation] unreached position");
				break;
				
			}
		}
	}
}

static void ifgreeter_infomation(Json::Value &json_object)
{
	if (json_object.isMember("statusCode") && json_object["statusCode"].isInt())
	{	
		int new_state = json_object["statusCode"].asInt();
		
		
		std::string msg;
		if (json_object.isMember("statusMsg") && json_object["statusMsg"].isString())
			msg = json_object["statusMsg"].asString();
		Json::Value temp;

		std::string jsonreponse;

		jsonreponse = g_json.navigate_system_state_ntf(new_state, msg);


		if (new_state >= 402 && new_state <= 408 || new_state == 600 || new_state == 601 || new_state == 301 || new_state == 303 || new_state == 304)
		{
			if (gaussian_ws_cache.greeter_state == new_state)
				return;
		}
		if (new_state < 404 || new_state>408)
		{

			if (json_object.isMember("statusData") && json_object["statusData"].isObject())
			{
				temp = json_object["statusData"];
				jsonreponse = g_json.navigate_system_state_ntf(new_state, msg, temp);
			}

		}

		robot_PostMessage::getPostManager()->post(MODULE_NAME, jsonreponse.c_str(), jsonreponse.size());
		gaussian_ws_cache.greeter_state = new_state;
		if (new_state == 1006)
		{
			FootCtrlMessage fcmsg;
			fcmsg.detail.state = 2004;
			fcmsg.response = g_json.navigate_state_ntf(&fcmsg);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, fcmsg.response.c_str(), fcmsg.response.size());
			g_navi_state = false;
		}
		if (new_state == 600)
		{
			
		}
	}
}

static void ifhealth_infomation(Json::Value &json_object)
{
	if (json_object.isMember("deviceTopic") && json_object["deviceTopic"].isBool())
	{
		Json::FastWriter json_writer;
		Json::Value  json_data;
		json_data["msg_type"] = Json::Value("NAVIGATE_HEALTH_RESPONSE");
		json_data["health_status"] = Json::Value(json_object);
		json_data["error"] = Json::Value(0);
		health = json_writer.write(json_data);

		bool lms = false;
		bool rms = false; 
		if (json_object.isMember("leftMotor") && json_object["leftMotor"].isBool())
			lms = json_object["leftMotor"].asBool();
		if (json_object.isMember("rightMotor") && json_object["rightMotor"].isBool())
			rms = json_object["rightMotor"].asBool();

		if (!lms || !rms)
		{
			LOGW("resume motor error");
			((robot_controller_gaussian*)gwi[3].cinterface)->resume_motor(-1);
		}
	}
}

void stringreplace(std::string &strBase, std::string strSrc, std::string strDes)
{
	std::string::size_type pos = 0;
	std::string::size_type srcLen = strSrc.size();
	std::string::size_type desLen = strDes.size();
	pos = strBase.find(strSrc, pos);
	while ((pos != std::string::npos))
	{
		strBase.replace(pos, srcLen, strDes);
		pos = strBase.find(strSrc, (pos + desLen));
	}
}

static int
callback_state(struct lws *wsi, enum lws_callback_reasons reason, void *user,void *in, size_t len)
{
	int n=0;
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	for (int i = 0; i < 4; i++)
	{
		if (wsi == gwi[i].wsi)
			n = i;
	}


	switch (reason) {

		  /* when the callback is used for client operations --> */

	case LWS_CALLBACK_CLOSED:
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		LOGE("%s closed error %d [%s]", gwi[n].url.c_str(), reason, in == NULL ? "nul":in);
		gwi[n].returnresult = true;
		gwi[n].success = false;

		gaussian_ws_cache.connect_state = 0;
		gaussian_ws_cache.save_state = 0;
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		LOGI("%s client has connected", gwi[n].url.c_str());
		gaussian_ws_cache.index = 0;
		gaussian_ws_cache.len = -1;
		gaussian_ws_cache.connect_state = 2;
		gaussian_ws_cache.save_state = 0;
		gwi[n].returnresult = true;
		gwi[n].success = true;

		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
	{
		Json::Value   json_object;
		Json::Reader  json_reader;
		if (in)
		{
			std::string temp;// = (char*)in;
			temp.assign((char*)in, (char*)in + len);
			int pos = temp.find("\"{\\\"");
			if (pos != -1)
			{
				stringreplace(temp, "\\", "");
				temp.erase(temp.begin());
				temp.pop_back();
				temp.pop_back();
			}
			if (temp.size() < 3)
				break;
			if (temp.at(0) != '{' || !(temp.at(temp.size() - 1) == '}'||temp.at(temp.size() - 2)=='}'))
				break;
			
			try
			{
				bool b_json_read = json_reader.parse(temp.c_str(), json_object);
				if (b_json_read)
				{
					if (gwi[n].url.compare("/gs-robot/notice/navigation_status") == 0)
					{
						//printf("%s\n", in);
						ifnavigation_infomation(json_object);
					}
					else if (gwi[n].url.compare("/gs-robot/notice/device_status") == 0)
						ifemergency_infomation(json_object);
					else  if (gwi[n].url.compare("/gs-robot/notice/status") == 0)
					{
						//printf("%s\n", in);
						ifgreeter_infomation(json_object);
					}
					else if (gwi[n].url.compare("/gs-robot/notice/system_health_status") == 0)
						ifhealth_infomation(json_object);
					else printf("unknow data\n");
				}
				else
				{
					LOGE("gaussian data error");
				}
			}
			catch (const Json::Exception& e)
			{
				LOGE("gaussian json data error %s",e.what());
			}
			catch (const std::exception& e) {
				LOGE("gaussian json data std error %s", e.what());
			}
			catch (...)
			{
				LOGE("gaussian json data error unkonw");
			}


		}
	}
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:


		break;
	case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
		/* reject everything else except permessage-deflate */
		if (strcmp((char*)in, "permessage-deflate"))
			return 1;
		break;

	default:
		break;
	}

	return 0;
}


static struct lws_protocols protocols[] = {
	/* first protocol must always be HTTP handler */

	{
		"",		/* name - can be overridden with -e */
		callback_state,
		0/*sizeof(struct gaussian_buffer_data)*/,	/* per_session_data_size */
		40960
	},
	{
		NULL, NULL, 0		/* End of list */
	}
};

static const struct lws_extension exts[] = {
	{
		"permessage-deflate",
		lws_extension_callback_pm_deflate,
		"permessage-deflate; client_no_context_takeover"
	},
	{
		"deflate-frame",
		lws_extension_callback_pm_deflate,
		"deflate_frame"
	},
	{ NULL, NULL, NULL /* terminator */ }
};


int robot_controller_gaussian::init()
{
	m_ip = GS_IP;
	m_port = GS_PORT;//sql

	if (gs_version())return 2;
	
	m_ws_th.start();

	if (check_init_finish() == 0)
	{
		LOGW("already initialize !!");

		return CONTROLL_OK;
	}
	else
		return try_initself();

	return CONTROLL_OK;
}

int robot_controller_gaussian::try_initself()
{
	if (inti_count >= 10)return 3;
	inti_count++;
	std::vector<map_data> maplist;
	special_point_data initdata;
	grid_point_data   cur_point;
	std::string mapname;

	if (get_map_list(maplist))
		return 1;

	if (maplist.size() == 0)
	{
		LOGE("none map ,please create local map !!!!");
		inti_count = 10;
		return 3;
	}


	for (int i = 0; i < maplist.size(); i++)
	{
		std::string temp;
		common_utf8_2gbk(&maplist[i].map_name, &temp);
		LOGD("%s", temp.c_str());
	}



	if (!common_config_get_field_string("robotunderpan", "mapname", &mapname))
	{
		get_current_map_name(mapname);
	}

	if (mapname.empty())
	{
		if (maplist.size() == 1)
			mapname = maplist[0].map_name;
		else
		{
			inti_count = 10;
			return 3;
		}
	}

	bool flag = false;
	for (int i = 0; i < maplist.size(); i++)
	{
		if (mapname.compare(maplist[i].map_name) == 0)
			flag = true;
	}
	if (!flag)
	{
		common_utf8_2gbk(&mapname, &mapname);
		LOGE("unfind map name:%s", mapname.c_str());
		inti_count = 10;
		return 3;
	}


	initdata.name = mapname;

	int tn = 0;
	if (!common_config_get_field_int("robotunderpan", "x", &tn))
		tn = 0;
	initdata.postion.x = tn;

	tn = 0;
	if (!common_config_get_field_int("robotunderpan", "y", &tn))
		tn = 0;
	initdata.postion.y = tn;

	tn = 0;
	if (!common_config_get_field_int("robotunderpan", "r", &tn))
		tn = 0;
	initdata.postion.angle = tn;//last save in sql, server start must init once;

	if (initdata.postion.x == 0 || initdata.postion.y == 0)
	{
		if (initialize_robot(initdata.name, "Current") != CONTROLL_OK)
			return 1;
		else
			return wait_init() ? 0 : 1;

	}
	else
	{
		device_status d_status;
		if (get_device_status(d_status) == CONTROLL_OK&& !d_status.charge&&!d_status.emergencyStop)
		{
			if (initialize_robot(initdata) != CONTROLL_OK)
				return 1;
			else
				return wait_init()?0:1;
		}
		else
			return 1;
	}
	return 0;
}

int robot_controller_gaussian::gs_version()
{
	std::string version = get_version();
	if (version.empty())return 1;

	if (inti_count == 0)
	{
		LOGI("controller gaussian version : %s", version.c_str());

		gaussian_version = get_number_version(version);

		LOGD("\n---VERSION----: %d", gaussian_version);
	}
	return 0;
}

int robot_controller_gaussian::get_number_version(std::string &version)
{
	Json::Value   json_object;
	Json::Reader  json_reader;
	int version_num = 0;
	bool b_json_read = json_reader.parse(version.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("data") && json_object["data"].isObject())
		{
			Json::Value   json_data = json_object["data"];
			std::string version, softversion;
			if (json_data.isMember("version") && json_data["version"].isString())
				version = json_data["version"].asString();
			if (json_data.isMember("softwareVersion") && json_data["softwareVersion"].isString())
				softversion = json_data["softwareVersion"].asString();
			if (!version.empty() && !softversion.empty() && softversion.compare(version) == 0)
			{
				//GS-SR06-FL-BANGBANG-10-1.1.4
				//GS-SR06-FL-CIRCLE-10-2.1.12
				if (version.find("GS-SR06-FL-CIRCLE") != -1)
				{
					int num = version.rfind("-");
					if (num != -1)
					{
						std::string numstr = version.substr(num + 1, version.size() - num - 1);
						num = numstr.find(".");
						int position = 1000;

						int begin_position = 0;
						while (num != -1)
						{
							std::string snum = numstr.substr(begin_position, num - begin_position);
							if (position >= 100)
								version_num += (position*atoi(snum.c_str()));
							position /= 10;
							begin_position = num + 1;
							num = numstr.find(".", begin_position);
							if (num == -1)
							{
								snum = numstr.substr(begin_position, numstr.length() - begin_position);
								version_num += atoi(snum.c_str());
								break;
							}
						}

					}
				}
				if (version.find("GS-SR06-FL-BANGBANG") != -1)
				{
					version_num = 9999;
				}

			}
		}
	}
	return version_num;
}

bool robot_controller_gaussian::is_navi()
{
	return g_navi_state;
}

bool robot_controller_gaussian::is_standby()
{
	if (g_navi_state || 
		gaussian_ws_cache.greeter_state == 600 ||
		gaussian_ws_cache.greeter_state == 601 ||
		gaussian_ws_cache.greeter_state == 604 ||
		gaussian_ws_cache.greeter_state == 1001)
		return false;
	return true;
}

int robot_controller_gaussian::updata_ultrasonic_data()
{
	ultrasonic_data ultdata;
	m_ultrasonic.clear();
	m_ultrasonic.resize(3);

	std::string url = "/gs-robot/real_time_data/ultrasonic_phit";
	append_url_head(url); 
	
	std::string out = imp_get(url);

	std::string data;
	if (!simple_msg(out, data))
		return CONTROLL_FAILED;

	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(data.c_str(), json_object);
	if (b_json_read)
	{

		if (json_object.isMember("gridPhits") && json_object["gridPhits"].isArray())
		{
			Json::Value   mobile_data = json_object["gridPhits"];

			for (int i = 0; i < mobile_data.size(); i++)
			{
				if (mobile_data[i].isObject())
				{
					Json::Value gridPhits_data = mobile_data[i];

					if (gridPhits_data.isMember("x") && gridPhits_data["x"].isInt())
					{
						ultdata.object_postion.x = gridPhits_data["x"].asInt();
					}
					if (gridPhits_data.isMember("y") && gridPhits_data["y"].isInt())
					{
						ultdata.object_postion.y = gridPhits_data["y"].asInt();
					}

				}

			}


		}

	}
	else
		return CONTROLL_FAILED;

	return CONTROLL_OK;
}

int robot_controller_gaussian::get_laser_data(std::vector<grid_point_data>& laser_list,int &maxSu)
{
	laser_list.clear();

	std::string url = "/gs-robot/real_time_data/laser_phit";
	append_url_head(url);
	std::string out = imp_get(url);

	maxSu = 0;
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		//"mapInfo":{"gridHeight":736,"gridWidth":800,
		if (json_object.isMember("mapInfo") && json_object["mapInfo"].isObject())
		{
			Json::Value mapinfo = json_object["mapInfo"];
			int hVaule = 0, wVaule = 0;
			if (mapinfo.isMember("gridHeight") && mapinfo["gridHeight"].isInt())
				hVaule = mapinfo["gridHeight"].asInt();
			if (mapinfo.isMember("gridWidth") && mapinfo["gridWidth"].isInt())
				wVaule = mapinfo["gridWidth"].asInt();
			maxSu = hVaule > wVaule ? hVaule : wVaule;
			if (maxSu)
			{
				if (json_object.isMember("gridPhits") && json_object["gridPhits"].isArray())
				{
					Json::Value   grid_data = json_object["gridPhits"];

					for (int i = 0; i < grid_data.size(); i++)
					{
						grid_point_data data;
						if (grid_data[i].isObject())
						{
							Json::Value gridPhits_data = grid_data[i];

							if (gridPhits_data.isMember("x") && gridPhits_data["x"].isInt())
							{
								data.x = gridPhits_data["x"].asInt();
							}
							if (gridPhits_data.isMember("y") && gridPhits_data["y"].isInt())
							{
								data.y = gridPhits_data["y"].asInt();
							}
							laser_list.push_back(data);
						}

					}
				}
			}

		}
	}
	else
		return CONTROLL_FAILED;

	return CONTROLL_OK;
}

int robot_controller_gaussian::get_device_status(device_status &ds)
{
	std::string mapname;

	std::string url = "/gs-robot/data/device_status";
	append_url_head(url);

	//LOGI("gaussian get device_status url %s", url.c_str());
	std::string out = imp_get(url);

	if (!simple_msg(out, out))
		return CONTROLL_FAILED;

	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{

		if (json_object.isMember("data") && json_object["data"].isObject())
		{
			Json::Value   json_data = json_object["data"];
			if (json_data.isMember("emergency") && json_data["emergency"].isBool())
			{
				ds.emergencyStop = json_data["emergency"].asBool();
				bool  stop2 = false;
				if (json_object.isMember("emergencyStop") && json_object["emergencyStop"].isBool())
				{
					stop2 = (int)json_object["emergencyStop"].asBool();
					if (ds.emergencyStop || stop2)ds.emergencyStop = true;
					else ds.emergencyStop = false;
				}
			}
			if (json_data.isMember("chargerStatus") && json_data["chargerStatus"].isBool())
			{
				ds.charge = json_data["chargerStatus"].asBool();
			}
			if (json_data.isMember("battery") && json_data["battery"].isDouble())
			{
				ds.battery = (int)json_data["battery"].asDouble();
			}
			if (json_data.isMember("charger") && json_data["charger"].isInt())
			{
				ds.charger = json_data["charger"].asInt();
				if (ds.charger != 0)
					ds.charge = true;
			}
			if (json_data.isMember("speed") && json_data["speed"].isDouble())
			{
				ds.speed = json_data["speed"].asDouble();
			}
			if (json_data.isMember("navigationSpeedLevel") && json_data["navigationSpeedLevel"].isInt())
			{
				ds.speedlevel = json_data["navigationSpeedLevel"].asInt();
			}
			if (json_data.isMember("uptime") && json_data["uptime"].isInt())
			{
				ds.startuptime = json_data["uptime"].asInt();
			}
			ds.navigation_status = is_navi();
		}else
			return CONTROLL_FAILED;
	
	}else
		return CONTROLL_FAILED;

	return CONTROLL_OK;
}

void robot_controller_gaussian::release()
{
	if (m_ws_th.state())
		m_ws_th.quit();
	robot_http_pool::getSingle()->ExitAll();
}

int robot_controller_gaussian::get_current_point(grid_point_data & point)
{
	//if (check_init() != 0)return CONTROLL_FAILED;

	std::string url = "/gs-robot/real_time_data/position" ;
	append_url_head(url);

	//LOGI("gaussian get_current_point get url %s", url.c_str());
	std::string out = imp_get(url);
	std::string data;
	if (!simple_msg(out, data))
	{
		if (data.compare("LOCALIZATION_NOT_READY") == 0)
			return 5006;
		return CONTROLL_FAILED;
	}

	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("angle") && json_object["angle"].isDouble())
		{
			point.angle = json_object["angle"].asDouble();
		}
		if (json_object.isMember("gridPosition") && json_object["gridPosition"].isObject())
		{
			Json::Value   json_gridPosition = json_object["gridPosition"];
			if (json_gridPosition.isMember("x") && json_gridPosition["x"].isInt())
			{
				point.x = json_gridPosition["x"].asInt();
			}
			if (json_gridPosition.isMember("y") && json_gridPosition["y"].isInt())
			{
				point.y = json_gridPosition["y"].asInt();
			}
		}
		if (json_object.isMember("worldPosition") && json_object["worldPosition"].isObject())
		{
			Json::Value   json_worldPosition = json_object["worldPosition"];
			if (json_worldPosition.isMember("orientation") && json_worldPosition["orientation"].isObject())
			{
				Json::Value   json_oriPosition = json_worldPosition["orientation"];
				if (json_oriPosition.isMember("w") && json_oriPosition["w"].isDouble())
				{
					point.world.w = json_oriPosition["w"].asDouble();
				}
				if (json_oriPosition.isMember("z") && json_oriPosition["z"].isDouble())
				{
					point.world.z = json_oriPosition["z"].asDouble();
				}
			}
			if (json_worldPosition.isMember("position") && json_worldPosition["position"].isObject())
			{
				Json::Value   json_oriPosition = json_worldPosition["position"];
				if (json_oriPosition.isMember("x") && json_oriPosition["x"].isDouble())
				{
					point.world.x = json_oriPosition["x"].asDouble();
				}
				if (json_oriPosition.isMember("y") && json_oriPosition["y"].isDouble())
				{
					point.world.y = json_oriPosition["y"].asDouble();
				}
			}
			
		}
	
	}
	else
		return CONTROLL_FAILED;

	return CONTROLL_OK;
}

int robot_controller_gaussian::get_twopoint_navigate_path(std::string mapname, grid_point_data beginpoint, grid_point_data endpoint, navigation_path_data &path)
{
	std::string url = "/gs-robot/data/navigation_path";
	append_url_head(url);

	std::string json;

	Json::Value   json_object;
	Json::Value   json_point;
	Json::Value   json_grid;
	Json::FastWriter json_writer;
	json_grid["x"] = Json::Value(beginpoint.x);
	json_grid["y"] = Json::Value(beginpoint.y);
	json_point["angle"] = Json::Value(beginpoint.angle);
	json_point["gridPosition"] = json_grid;
	json_object["start"] = json_point;

	json_grid.clear();
	json_point.clear();

	json_grid["x"] = Json::Value(endpoint.x);
	json_grid["y"] = Json::Value(endpoint.y);
	json_point["angle"] = Json::Value(endpoint.angle);
	json_point["gridPosition"] = json_grid;
	json_object["end"] = json_point;
	json_object["mapName"] = Json::Value(mapname);
	
	json = json_writer.write(json_object);

	std::string out = imp_post(json,url);
	std::string data;
	if (!simple_msg(out, data))
		return CONTROLL_FAILED;
	json_object.clear();
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("data") && json_object["data"].isObject())
		{
			Json::Value   json_data = json_object["data"];
			if (json_data.isMember("gridPhits") && json_data["gridPhits"].isArray())
			{
				Json::Value json_grid = json_data["gridPhits"];
				for (int i = 0; i < json_grid.size(); i++)
				{
					if (json_grid[i].isObject())
					{
						grid_point_data temp_pos;
						if (json_grid[i].isMember("x") && json_grid[i]["x"].isInt())
							temp_pos.x = json_grid[i]["x"].asInt();
						if (json_grid[i].isMember("y") && json_grid[i]["y"].isInt())
							temp_pos.y = json_grid[i]["y"].asInt();
						path.path_pos_list.push_back(temp_pos);
					}
				}
			}
			if (json_data.isMember("mapInfo") && json_data["mapInfo"].isObject())
			{
				Json::Value   json_map_one = json_data["mapInfo"];
				if (json_map_one.isMember("gridHeight") && json_map_one["gridHeight"].isInt())
					path.map_info.map_h = json_map_one["gridHeight"].asInt();
				if (json_map_one.isMember("gridWidth") && json_map_one["gridWidth"].isInt())
					path.map_info.map_w = json_map_one["gridWidth"].asInt();
				if (json_map_one.isMember("originX") && json_map_one["originX"].isDouble())
					path.map_info.originX = json_map_one["originX"].asDouble();
				if (json_map_one.isMember("originY") && json_map_one["originY"].isDouble())
					path.map_info.originY = json_map_one["originY"].asDouble();
				if (json_map_one.isMember("resolution") && json_map_one["resolution"].isDouble())
					path.map_info.rate = json_map_one["resolution"].asDouble();
			}
		}
	}
	else
		return CONTROLL_FAILED;
	return CONTROLL_OK;
}

int robot_controller_gaussian::get_navigate_path(navigation_path_data &path)
{
	std::string url = "/gs-robot/real_time_data/navigation_path";
	append_url_head(url);
	std::string out = imp_get(url);
	std::string data;
	if (!simple_msg(out, data))
		return CONTROLL_FAILED;
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("data") && json_object["data"].isObject())
		{
			Json::Value   json_data = json_object["data"];
			if (json_data.isMember("gridPhits") && json_data["gridPhits"].isArray())
			{
				Json::Value json_grid = json_data["gridPhits"];
				for (int i = 0; i < json_grid.size(); i++)
				{
					if (json_grid[i].isObject())
					{
						grid_point_data temp_pos;
						if (json_grid[i].isMember("x") && json_grid[i]["x"].isInt())
							temp_pos.x = json_grid[i]["x"].asInt();
						if (json_grid.isMember("y") && json_grid["y"].isInt())
							temp_pos.y = json_grid["y"].asInt();
						path.path_pos_list.push_back(temp_pos);
					}
				}
			}
			if (json_data.isMember("mapInfo") && json_data["mapInfo"].isObject())
			{
				Json::Value   json_map_one = json_data["mapInfo"];
				if (json_map_one.isMember("gridHeight") && json_map_one["gridHeight"].isInt())
					path.map_info.map_h = json_map_one["gridHeight"].asInt();
				if (json_map_one.isMember("gridWidth") && json_map_one["gridWidth"].isInt())
					path.map_info.map_w = json_map_one["gridWidth"].asInt();
				if (json_map_one.isMember("originX") && json_map_one["originX"].isDouble())
					path.map_info.originX = json_map_one["originX"].asDouble();
				if (json_map_one.isMember("originY") && json_map_one["originY"].isDouble())
					path.map_info.originY = json_map_one["originY"].asDouble();
				if (json_map_one.isMember("resolution") && json_map_one["resolution"].isDouble())
					path.map_info.rate = json_map_one["resolution"].asDouble();
			}
		}
	}
	else
		return CONTROLL_FAILED;
	return CONTROLL_OK;
}

int robot_controller_gaussian::navigate_to(grid_point_data& spd)
{
	if (gaussian_ws_cache.in_stop)
	{
		LOGW("on stop cant navigate!!!");
		return CONTROLL_FAILED;
	}

	//if (check_init() != 0)return CONTROLL_FAILED;
	std::string url = "/gs-robot/cmd/navigate";
	append_url_head(url);
	std::string out;
	std::vector<unsigned char > post_data;
	std::string json;

	Json::Value   json_object;
	Json::Value   json_point;
	Json::Value   json_grid;
	Json::FastWriter json_writer;
	json_grid["x"] = Json::Value(spd.x);
	json_grid["y"] = Json::Value(spd.y);
	json_point["gridPosition"] = json_grid;
	json_point["angle"] = Json::Value((int)spd.angle);
	json_object["destination"] = json_point;
	json = json_writer.write(json_object);

	LOGI("gaussian navigate_to post url %s,data :\n%s", url.c_str(), json.c_str());

	out= imp_post(json, url);

	if (!simple_msg(out, out)) 
		return CONTROLL_FAILED;

	return CONTROLL_OK;
}

int robot_controller_gaussian::navigate_to(std::string name)
{
	if (gaussian_ws_cache.in_stop)
	{
		LOGW("on stop cant navigate!!!");
		return CONTROLL_FAILED;
	}
	
	std::string mapname;
	std::vector<special_point_data> point_list;
	if (get_current_map_name(mapname)) return 5012;


	common_urlencode(&mapname, &mapname); common_urlencode(&name, &name);
	std::string url = "/gs-robot/cmd/position/navigate?map_name=" + mapname + "&position_name=" + name;
	append_url_head(url);

	LOGI("gaussian navigate_to get url %s", url.c_str());
	std::string out = imp_get(url);

	if (!simple_msg(out, out))
		return 5009;
	return CONTROLL_OK;
}

std::string robot_controller_gaussian::imp_post(std::string json, std::string url,int t)
{
	VEC_BYTE vec_str, post_value;
	std::string out;
	post_value.insert(post_value.end(), json.begin(), json.end());

	if (!robot_http_pool::getSingle()->postRequest(url, post_value, vec_str, t))
		return out;
	out.insert(out.end(), vec_str.begin(), vec_str.end());
	return out;
}

int robot_controller_gaussian::resume_motor(int type)
{
	std::string mapname,number;
	common_itoa_x(type, &number);
	std::string url = "/gs-robot/cmd/clear_mcu_error?error_id=" + number;
	append_url_head(url);
	//LOGI("gaussian operate_motor get url %s", url.c_str());
	std::string out = imp_get(url);

	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	return CONTROLL_OK;
}

int robot_controller_gaussian::motor(bool b)
{
	std::string mapname, number;
	int type = 1;
	if (!b)type = 0;
	common_itoa_x(type, &number);
	std::string url = "/gs-robot/cmd/operate_motor_driver?type=" + number;
	append_url_head(url);
	LOGI("gaussian operate_motor get url %s", url.c_str());
	std::string out = imp_get(url);

	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	return CONTROLL_OK;
}

int robot_controller_gaussian::detect_move_data(std::vector<move_object_detect>& object_detect)
{
	object_detect.clear();

	//std::string url = "/gs-robot/real_time_data/mobile_data";
	std::string url = "/gs-robot/real_time_data/non_map_data";
	append_url_head(url);
	std::string out = imp_get(url);

	std::string data;
	if (!simple_msg(out, data))
		return CONTROLL_FAILED;
	Json::Value   json_object;
	Json::Reader  json_reader;
	std::string key_string = "non_map_value"; // mobile_value
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("data") && json_object["data"].isObject())
		{
			Json::Value  jsondata = json_object["data"];
			if (jsondata.isMember(key_string) && jsondata[key_string].isArray())
			{
				Json::Value   mobile_data = jsondata[key_string];

				for (int i = 0; i < mobile_data.size(); i++)
				{
					if (mobile_data[i].isObject())
					{
						move_object_detect move_data;
						Json::Value mobile_one = mobile_data[i];
						if (mobile_one.isMember("distance") && mobile_one["distance"].isDouble())
						{
							move_data.distance = mobile_one["distance"].asDouble();
						}
						if (mobile_one.isMember("angle") && mobile_one["angle"].isDouble())
						{
							move_data.object_angle = mobile_one["angle"].asDouble();
						}
						if (mobile_one.isMember("objectAngle") && mobile_one["objectAngle"].isDouble())
						{
							move_data.object_angle = mobile_one["objectAngle"].asDouble();
						}
						if (mobile_one.isMember("objectSpeed") && mobile_one["objectSpeed"].isDouble())
						{
							move_data.object_speed = mobile_one["objectSpeed"].asDouble();
						}
						if (mobile_one.isMember("possibility") && mobile_one["possibility"].isDouble())
						{
							move_data.possibility = mobile_one["possibility"].asDouble();
						}
						if (mobile_one.isMember("is_human") && mobile_one["is_human"].isInt())
						{
							move_data.is_human = mobile_one["is_human"].asInt();
						}

						if (mobile_one.isMember("gridPosition") && mobile_one["gridPosition"].isObject())
						{
							Json::Value json_grid = mobile_one["gridPosition"];
							if (json_grid.isMember("x") && json_grid["x"].isInt())
								move_data.object_position.x = json_grid["x"].asInt();
							if (json_grid.isMember("y") && json_grid["y"].isInt())
								move_data.object_position.y = json_grid["y"].asInt();
						}
						object_detect.push_back(move_data);
					}

				}
			}


		}
	}
	else
		return CONTROLL_FAILED;

	return CONTROLL_OK;
}

int robot_controller_gaussian::start_charge()
{
	std::string url = "/gs-robot/cmd/start_charge";
	return simple_get(url);
}

int robot_controller_gaussian::stop_charge()
{
	std::string url = "/gs-robot/cmd/stop_charge";
	return simple_get(url);
}

int robot_controller_gaussian::get_charge_state(int &state)
{
	std::string url = "/gs-robot/data/charge_status";
	append_url_head(url);
	std::string out = imp_get(url);
	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	state = 0;
	if (out.compare("CHARGER_ON") == 0)
		state = 1;
	if (out.compare("CHARGER_TOUCHED") == 0)
		state = 2;
	if (out.compare("CHARGER_OFF") == 0)
		state = 3;
	return CONTROLL_OK;
}

int robot_controller_gaussian::simple_get(std::string url)
{
	append_url_head(url);
	std::string out = imp_get(url);
	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	return CONTROLL_OK;
}

std::string robot_controller_gaussian::imp_get(std::string url,int t)
{
	std::string out;
	VEC_BYTE vec_str;
	if (!robot_http_pool::getSingle()->getRequest(url, vec_str, t))
		return out;

	out.insert(out.end(), vec_str.begin(), vec_str.end());
	return out;
}

int robot_controller_gaussian::navigate_pause()
{
	std::string url = "/gs-robot/cmd/pause_navigate";
	return simple_get(url);
}

int robot_controller_gaussian::navigate_cancel()
{
	std::string url = "/gs-robot/cmd/cancel_navigate";
	g_navi_state = false;
	return simple_get(url);
}

int robot_controller_gaussian::navigate_resume()
{
	std::string url = "/gs-robot/cmd/resume_navigate";
	return simple_get(url);
}

int robot_controller_gaussian::get_special_point_list(std::string mapname, std::vector<special_point_data>&point_list, int filtertype /*= -1*/)
{
	point_list.clear();
	std::string map_name = mapname;
	if (map_name.empty())
	{
		if (get_current_map_name(map_name))
			return -1;
	}

	std::string url = "/gs-robot/data/positions?map_name=";
	common_urlencode(&map_name, &map_name);
	url.append(map_name);
	if (filtertype >= 0)
	{
		url.append("&type=");
		std::string number;
		common_itoa_x(filtertype, &number);
		url.append(number);
	}
	append_url_head(url);
	std::string out = imp_get(url);
	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("data") && json_object["data"].isArray())
		{
			Json::Value   json_data = json_object["data"];
			for (int i = 0; i < json_data.size(); i++)
			{
				special_point_data temp_pos;
				if (json_data[i].isObject())
				{
					if (json_data[i].isMember("type") && json_data[i]["type"].isInt())
					{
						int type = json_data[i]["type"].asInt();
						if (filtertype >= 0 && type != filtertype)continue;
						if (json_data[i].isMember("angle") && json_data[i]["angle"].isDouble())
						{
							temp_pos.postion.angle = json_data[i]["angle"].asDouble();
						}
						if (json_data[i].isMember("gridX") && json_data[i]["gridX"].isInt())
						{
							temp_pos.postion.x = json_data[i]["gridX"].asInt();
						}
						if (json_data[i].isMember("gridY") && json_data[i]["gridY"].isInt())
						{
							temp_pos.postion.y = json_data[i]["gridY"].asInt();
						}
						if (json_data[i].isMember("name") && json_data[i]["name"].isString())
						{
							temp_pos.name = json_data[i]["name"].asString();
						}
						temp_pos.type = type;
						point_list.push_back(temp_pos);
					}
				}
			}
		}
		else
			return CONTROLL_FAILED;
	}
	else
		return CONTROLL_FAILED;
	return CONTROLL_OK;
}


int robot_controller_gaussian::remember_point(std::string mapname, special_point_data&spd)
{
	std::string map_name = mapname;
	if (map_name.empty())
	{
		if (get_current_map_name(map_name))
			return -1;
	}

	std::string url = "/gs-robot/cmd/position/add_position";
	append_url_head(url);
	std::string json;
	Json::Value   json_object;
	Json::FastWriter json_writer;
	json_object["angle"] = Json::Value((int)spd.postion.angle);
	json_object["gridX"] = Json::Value((int)spd.postion.x);
	json_object["gridY"] = Json::Value((int)spd.postion.y);
	json_object["mapName"] = Json::Value(map_name);
	json_object["name"] = Json::Value(spd.name);
	json_object["type"] = Json::Value(spd.type);
	json = json_writer.write(json_object);
	std::string out = imp_post(json, url);
	std::string data;
	if (!simple_msg(out, data))
		return CONTROLL_FAILED;
	return CONTROLL_OK;
}


int robot_controller_gaussian::delete_point(std::string mapname, std::string name)
{
	std::string map_name = mapname;
	if (map_name.empty())
	{
		if (get_current_map_name(map_name))
			return -1;
	}
	common_urlencode(&map_name, &map_name);
	common_urlencode(&name, &name);
	std::string url = "/gs-robot/cmd/delete_position?map_name=" + map_name + "&position_name=" + name;
	return simple_get(url);
}

int robot_controller_gaussian::get_map_picture(std::string mapname, std::vector<char> &mapdata)
{
	mapdata.clear();
	common_urlencode(&mapname, &mapname);
	std::string url = "/gs-robot/data/map_png?map_name=";
	url.append(mapname);
	append_url_head(url);

	VEC_BYTE vec_str;

	if (!robot_http_pool::getSingle()->getRequest(url, vec_str))
	//if (!common_http_get(m_http, url.c_str(), &vec_str))
		return CONTROLL_FAILED;
	mapdata.insert(mapdata.end(), vec_str.begin(), vec_str.end());

	if (mapdata.size())
		return CONTROLL_OK;
	return CONTROLL_FAILED;
}

int robot_controller_gaussian::get_map_list(std::vector<map_data> &list)
{
	list.clear();
	std::string url = "/gs-robot/data/maps";
	append_url_head(url);
	//LOGI("gaussian maplist get url %s", url.c_str());

	std::string out = imp_get(url);
	std::string data;
	if (!simple_msg(out, data))
		return  CONTROLL_FAILED;
	
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("data") && json_object["data"].isArray())
		{
			for (int i = 0; i < json_object["data"].size();i++)
			{
				if (json_object["data"][i].isObject())
				{
					map_data  temp_map;
					Json::Value   json_map_list = json_object["data"][i];
					if (json_map_list.isMember("name") && json_map_list["name"].isString())
						temp_map.map_name = json_map_list["name"].asString();
					if (json_map_list.isMember("mapInfo") && json_map_list["mapInfo"].isObject())
					{
						Json::Value   json_map_one = json_map_list["mapInfo"];
						if (json_map_one.isMember("gridHeight") && json_map_one["gridHeight"].isInt())
							temp_map.map_h = json_map_one["gridHeight"].asInt();
						if (json_map_one.isMember("gridWidth") && json_map_one["gridWidth"].isInt())
							temp_map.map_w = json_map_one["gridWidth"].asInt();
						if (json_map_one.isMember("originX") && json_map_one["originX"].isDouble())
							temp_map.originX = json_map_one["originX"].asDouble();
						if (json_map_one.isMember("originY") && json_map_one["originY"].isDouble())
							temp_map.originY = json_map_one["originY"].asDouble();
						if (json_map_one.isMember("resolution") && json_map_one["resolution"].isDouble())
							temp_map.rate = json_map_one["resolution"].asDouble();
					}
					list.push_back(temp_map);
				}
			}

		}

	}
	else
		return CONTROLL_FAILED;
	return CONTROLL_OK;
}

int robot_controller_gaussian::load_map(std::string name)
{
	common_urlencode(&name, &name);
	std::string url = "/gs-robot/cmd/use_map?map_name=" + name;
	append_url_head(url);
	//LOGI("gaussian load_map get url %s",url.c_str());
	std::string out = imp_get(url);
	return simple_msg(out,out) ? CONTROLL_OK : CONTROLL_FAILED;
}

bool robot_controller_gaussian::simple_msg(std::string &out,std::string &data_compare)
{
	if (out.size() == 0)
		return false;
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		std::string errorCode;
		if (json_object.isMember("errorCode") && json_object["errorCode"].isString())
		{
			errorCode = json_object["errorCode"].asString();
			if (errorCode.size() != 0)
				LOGE("errorCode :%s", errorCode.c_str());

		}

		if (json_object.isMember("msg") && json_object["msg"].isString())
		{
			std::string msg = json_object["msg"].asString();
			if (msg.compare("successed") != 0)
			{
				LOGE("errormsg :%s", msg.c_str());
				data_compare = errorCode;
				return false;
			}
				
		}

		if (json_object.isMember("successed") && json_object["successed"].isBool())
		{
			if (!json_object["successed"].asBool())
			{
				data_compare = errorCode;
				return false;
			}
				
		}
		if (json_object.isMember("data") && json_object["data"].isString())
		{
			std::string msg = json_object["data"].asString();
			data_compare = msg;
		}

	}
	else
	{
		LOGE("gaussian json data error %s", out.c_str());
		return false;
	}
	return true;
}

int robot_controller_gaussian::check_init_finish()
{
	std::string url = "/gs-robot/cmd/is_initialize_finished";
	append_url_head(url);
	std::string out = imp_get(url);
	std::string data;
	if (out.size() == 0)
		return 2;
	if (!simple_msg(out, data))
		return -1;
	if (data.compare("true"))
		return 1;

	return CONTROLL_OK;
}

int robot_controller_gaussian::check_rotate_finish()
{
	std::string url = "/gs-robot/cmd/is_rotate_finished";
	append_url_head(url);
	std::string out = imp_get(url);
	std::string data;
	if (!simple_msg(out, data))
		return -1;
	if (data.compare("true"))
		return 1;
	return CONTROLL_OK;
}

int robot_controller_gaussian::initialize_robot(std::string mapname,std::string name)
{
	if (load_map(mapname) != CONTROLL_OK)return CONTROLL_FAILED;
	common_urlencode(&name, &name);
	common_urlencode(&mapname, &mapname);
	std::string url = "/gs-robot/cmd/initialize_directly?map_name="+mapname+"&init_point_name=" + name;
	append_url_head(url);
	//LOGW("gaussian initialize_robot get url %s", url.c_str());
	std::string out = imp_get(url,45);
	if (!simple_msg(out,out)) return CONTROLL_FAILED;
	//init();
	return CONTROLL_OK;
//	return wait_init();
}

int robot_controller_gaussian::initialize_robot(special_point_data spd)
{
	if (load_map(spd.name) != CONTROLL_OK)return CONTROLL_FAILED;

	std::string url;
	std::vector<unsigned char > post_data;
	std::string json;
	std::string out;
	if (spd.type == 2)
	{
		std::string mapname = spd.name;
		common_urlencode(&mapname, &mapname);
		url = "/gs-robot/cmd/initialize_global?map_name=" + mapname;
		append_url_head(url);
		out = imp_get(url, 45);
	}
	else
	{
		if (spd.type == 1)
			url = "/gs-robot/cmd/initialize_customized_directly";
		else
			url = "/gs-robot/cmd/initialize_customized";

		Json::Value   json_object;
		Json::Value   json_point;
		Json::Value   json_grid;
		Json::FastWriter json_writer;
		json_grid["x"] = Json::Value(spd.postion.x);
		json_grid["y"] = Json::Value(spd.postion.y);
		json_point["gridPosition"] = json_grid;
		json_point["angle"] = Json::Value(spd.postion.angle);
		json_object["mapName"] = Json::Value(spd.name);
		json_object["point"] = json_point;
		json = json_writer.write(json_object);
		std::string temp;
		common_utf8_2gbk(&json, &temp);
		LOGW("gaussian initialize_robot post url %s,data %s", url.c_str(), temp.c_str());
		append_url_head(url);
		out = imp_post(json, url, 45);

	}

	if (!simple_msg(out, out)) return CONTROLL_FAILED;

	return CONTROLL_OK;
//	return wait_init();
}

int robot_controller_gaussian::initialize_cancel_robot()
{
	std::string url = "/gs-robot/cmd/stop_initialize";
	return simple_get(url);
}

std::string robot_controller_gaussian::get_health_report()
{
	LOCK_HELPER_AUTO_HANDLER(m_ml);
	std::string temp = health;
	if (temp.empty())
	{
		Json::FastWriter json_writer;
		Json::Value json_object;
		json_object["msg_type"] = Json::Value("NAVIGATE_HEALTH_RESPONSE");
		json_object["error"] = Json::Value(1);
		temp = json_writer.write(json_object);
	}
	return temp;
}

int robot_controller_gaussian::wait_init()
{
	Timer_helper  t;
	int rt = 0;
	do
	{
		rt = check_init_finish();
		if (rt == 0)
			break;
		if (rt == -1||rt==2)
		{
			LOGE("gaussian initialize failed");
			break;
		}
		common_thread_sleep(500);
		if (t.isreach(120000))
		{
			t.update();
			LOGW("timeout wait for gaussian init failed");
			return 10090;
		}
	} while (1);

	return  rt ? CONTROLL_FAILED : CONTROLL_OK;
}

int robot_controller_gaussian::get_current_map_name(std::string &mapname)
{
	mapname.clear();
	//if (check_init()!= 0)return false;//TODO测试未初始化情况
	std::string url = "/gs-robot/real_time_data/current_initialize_status";
	append_url_head(url);
	std::string out = imp_get(url);
	if (out.size() == 0)
		return 2;
	Json::Value   json_object;
	Json::Reader  json_reader(Json::Features::strictMode());
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("currentMap") && json_object["currentMap"].isString())
		{
			std::string msg = json_object["currentMap"].asString();
			if (msg.size() != 0)
			{
				mapname = msg;
				return 0;
			}
				
		}
	}
	else
	{
		LOGE("gaussian json error data %s", out.c_str());
	}
	return 1;
}

int robot_controller_gaussian::get_current_speed(double &ls, double &as)
{
	std::string url = "/gs-robot/real_time_data/cmd_vel";
	append_url_head(url);
	std::string out = imp_get(url);
	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	ls = 0;
	as = 0;
	Json::Value   json_object;
	Json::Reader  json_reader;
	int ok = 1;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("data") && json_object["data"].isObject())
		{
			Json::Value   json_data = json_object["data"];
			if (json_data.isMember("angular") && json_data["angular"].isObject())
			{
				if (json_data["angular"].isMember("y") && json_data["angular"]["y"].isDouble())
					as = json_data["angular"]["y"].asDouble();
				if (as == 0)
				{
					if (json_data["angular"].isMember("z") && json_data["angular"]["z"].isDouble())
						as = json_data["angular"]["z"].asDouble();
				}
			}
			if (json_data.isMember("linear") && json_data["linear"].isObject())
			{
				if (json_data["linear"].isMember("x") && json_data["linear"]["x"].isDouble())
					ls = json_data["linear"]["x"].asDouble();
				ok = 0;
			}
		}
	}
	else
	{
		LOGE("gaussian json error data %s", out.c_str());
	}
	return ok;
}

int robot_controller_gaussian::get_protector(std::vector<int >& frame)
{
	std::string url = "/gs-robot/real_time_data/protector";
	append_url_head(url);
	std::string out = imp_get(url);
	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	frame.clear();
	for (int i = 0; i < out.size();i++)
		frame.push_back(out[i]);
	return frame.size() == 0 ? CONTROLL_FAILED : CONTROLL_OK;
}

int robot_controller_gaussian::scan_map(std::string name)
{
	std::string url = "/gs-robot/cmd/start_scan_map?map_name=";
	common_urlencode(&name, &name);
	url.append(name);
	append_url_head(url);
	std::string out = imp_get(url);
	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	return  CONTROLL_OK;
}

int robot_controller_gaussian::save_map()
{
	std::string url = "/gs-robot/cmd/stop_scan_map";
	append_url_head(url);
	std::string out = imp_get(url);
	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	return  CONTROLL_OK;
}

int robot_controller_gaussian::cancel_scan()
{
	std::string url = "/gs-robot/cmd/cancel_scan_map";
	append_url_head(url);
	std::string out = imp_get(url);
	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	return  CONTROLL_OK;
}

int robot_controller_gaussian::get_scan_map(std::vector<char> &mapdata)
{
	mapdata.clear();
	std::string url = "/gs-robot/real_time_data/scan_map_png";
	append_url_head(url);

	VEC_BYTE vec_str;
	if (!robot_http_pool::getSingle()->getRequest(url, vec_str))
		//if (!common_http_get(m_http, url.c_str(), &vec_str))
		return CONTROLL_FAILED;
	mapdata.insert(mapdata.end(), vec_str.begin(), vec_str.end());

	if (mapdata.size())
		return CONTROLL_OK;
	return CONTROLL_FAILED;
}

int robot_controller_gaussian::del_map(std::string name)
{
	std::string url = "/gs-robot/cmd/delete_map?map_name=";
	common_urlencode(&name, &name);
	url.append(name);
	append_url_head(url);
	std::string out = imp_get(url);
	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	return  CONTROLL_OK;
}

int robot_controller_gaussian::get_virtual_wall(std::string name, std::string &data)
{
	std::string url = "/gs-robot/data/virtual_obstacles?map_name=";
	common_urlencode(&name, &name);
	url.append(name);
	append_url_head(url);

	std::string out = imp_get(url);
	std::string resdata;
	if (!simple_msg(out, resdata))
		return CONTROLL_FAILED;

	
	Json::Value   json_object;
	Json::Reader  json_reader;
	Json::FastWriter json_writer;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("data") && json_object["data"].isObject())
		{
			Json::Value json_data = json_object["data"];
			Json::Value json_data_carpets, json_data_decelerations, json_data_obstacles;
			Json::Value temp;
			if (json_data.isMember("carpets") && json_data["carpets"].isObject())
				json_data_carpets = json_data["carpets"];

			if (json_data.isMember("decelerations") && json_data["decelerations"].isObject())
				json_data_decelerations = json_data["decelerations"];

			if (json_data.isMember("obstacles") && json_data["obstacles"].isObject())
				json_data_obstacles = json_data["obstacles"];

			temp["carpets"] = json_data_carpets;
			temp["decelerations"] = json_data_decelerations;
			temp["obstacles"] = json_data_obstacles;

			data = json_writer.write(temp);
			return CONTROLL_OK;
		}

	}
	else
		;
	return CONTROLL_FAILED;
}

int robot_controller_gaussian::update_virtual_wall(std::string name, std::string json)
{
	std::string url = "/gs-robot/cmd/update_virtual_obstacles";
	append_url_head(url);
	std::string out = imp_post(json, url);
	std::string data;
	if (!simple_msg(out, data))
		return CONTROLL_FAILED;
	return CONTROLL_OK;
}

std::string robot_controller_gaussian::get_slopes(std::string name)
{
	return "";
}

int robot_controller_gaussian::update_slopes(std::string name, std::string json)
{
	return 0;
}

int robot_controller_gaussian::move(double lspeed, double aspeed)
{
	if (gaussian_ws_cache.in_stop)
	{
		//LOGD("on stop cant move!!!");
		return CONTROLL_FAILED;
	}
	std::string url = "/gs-robot/cmd/move";
	append_url_head(url);

	std::string json;

	Json::Value   json_object;
	Json::Value   json_speed;
	Json::FastWriter json_writer;
	json_speed["linearSpeed"] = Json::Value(lspeed);
	json_speed["angularSpeed"] = Json::Value(aspeed);
	json_object["speed"] = json_speed;
	json = json_writer.write(json_object);

	std::string out = imp_post(json, url);
	if (!simple_msg(out, out))
		return CONTROLL_FAILED;

	return CONTROLL_OK;
}

int robot_controller_gaussian::rotate(double angle, double aspeed)
{
	if (gaussian_ws_cache.in_stop)
	{
		LOGD("on stop cant rotate!!!");
		return CONTROLL_FAILED;
	}
	std::string url = "/gs-robot/cmd/rotate";
	append_url_head(url);

	std::string json;
	Json::Value   json_object;
	Json::FastWriter json_writer;
	json_object["rotateAngle"] = Json::Value((int)angle);
	json_object["rotateSpeed"] = Json::Value(aspeed);
	json = json_writer.write(json_object);

	std::string out = imp_post(json, url);
	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	return CONTROLL_OK;

}

int robot_controller_gaussian::moveto(double lspeed, double distance)
{
	if (gaussian_ws_cache.in_stop)
	{
		LOGD("on stop cant moveto!!!");
		return CONTROLL_FAILED;
	}
	char buffer[1024] = { 0 };
	sprintf_s(buffer, 1024, "%0.4f", distance);
	std::string url = "/gs-robot/cmd/move_to?distance=";

	std::string tmp = buffer;
	url.append(tmp);
	url.append("&speed=");
	sprintf_s(buffer, 1024, "%0.4f", lspeed);
	tmp = buffer;
	url.append(tmp);

	if (simple_get(url))
		return CONTROLL_FAILED;

	return CONTROLL_OK;
}

int robot_controller_gaussian::check_moveto_finish()
{
	std::string url = "/gs-robot/cmd/is_move_to_finished";
	append_url_head(url);
	std::string out = imp_get(url);
	std::string data;
	if (!simple_msg(out, data))
		return -1;
	if (data.compare("true"))
		return 1;
	return CONTROLL_OK;
}

int robot_controller_gaussian::stop_moveto()
{
	std::string url = "/gs-robot/cmd/stop_move_to";
	return simple_get(url);
}

int robot_controller_gaussian::stop_rotate()
{
	std::string url = "/gs-robot/cmd/stop_rotate";
	return simple_get(url);
}

std::string robot_controller_gaussian::get_version()
{	
	std::string url = "/gs-robot/info/version";
	append_url_head(url);

	std::string out = imp_get( url);
	if (!simple_msg(out, out))
		return "";
	return out;
}

int robot_controller_gaussian::set_navigation_speed(int level)
{
	std::string num;
	std::string url = "/gs-robot/cmd/set_navigation_speed_level?level=";
	common_itoa_x(level, &num);
	url.append(num);
	return simple_get(url);
}

int robot_controller_gaussian::shutdown()
{
	std::string url = "/gs-robot/cmd/power_off";
	return simple_get(url);
}

int robot_controller_gaussian::soft_reset_reboot()
{
	std::string url = "/gs-robot/cmd/reset_robot_setting";
	return simple_get(url);
}
int robot_controller_gaussian::start_greeter(std::string mapname, std::string name)
{
	common_urlencode(&name, &name); common_urlencode(&mapname, &mapname);
	std::string url = "/gs-robot/cmd/start_greeter?map_name=" + mapname + "&greeter_name=" + name;
	return simple_get(url);
}

int robot_controller_gaussian::stop_greeter()
{
	std::string url = "/gs-robot/cmd/stop_greeter";
	return simple_get(url);
}

int robot_controller_gaussian::pause_greeter()
{
	std::string url = "/gs-robot/cmd/pause_greeter";
	return simple_get(url);
}

int robot_controller_gaussian::resume_greeter()
{
	std::string url = "/gs-robot/cmd/resume_greeter";
	return simple_get(url);
}

int robot_controller_gaussian::start_queue(TaskQueue &taskQueue)
{
	std::string url = "/gs-robot/cmd/start_task_queue";
	append_url_head(url);

	std::string json;

	Json::Value   json_object;
	Json::FastWriter json_writer;
	json_object["name"] = taskQueue.queuename;
	json_object["loop"] = taskQueue.loop;
	json_object["loop_count"] = taskQueue.loop_count;
	json_object["map_name"] = taskQueue.mapname;
	for (int i = 0; i < taskQueue.queueinfo.size();i++)
	{
		Json::Value   task;
		task["name"] = taskQueue.queueinfo[i].taskname;
		if (taskQueue.queueinfo[i].taskname.compare("PlayPathTask") == 0)
		{
			Json::Value   taskparam;
			taskparam["map_name"] = taskQueue.queueinfo[i].mapname;
			taskparam["path_name"] = taskQueue.queueinfo[i].path_name;
			task["start_param"] = taskparam;
		}
		if (taskQueue.queueinfo[i].taskname.compare("PlayGraphPathTask") == 0)
		{
			Json::Value   taskparam;
			taskparam["map_name"] = taskQueue.queueinfo[i].mapname;
			taskparam["graph_name"] = taskQueue.queueinfo[i].graph_name;
			taskparam["graph_path_name"] = taskQueue.queueinfo[i].graph_path_name;
			task["start_param"] = taskparam;
		}
		if (taskQueue.queueinfo[i].taskname.compare("PlayGraphPathGroupTask") == 0)
		{
			Json::Value   taskparam;
			taskparam["map_name"] = taskQueue.queueinfo[i].mapname;
			taskparam["graph_name"] = taskQueue.queueinfo[i].graph_name;
			taskparam["graph_path_group_name"] = taskQueue.queueinfo[i].graph_path_group_name;
			task["start_param"] = taskparam;
		}
		if (!taskQueue.queueinfo[i].pointname.empty() && taskQueue.queueinfo[i].taskname.compare("NavigationTask") == 0)
		{
			Json::Value   taskparam;
			taskparam["map_name"] = taskQueue.queueinfo[i].mapname;
			taskparam["position_name"] = taskQueue.queueinfo[i].pointname;
			task["start_param"] = taskparam;
		}
		if (taskQueue.queueinfo[i].position.x != 0 && taskQueue.queueinfo[i].taskname.compare("NavigationTask") == 0)
		{
			Json::Value   taskparam, destination, gridPosition;
			taskparam["map_name"] = taskQueue.queueinfo[i].mapname;
			gridPosition["x"] = taskQueue.queueinfo[i].position.x;
			gridPosition["y"] = taskQueue.queueinfo[i].position.y;
			destination["angle"] = (int)taskQueue.queueinfo[i].position.angle;
			destination["gridPosition"] = gridPosition;
			taskparam["destination"] = destination;
			task["start_param"] = taskparam;
		}
		json_object["tasks"].append(task);
	}
	json = json_writer.write(json_object);

	std::string out = imp_post(json, url);
	std::string data;
	if (!simple_msg(out, out))
		return CONTROLL_FAILED;
	return CONTROLL_OK;

}

int robot_controller_gaussian::stop_queue()
{
	std::string url = "/gs-robot/cmd/stop_task_queue";
	//std::string url = "/gs-robot/data/task_queues?map_name=FerlyMap";
	return simple_get(url);
}

int robot_controller_gaussian::pause_queue()
{
	std::string url = "/gs-robot/cmd/pause_task_queue";
	return simple_get(url);
}

int robot_controller_gaussian::resume_queue()
{
	std::string url = "/gs-robot/cmd/resume_task_queue";
	return simple_get(url);
}

int robot_controller_gaussian::isfinish_queue(bool &st)
{
	std::string url = "/gs-robot/cmd/is_task_queue_finished";
	append_url_head(url);
	std::string temp;
	std::string out = imp_get(url);
	if (!simple_msg(out, temp))
		return CONTROLL_FAILED;
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("data") && json_object["data"].isBool())
		{
			st = json_object["data"].asBool();
			return  CONTROLL_OK;
		}
	}
	else
	{
		LOGE("gaussian json error data %s", out.c_str());
	}
	return CONTROLL_FAILED;
}

int robot_controller_gaussian::get_graphlist(std::string mapname,std::string &data)
{
	//std::string url = "/gs-robot/data/paths?map_name=";
	std::string url = "/gs-robot/data/graph_paths?map_name=";
	Json::Value   json_outobject;
	Json::FastWriter json_writer;
	json_outobject["msg_type"] = Json::Value("NAVIGATE_GET_GRAPHPATH_LIST_RESPONSE");
	data.clear();

	std::string mapname_temp;
	if (mapname.empty())
	{
		if (get_current_map_name(mapname_temp) != 0)
		{
			json_outobject["error"] = Json::Value(109);
			data = json_writer.write(json_outobject);
			return CONTROLL_FAILED;
		}
		mapname = mapname_temp;
	}

	url.append(mapname);
	append_url_head(url);
	std::string out = imp_get(url);


	//LOGI("result:%s", out.c_str());
	if (!simple_msg(out, out))
	{
		json_outobject["error"] = Json::Value(CONTROLL_FAILED);
		data = json_writer.write(json_outobject);
		return CONTROLL_FAILED;
	}
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("data") && json_object["data"].isArray())
		{		
			json_outobject["data"] = json_object["data"];
			json_outobject["error"] = Json::Value(0);
			data = json_writer.write(json_outobject);
			return 0;
		}
	}
	else
	{
		LOGE("gaussian json error data %s", out.c_str());
	}

	json_outobject["error"] = Json::Value(CONTROLL_FAILED);
	data = json_writer.write(json_outobject);
	return CONTROLL_FAILED;
}

int robot_controller_gaussian::get_tasklist(std::string mapname,std::string &data)
{
	std::string url = "/gs-robot/data/task_queues?map_name=";

	Json::Value   json_outobject;
	Json::FastWriter json_writer;
	json_outobject["msg_type"] = Json::Value("NAVIGATE_GET_TASK_LIST_RESPONSE");
	data.clear();

	std::string mapname_temp;
	if (mapname.empty())
	{
		if (get_current_map_name(mapname_temp) != 0)
		{
			json_outobject["error"] = Json::Value(109);
			data = json_writer.write(json_outobject);
			return CONTROLL_FAILED;
		}
		mapname = mapname_temp;
	}

	url.append(mapname);
	append_url_head(url);
	std::string out = imp_get(url);

	//LOGI("result:%s", out.c_str());
	if (!simple_msg(out, out))
	{
		json_outobject["error"] = Json::Value(CONTROLL_FAILED);
		data = json_writer.write(json_outobject);
		return CONTROLL_FAILED;
	}
	
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(out.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("data") && json_object["data"].isArray())
		{
			json_outobject["data"] = json_object["data"];
			json_outobject["error"] = Json::Value(0);
			data = json_writer.write(json_outobject);
			return 0;
		}
	}
	else
	{
		LOGE("gaussian json error data %s", out.c_str());
	}

	json_outobject["error"] = Json::Value(CONTROLL_FAILED);
	data = json_writer.write(json_outobject);
	return CONTROLL_FAILED;
}

robot_controller_gaussian::~robot_controller_gaussian()
{
	if (m_ml)common_mutex_threadlock_release(m_ml);
}

void robot_controller_gaussian::append_url_head(std::string &url)
{
	std::string temp;
	temp.append("http://");
	temp.append(m_ip);
	temp.append(":");
	temp.append(m_port);
	url.insert(url.begin(), temp.begin(),temp.end());
}

void gaussian_thread::run()
{
	LOGI("websocket run");
	lws_set_log_level(0, NULL);
	Timer_helper     now, connect_time;


	

	int n = 0;
	while (1) {
		if (m_ws_context == NULL)
		{
			m_ws_info.port = CONTEXT_PORT_NO_LISTEN;
			m_ws_info.protocols = protocols;
			m_ws_info.gid = -1;
			m_ws_info.uid = -1;
			m_ws_info.ssl_cert_filepath = NULL;
			m_ws_info.ssl_private_key_filepath = NULL;
			m_ws_info.ws_ping_pong_interval = 0;
			//m_ws_info.extensions = exts;
			m_ws_info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
			m_ws_info.options |= LWS_SERVER_OPTION_VALIDATE_UTF8;

			m_ws_context = lws_create_context(&m_ws_info);
			if (m_ws_context == NULL) {
				LOGE("creating ws context failed\n");
				return;
			}
			create_websocket_url();
		}


		bool conect_success = true;
		bool continue_flag = false;
		int totle = 2;
		if (gaussian_version > 2103)
			totle = 4;

		for (int i = 0; i < totle; i++)
		{
			if (!gwi[i].wsi) {
				LOGE("Client failed to connect to %s %s:%u %s\n",
					m_ws_client_info[i].address, m_ws_client_info[i].path, m_ws_client_info[i].port, gwi->url.c_str());
				continue_flag = true;

			}
		}

		for (int i = 0; i < totle; i++)
		{
			if (gwi[i].returnresult&&!gwi[i].success)
				conect_success = false;
		}
		if (!conect_success && connect_time.isreach(10000))
		{
			connect_time.update();

			lws_context_destroy(m_ws_context);

			m_ws_context = NULL;

			continue;
			
		}



		if (now.isreach(250))
		{
			lws_callback_on_writable_all_protocol(m_ws_context,
				&protocols[0]);
			now.update();
		}

		n = lws_service(m_ws_context, 100);

		Message MSG = getq(0);
		if (MSG.msgType == _EXIT_THREAD_)break;

	}
	if (m_ws_context)
	{
		lws_context_destroy(m_ws_context);

		m_ws_context = NULL;
	}

	LOGI("websocket exit");
}

bool gaussian_thread::create_one_url(std::string url, gaussian_websocket_info *gwi, lws_client_connect_info* lcci)
{
	gwi->wsi = NULL; gwi->returnresult = false; gwi->success = false; gwi->url.clear();
	gwi->cinterface = parent;
	if (!handle_ws_client_info(gwi->buffer, gwi->path, url.c_str(), lcci))
	{
		LOGE("creating ws client_info navigation_status failed\n");
		return false;
	}
	gwi->wsi = lws_client_connect_via_info(lcci);
	gwi->url = url;
	return true;
}

void gaussian_thread::create_websocket_url()
{
	LOCK_HELPER_AUTO_HANDLER(m_ml); 

	if (!create_one_url("/gs-robot/notice/navigation_status", &gwi[0], &m_ws_client_info[0]))
		return;

	if (!create_one_url("/gs-robot/notice/device_status", &gwi[1], &m_ws_client_info[1]))
		return;

	if (gaussian_version >2103)
	{

		if (!create_one_url("/gs-robot/notice/status", &gwi[2], &m_ws_client_info[2]))
			return;

		if (!create_one_url("/gs-robot/notice/system_health_status", &gwi[3], &m_ws_client_info[3]))
			return;
	}

}

bool gaussian_thread::handle_ws_client_info(char *buffer,char *path,std::string url, lws_client_connect_info *lccinfo)
{
	std::string ws_url = "http://";
	ws_url.append(GS_IP);
	ws_url.append(":");
	ws_url.append(GS_WS_PORT);
	//ws_url.append("/gs-robot/notice/status");
	//ws_url.append("/gs-robot/notice/navigation_status");
	ws_url.append(url);
	
	memset(buffer, 0x00, 1024); memset(path, 0x00, 1024);
	strcpy_s(buffer, 1024, ws_url.c_str());
	const char *prot, *p;
	memset(lccinfo, 0, sizeof(lws_client_connect_info));
	lccinfo->port = atoi(GS_WS_PORT);

	if (lws_parse_uri(buffer, &prot, &lccinfo->address, &lccinfo->port, &p))
	{
		LOGE("gaussian webs url error");
		return false;
	}

	path[0] = '/';
	strncpy(path + 1, p, 1024 - 2);
	path[1024 - 1] = '\0';
	lccinfo->path = path;
	LOGI("gaussian webs address %s", path);

	char bufferr_uri[1024] = { 0 };
	sprintf_s(bufferr_uri, 1024, "%s:%s", GS_IP, GS_WS_PORT);

	lccinfo->context = m_ws_context;
	lccinfo->ssl_connection = 0;
	lccinfo->host = lccinfo->address;
	lccinfo->origin = lccinfo->address;

	lccinfo->host = bufferr_uri;
	lccinfo->origin = bufferr_uri;
	lccinfo->protocol = NULL;

	return true;
}
