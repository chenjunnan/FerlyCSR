#pragma once
#include "xCommon.h"
#include <map>
#include <atomic> 
#include <set>



class robot_global_autointerface
{
public :
	virtual int onInit() = 0;
	virtual int onUninit() = 0;
	virtual ~robot_global_autointerface(){};
};

class robot_global_singleton
{
public:
	static robot_global_singleton* Get();
	int Register_Init(std::string type,robot_global_autointerface*);
	int UnRegister_Uninit(robot_global_autointerface*);
	~robot_global_singleton();
	static CHANDLE	guard;
private:
	robot_global_singleton();
	static robot_global_singleton* ifly_object;
	CHANDLE	m_ml_list;
	std::map<std::string, std::pair<int, std::set<robot_global_autointerface*> > > m_register_list;
};

