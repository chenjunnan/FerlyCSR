#pragma once

#include <list>
#include "common_helper.h"
#include <set>
#include "robot_json_rsp.h"

struct Expression_task
{
	int appid;
	int messagetype;
	std::string fromguid;
	std::string content;
	std::set<std::string> expressionpost_list;
	unsigned long long times;
	int return_state;
	Expression_task()
	{
		appid = 0;
		messagetype = 0;
		times = 0;
		return_state = 0;
	}
};


class robot_expression_module
{
public:
	robot_expression_module();
	void init();
	void stop();
	int message_handler(Expression_task &rcs);

	void remove_overdue();

	~robot_expression_module();
private:
	std::list<Expression_task> m_tasklist;
	std::set<std::string>  expression_client_list;
	robot_json_rsp robot_json;
};

