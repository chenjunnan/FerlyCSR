#pragma once
#include <string>
#include "json\json.h"

class robot_json_rsp
{
public:
	robot_json_rsp(){}
	~robot_json_rsp(){}
	std::string robot_json_rsp::APPLYFOR_INTERVENTION(int errorcode, int uid=0,std::string channle="")
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("PSYCHEL_APPLYFOR_INTERVENTION_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		if (errorcode == 0)
		{
			if (uid!=0)
				json_object["uid"] = Json::Value(uid);
			if (!channle.empty())
				json_object["channle"] = Json::Value(channle);
		}

		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::ABANDON_INTERVENTION(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("PSYCHEL_ABANDON_INTERVENTION_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::POST_MESSAGEDATA(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("PSYCHEL_POST_MESSAGEDATA_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::CLIENT_MESSAGE_NOTIFY(Json::Value &value, std::string f, std::string t)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("PSYCHEL_CLIENT_MESSAGE_NOTIFY");
		json_object["data"] = value;
		json_object["function"] = f;
		json_object["packType"] = t;
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::INTERVENTION_NOTIFY(int state,std::string msg)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("PSYCHEL_INTERVENTION_NOTIFY");
		json_object["state"] = state;
		json_object["msg"] = msg;
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::CAMERA_CHANGE_USER(std::string streanstr)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["streamname"] = Json::Value(streanstr);
		json_object["msg_type"] = Json::Value("CAMERA_CHANGE_USER_REQUEST");
		return json_writer.write(json_object);
		
	}
	std::string robot_json_rsp::CAMERA_RESUME_USER(std::string streanstr)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["streamname"] = Json::Value(streanstr);
		json_object["msg_type"] = Json::Value("CAMERA_RESUME_USER_REQUEST");
		return json_writer.write(json_object);

	}


	std::string PSYCHEL_GET_STATUS(bool status)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["status"] = Json::Value(status);
		json_object["msg_type"] = Json::Value("PSYCHEL_GET_STATUS_RESPONSE");
		return json_writer.write(json_object);
	}
	std::string PSYCHEL_SHUTUP(int error)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["error"] = Json::Value(error);
		json_object["msg_type"] = Json::Value("PSYCHEL_SHUTUP_RESPONSE");
		return json_writer.write(json_object);
	}
	std::string PSYCHEL_SPEAK(int error)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["error"] = Json::Value(error);
		json_object["msg_type"] = Json::Value("PSYCHEL_SPEAK_RESPONSE");
		return json_writer.write(json_object);
	}
	std::string PSYCHEL_USERONLINE(unsigned int uid,std::string streamname)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["uid"] = Json::Value(uid);
		json_object["stream"] = Json::Value(streamname);
		json_object["msg_type"] = Json::Value("PSYCHEL_HELPER_ONLINE_NOTIFY");
		return json_writer.write(json_object);
	}
	std::string PSYCHEL_USEROFFLINE(unsigned int uid, std::string streamname)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["uid"] = Json::Value(uid);
		json_object["stream"] = Json::Value(streamname);
		json_object["msg_type"] = Json::Value("PSYCHEL_HELPER_OFFLINE_NOTIFY");
		return json_writer.write(json_object);
	}
	std::string PSYCHEL_HELPERLIST(int err,std::map<unsigned int,std::string > &uid2name)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["error"] = Json::Value(err);
		for (auto it = uid2name.begin(); it != uid2name.end();it++)
		{
			Json::Value item;
			item["uid"] = it->first;
			item["stream"] = it->second;
			json_object["helperlist"].append(item);
		}
		json_object["msg_type"] = Json::Value("PSYCHEL_HELPER_LIST_RESPONSE");
		return json_writer.write(json_object);
	}
};