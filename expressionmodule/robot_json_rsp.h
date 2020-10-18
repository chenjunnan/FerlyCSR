#pragma once
#include <string>
#include "json\json.h"
class robot_json_rsp
{
public:
	robot_json_rsp(){}
	~robot_json_rsp(){}
	
	std::string robot_json_rsp::default_expression_return(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("EXPRESSION_CHANGE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
}; 