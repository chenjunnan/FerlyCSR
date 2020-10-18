#include "MagneticTrack_ctrol.h"
#include "robot_PostMessage.h"


void MagneticTrack_ctrol_thread::mems_in(MEMSCTROLS &info)
{
	unsigned long long t = common_get_longcur_time_stampms();
	m_ml.lock();
	nowinfo = info;
	nowinfo.times = t;
	m_ml.unlock();
}


void MagneticTrack_ctrol_thread::do_action()
{

}

void MagneticTrack_ctrol_thread::stop_action()
{

}

int MagneticTrack_ctrol_thread::iMove(double ls, double as)
{
	return m_controler->move(ls, as);
}

int MagneticTrack_ctrol_thread::iPosition(PositionStc *Ps)
{
	grid_point_data current;
	int r = m_controler->get_current_point(current);
	Ps->angle = current.angle;
	Ps->ww = current.world.w;
	Ps->wx = current.world.x;
	Ps->wy = current.world.y;
	Ps->wz = current.world.z;
	Ps->x = current.x;
	Ps->y = current.y;
	return r;
}

int MagneticTrack_ctrol_thread::iProtector(ProtectorStc *Ps)
{
	std::vector<int> value;
	int r = m_controler->get_protector(value);
	for (int i = 0;  (value.size() && i < 6); i++)
		Ps->state[i] = (value[i] == 0);
	return r;
}

int MagneticTrack_ctrol_thread::iRotate(double angle, double as)
{
	return  m_controler->rotate(angle, as);
}

int MagneticTrack_ctrol_thread::iRotateWait()
{
	return m_controler->check_rotate_finish();
}

int MagneticTrack_ctrol_thread::iRotateCancle()
{
	return m_controler->stop_rotate();
}

void MagneticTrack_ctrol_thread::run()
{
	unsigned long long last_time_spit = 0;
	unsigned long long last_serial_command = 0;
	unsigned char buffer[2] = { 0x5A, 0x00 };
	// 	//double des_x = desx, des_y = desy;
	// 	double des_x = 6.891, des_y = 1.402;
	int _level = 0;
	common_config_get_field_int("robotunderpan", "level", &_level);
	if (_level != 0 && _level != 1)_level = 0;

	int port_o = 0;
	typedef CMagneticNavigation* (*fCreateNavigation)();
	typedef void(*fDestoryNavigation)(CMagneticNavigation **p);
	fCreateNavigation crt = NULL;
	fDestoryNavigation dcrt = NULL;
	LOGI("load MagneticNavigation module");
	HMODULE  moduleinstance = LoadLibrary("MagneticNavigation.dll");
	/*HMODULE  moduleinstance = LoadLibraryEx("MagneticNavigation.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);*/
	if (moduleinstance){
		crt = (fCreateNavigation)GetProcAddress(moduleinstance, "CreateNavigation");
		dcrt = (fDestoryNavigation)GetProcAddress(moduleinstance, "DestoryNavigation");
	}
	else
	{
		LOGE("load library error %d", GetLastError());
		std::string out = m_json.memsnotify(1001, "load library failed");
		robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.size());
		return;
	}

	if (crt == NULL || dcrt == NULL)
	{
		
		LOGE("get address error %d", GetLastError());
		std::string out = m_json.memsnotify(1001, "load library failed");
		robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.size());
		FreeLibrary(moduleinstance);
		return;
	}

	if (!common_config_get_field_int("robotunderpan", "port", &port_o))
	{
		LOGE("mase serial port no config!!");
		std::string out = m_json.memsnotify(1002, "serial port no config");
		robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.size());
		FreeLibrary(moduleinstance);
		return;
	}
	LOGI("try open com COM%d.", port_o);
	if (!m_cmems.open_com(port_o))
	{
		LOGE("mase serial port open error!!");
		std::string out = m_json.memsnotify(1003, "open serial port failed");
		robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.size());
		FreeLibrary(moduleinstance);
		return;
	}
	LOGW("enter mase thread");
	bool loop = true;
	CMagneticNavigation *mnavi = crt();
	mnavi->setController(this);
	int  waittime = 50;
	unsigned long long now_mms = 0;
	mnavi->Init(_level);
	std::string out = m_json.memsnotify(1005, "task failed ");
	while (loop)
	{
		Message  msg = getq(waittime);
		switch (msg.msgType)
		{
		case  _EXIT_THREAD_:
			loop = false;
			continue;
		}
		MEMSCTROLS temp;
		m_ml.lock();
		temp = nowinfo;
		m_ml.unlock();
		now_mms = common_get_longcur_time_stampms();

		if ((now_mms - temp.times) < 100)
		{
			handledata.angle = temp.angle;
			handledata.bpos = temp.bpos;
			handledata.fpos = temp.fpos;
			handledata.quadrant = temp.quadrant;
			handledata.times = temp.times;

			waittime = 10;
			//printf("%d %d %d %d\n", handledata.bpos, handledata.fpos, handledata.angle, handledata.quadrant);
			//int r = 100;
			int r = mnavi->Handle(&handledata);
			if (r == 0)
			{
				out = m_json.memsnotify(0, " success");
				loop = false;
			}
			if (r == 101||r==102)
				loop = false;
			if (r == 102)
				out = m_json.memsnotify(1004, "task failed need init");
		}
		else
		{
			LOGW("sensor no data");
			waittime += 50;
			if (waittime > 500 && !m_cmems.state())
			{
				LOGW("try reopen serial COM%d.", port_o);
				int ret = m_cmems.autoopen_com(port_o);
				LOGD("serial reopen ret :%d.",ret);
			
			}

		}
		if ((now_mms - handledata.times) > 5000)
		{
			out = m_json.memsnotify(1005, "task failed timeout");
			loop = false;
		}

	}

	robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.size());
	dcrt(&mnavi);
	FreeLibrary(moduleinstance);
	m_cmems.close_com();
	LOGW("exit mase thread");
}

//          leaser
//		  back  pos
//	   [4]	 32----2  [3]
//
//     [2]   32----2  [1]
//		  front pos
//        charge
// 
