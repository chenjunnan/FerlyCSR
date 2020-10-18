#pragma once
#include <string>
#include "json\json.h"
struct idcardys_message
{
	int appid;
	int messagetype;
	int tasktype;
	std::string fromguid;
	std::string content;
	int return_state;
	int				    Sex;//0 unknow  1 man 2 woman
	std::string         ErrorInfo;
	std::string         CnName;
	std::string         EnName;
	std::string         Nation;
	std::string         BornDay;
	std::string         Address;
	std::string         IDNum;
	std::string         BeginValidity;
	std::string         EndValidity;
	std::string         Issued;
	std::string         IdVersion;
	std::string         IdType;
	std::string         NewAdd;
	std::string         FigData;
	std::string         DNCode;
	bool                isForeigners;
	int                 FigLen;
	int                 Age;
	int                 timeout;
	int                 DevState;

	idcardys_message()
	{
		appid = 0;
		messagetype = 0;
		tasktype = -1;
		return_state = 0;
		isForeigners = false;
		Sex = 0;
		Age = 0;
		timeout = 60;
	}
};
class robot_json_rsp
{
public:
	robot_json_rsp(){}
	~robot_json_rsp(){}
	

	std::string robot_json_rsp::idcardys_wait(idcardys_message &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("IDCARD_WAIT_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
		{
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		}
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::idcardys_cancel(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("IDCARD_CANCEL_RESPONSE");
		json_object["error"] = Json::Value(errorcode);

		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::idcardys_result(idcardys_message &data)
	{
		Json::Value   json_idysinfo;
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("IDCARD_RESULT_RESPONSE");
		json_object["error"] = Json::Value(data.return_state);
		if (data.return_state != 0)
		{
			json_object["error_msg"] = Json::Value(data.ErrorInfo);
		}

		else
		{
			json_idysinfo["isForeigners"] = Json::Value(data.isForeigners);
			json_idysinfo["sex"] = Json::Value(data.Sex);
			json_idysinfo["ErrorInfo"] = Json::Value(data.ErrorInfo);
			json_idysinfo["name"] = Json::Value(data.CnName);
			json_idysinfo["chnname"] = Json::Value(data.EnName);
			json_idysinfo["nation"] = Json::Value(data.Nation);
			json_idysinfo["birthday"] = Json::Value(data.BornDay);
			json_idysinfo["age"] = Json::Value(data.Age);
			json_idysinfo["address"] = Json::Value(data.Address);
			json_idysinfo["idno"] = Json::Value(data.IDNum);
			json_idysinfo["start_sign"] = Json::Value(data.BeginValidity);
			json_idysinfo["end_sign"] = Json::Value(data.EndValidity);
			json_idysinfo["sign_gov"] = Json::Value(data.Issued);
			json_idysinfo["idversion"] = Json::Value(data.IdVersion);
			json_idysinfo["idtype"] = Json::Value(data.IdType);
			json_idysinfo["newaddress"] = Json::Value(data.NewAdd);
			json_idysinfo["finger"] = Json::Value(data.FigData);
			json_idysinfo["fingerLen"] = Json::Value(data.FigLen);
			json_idysinfo["DNCode"] = Json::Value(data.DNCode);
			json_idysinfo["person_image_path"] = Json::Value("c:\\zp.bmp");
			json_object["result"] = Json::Value(json_idysinfo);
		}
		return json_writer.write(json_object);
	}

	std::string robot_json_rsp::stateinfo_result(idcardys_message &data)
	{
		Json::Value   json_stateinfo;
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("IDCARD_DEVSTATE_INFO_RESPONSE");
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
}; 