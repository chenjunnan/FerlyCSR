#include "robot_global_singleton.h"
#include <stdio.h>
#include "common_helper.h"

robot_global_singleton* robot_global_singleton::ifly_object=NULL;
CHANDLE robot_global_singleton::guard = common_mutex_create_threadlock();
robot_global_singleton::robot_global_singleton()
{
	m_ml_list = common_mutex_create_threadlock();
}


robot_global_singleton* robot_global_singleton::Get()
{
	if (ifly_object == NULL)
	{
		LOCK_HELPER_AUTO_HANDLER(guard)
		if (ifly_object == NULL)
			ifly_object = new robot_global_singleton();
	}
	return ifly_object;
}

int robot_global_singleton::Register_Init(std::string type, robot_global_autointerface*_cb)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml_list)
	if (m_register_list[type].first==0)
	{
		m_register_list[type].second.insert(_cb);
		m_register_list[type].first++;
		int ret=_cb->onInit();
		return ret;
	}
	m_register_list[type].second.insert(_cb);
	m_register_list[type].first++;
	return 0;
}

int robot_global_singleton::UnRegister_Uninit(robot_global_autointerface* _cb)
{
	LOCK_HELPER_AUTO_HANDLER(m_ml_list)
	int ret = 0;
	for (auto it = m_register_list.begin(); it != m_register_list.end();it++)
	{
		auto find_cb = it->second.second.find(_cb);
		if (find_cb != it->second.second.end())
		{
			it->second.first--;	
			if (it->second.first == 0)
				ret=(*find_cb)->onUninit();
			it->second.second.erase(find_cb);
		}
	}
	return ret;
}


robot_global_singleton::~robot_global_singleton()
{
	common_mutex_threadlock_release(m_ml_list);
}
