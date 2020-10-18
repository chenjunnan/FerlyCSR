#include "iot.h"

void appendmessagetype(Json::Value &json_object_post, int type)
{
	if (type == OPENDOOR)
		json_object_post["msg_type"] = Json::Value("IOT_OPEN_DOOR_RESPONSE");
	if (type == CLOSEDOOR)
		json_object_post["msg_type"] = Json::Value("IOT_CLOSE_DOOR_RESPONSE");
	if (type == SETBOXAP)
		json_object_post["msg_type"] = Json::Value("IOT_SET_BOXAP_RESPONSE");
	if (type == SETFRAMEADSORB_OPEN)
		json_object_post["msg_type"] = Json::Value("IOT_ENABLE_FRAME_ADSORB_RESPONSE");
	if (type == SETFRAMEADSORB_CLOSE)
		json_object_post["msg_type"] = Json::Value("IOT_DISENABLE_FRAME_ADSORB_RESPONSE");
	if (type == GETBOXBATTERY)
		json_object_post["msg_type"] = Json::Value("IOT_GET_BOX_BATTERY_RESPONSE");
	if (type == GETDOORSTATUS)
		json_object_post["msg_type"] = Json::Value("IOT_GET_DOOR_STATUS_RESPONSE");
	if (type == GETPAPERSTATUS)
		json_object_post["msg_type"] = Json::Value("IOT_GET_PAPER_STATUS_RESPONSE");
}

void appenderrormessage(Json::Value &json_object_post, int error)
{
	json_object_post["error"] = error;
	if (error == 0)
		json_object_post["msg"] = Json::Value("success");
	if (error == 1)
		json_object_post["msg"] = Json::Value("write error");
	if (error == 2)
		json_object_post["msg"] = Json::Value("wait timeout");
	if (error == 3)
		json_object_post["msg"] = Json::Value("cant find id");
	if (error == 4)
		json_object_post["msg"] = Json::Value("offline missing");
	if (error == 5)
		json_object_post["msg"] = Json::Value("full task");
	if (error == 6)
		json_object_post["msg"] = Json::Value("action failed");
	if (error == 7)
		json_object_post["msg"] = Json::Value("data break");
	if (error == 8)
		json_object_post["msg"] = Json::Value("data break");
	if (error == 9)
		json_object_post["msg"] = Json::Value("serial port error");
}

std::string JsonPackage(std::string id, int type, int error)
{
	Json::Value   json_object_post;
	Json::FastWriter json_writer;
	appendmessagetype(json_object_post, type);
	json_object_post["id"] = Json::Value(id);
	appenderrormessage(json_object_post, error);
	std::string out = json_writer.write(json_object_post);
	return out;
}

std::string JsonPackage(std::string id, int type, int value, int error)
{
	Json::Value   json_object_post;
	Json::FastWriter json_writer;
	appendmessagetype(json_object_post, type);
	if (type == GETBOXBATTERY&&error==0)
		json_object_post["battery"] = Json::Value(value);
	json_object_post["id"] = Json::Value(id);
	appenderrormessage(json_object_post, error);
	std::string out = json_writer.write(json_object_post);
	return out;
}

std::string JsonPackage(std::string id, int type, std::vector<std::string > infoarray, int error)
{
	Json::Value   json_object_post;
	Json::FastWriter json_writer;
	appendmessagetype(json_object_post, type);
	json_object_post["id"] = Json::Value(id);
	bool  hasData = false;
	if (type == GETDOORSTATUS)
	{
		if (error == 0)
		{
			for (int i = 1; i < infoarray.size(); i++)
			{
				Json::Value   json_data;
				if (infoarray[i].size() != 2)continue;
				for (int j = 0; j < 2; j++)
				{
					Json::Value   json_layer;
					std::string status, surface;
					json_layer["surface"] = Json::Value(j == 0 ? "front" : "back");
					json_layer["status"] = Json::Value(infoarray[i].at(j) == '1' ? "open" : "close");
					json_data["door"].append(json_layer);
					hasData = true;
				}
				json_data["layer"] = Json::Value(i - 1);
				json_object_post["data"].append(json_data);
			}
		}
	}

	if (type == GETPAPERSTATUS)
	{
		if (error == 0)
		{
			for (int i = 1; i < infoarray.size(); i++)
			{
				Json::Value   json_data;
				if (infoarray[i].size() != 1)continue;
				json_data["sensor"] = Json::Value(infoarray[i].at(0) == '1' ? "full" : "ready");
				json_data["layer"] = Json::Value(i - 1);
				json_object_post["data"].append(json_data);
				hasData = true;
			}
		}

	}
	if (error==0&&!hasData)error = 7;

	appenderrormessage(json_object_post, error);
	std::string out = json_writer.write(json_object_post);
	return out;
}

std::vector<std::string> split(std::string strtem, char a)
{
	std::vector<std::string> strvec;
	std::string::size_type pos1, pos2;
	pos2 = strtem.find(a);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		strvec.push_back(strtem.substr(pos1, pos2 - pos1));
		pos1 = pos2 + 1;

		pos2 = strtem.find(a, pos1);
	}

	strvec.push_back(strtem.substr(pos1));
	return strvec;
}
