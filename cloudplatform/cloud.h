#pragma once
#include <vector>
class cloud_mq_interface
{
public:
	cloud_mq_interface(){};
	virtual void MessageIn(std::vector<char> &data) = 0;
	virtual void ThreadQuit() = 0;
	virtual ~cloud_mq_interface(){}
};