#pragma once
#include <string>
#include "json\json.h"
struct idcard_message
{
	int appid;
	int messagetype;
	int tasktype;
	std::string fromguid;
	std::string content;
	int return_state;
	int				   sex;//0 unknow  1 man 2 woman
	int                age;
	std::string        chnname;
	std::string        name;
	bool               isForeigners;
	std::string         nation;
	std::string         birthday;
	std::string         address;
	std::string         idno;
	std::string         start_sign;
	std::string         end_sign;
	std::string         front_image_path;
	std::string         back_image_path;
	std::string         sign_gov;
	std::string         person_image_path;
	std::string         idversion;
	std::string         idtype;
	std::string         newaddress;
	std::string         finger;
	int                 timeout;
	idcard_message()
	{
		appid = 0;
		messagetype = 0;
		return_state = 0;
		isForeigners = false;
		age = 0;
		sex = 0;
		timeout = 60;
		tasktype = -1;
	}
};
class robot_json_rsp
{
public:
	robot_json_rsp(){}
	~robot_json_rsp(){}
	

	std::string robot_json_rsp::idcard_wait(idcard_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("IDCARD_WAIT_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
		{
			json_object["error_msg"] = Json::Value(data.name);
		}
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::idcard_cancel(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("IDCARD_CANCEL_RESPONSE");
		json_object["error"] = Json::Value(errorcode);

		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::idcard_result(idcard_message &data)
	{
		Json::Value   json_idinfo;
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("IDCARD_RESULT_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
		{
			json_object["error_msg"] = Json::Value(data.name);
		}
		else

		{
			json_idinfo["isForeigners"] = Json::Value(data.isForeigners);
			json_idinfo["sex"] = Json::Value(data.sex);
			json_idinfo["age"] = Json::Value(data.age);
			json_idinfo["chnname"] = Json::Value(data.chnname);
			json_idinfo["name"] = Json::Value(data.name);
			json_idinfo["nation"] = Json::Value(data.nation);
			json_idinfo["birthday"] = Json::Value(data.birthday);
			json_idinfo["address"] = Json::Value(data.address);
			json_idinfo["idno"] = Json::Value(data.idno);
			json_idinfo["start_sign"] = Json::Value(data.start_sign);
			json_idinfo["end_sign"] = Json::Value(data.end_sign);
			json_idinfo["front_image_path"] = Json::Value(data.front_image_path);
			json_idinfo["back_image_path"] = Json::Value(data.back_image_path);
			json_idinfo["sign_gov"] = Json::Value(data.sign_gov);
			json_idinfo["person_image_path"] = Json::Value(data.person_image_path);
			json_idinfo["idversion"] = Json::Value(data.idversion);
			json_idinfo["idtype"] = Json::Value(data.idtype);
			json_idinfo["newaddress"] = Json::Value(data.newaddress);
			json_idinfo["finger"] = Json::Value(data.finger);
			json_object["result"] = Json::Value(json_idinfo);
		}
		return json_writer.write(json_object);
	}
}; 