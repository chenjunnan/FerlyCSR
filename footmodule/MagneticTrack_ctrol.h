#pragma once
#include "common_helper.h"
#include "robot_controller_interface.h"
#include "robot_controller_struct.h"
#include "robot_controller_mems.h"
#include "..\MagneticNavigation\MagneticNavigation.h"
class MagneticTrack_ctrol_thread :
	public single_handle_thread_base,
	public Controllerinterface,
	public MEMSCTROLSHandler
{
public:
	MagneticTrack_ctrol_thread(){ m_controler = NULL;	m_cmems.handleer = this; }
	virtual ~MagneticTrack_ctrol_thread(){ if (state())quit(); }
	virtual void mems_in(MEMSCTROLS &);
	MEMSCTROLS nowinfo;
	MagneticData handledata;
	void do_action();
	void stop_action();
	virtual int iMove(double ls, double as);
	virtual int iPosition(PositionStc *Ps);
	virtual int iProtector(ProtectorStc *Ps);
	virtual int iRotate(double angle, double as);
	virtual int iRotateWait();
	virtual int iRotateCancle();
private:
	std::mutex m_ml;
	robot_controller_mems m_cmems;
	virtual void run();
};

