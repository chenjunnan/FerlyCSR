#pragma once
#include "xCommon.h"
#define MAX_POOL  20
class robot_http_pool
{
public:

	static robot_http_pool* getSingle();

	void ExitAll();

	bool getRequest(std::string url, VEC_BYTE&result,int timeout=5);

	bool postRequest(std::string url, VEC_BYTE&vaule, VEC_BYTE&result, int timeout = 5);

	~robot_http_pool();

private:
	robot_http_pool();
	void init();
	int AllocHttpIndex();
	void FreeHttpIndex(int i);
	static robot_http_pool* m_ins;
	CHANDLE			m_http[MAX_POOL];
	CHANDLE			m_maplocker;
	static CHANDLE	m_msinglelocker;
	std::vector<bool> m_finish_map;
};

