#pragma once
#include <string>
#include "json\json.h"
struct icreader_message
{
	int appid;
	int messagetype;
	int tasktype;
	std::string fromguid;
	std::string content;
	int return_state;
	std::vector<std::string>        mag_data;
	int				   type;
	int				   timeout;
	int				   card_type;
	std::string            errors;
	icreader_message()
	{
		appid = 0;
		messagetype = 0;
		return_state = 0;
		type = 0; //0 ascii 1	bin
		timeout = 60;
		card_type = 0;  //0 mag ka
		tasktype = -1;
	}
};
class robot_json_rsp
{
public:
	robot_json_rsp(){}
	~robot_json_rsp(){}
	

	std::string robot_json_rsp::iccard_wait(icreader_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("ICCARD_WAIT_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.errors);
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::iccard_result(icreader_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("ICCARD_RESULT_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.errors);

		if (data.return_state == 0 && data.mag_data.size() != 0)
		{
			for (int i = 0; i < data.mag_data.size(); i++)
				json_object["mag_result"].append(data.mag_data[i]);
			if (data.type == 0)
			{
				json_object["result_type"] = Json::Value("ascii");
			}
			if (data.type == 1)
			{
				json_object["result_type"] = Json::Value("bin");
			}
		}

		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::iccard_cancel(icreader_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("ICCARD_CANCEL_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.errors);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::iccard_remove(icreader_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("ICCARD_REMOVE_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
			json_object["error_msg"] = Json::Value(data.errors);
		return json_writer.write(json_object);
	}
}; 