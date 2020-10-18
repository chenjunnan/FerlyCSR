#pragma once
#include <string>
#include "json\json.h"
class robot_json_rsp
{
public:
	robot_json_rsp(){}
	~robot_json_rsp(){}
	
	std::string robot_json_rsp::print_text(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("PRINT_TEXT_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::print_feedpapaer(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("PRINT_FEEDPAPER_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::print_cutpapaer(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("PRINT_CUTPAPER_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::print_bmpfile(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("PRINT_BMPFILE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::print_status(int rett,bool nopaper, bool outofpaper)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("PRINT_STATUS_RESPONSE");
		json_object["error"] = Json::Value(rett);
		if (rett == 0)
		{
			json_object["nopaper"] = Json::Value(nopaper);
			json_object["outofpaper"] = Json::Value(outofpaper);
		}
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::print_close(int rett)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("PRINT_CLOSE_RESPONSE");
		json_object["error"] = Json::Value(rett);
		return json_writer.write(json_object);
	}
}; 