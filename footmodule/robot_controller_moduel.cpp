#include "robot_controller_moduel.h"
#include "common_helper.h"
#include "robot_controller_gaussian.h"
#include <hash_map>
#include "robot_PostMessage.h"
#include "robot_controller_ferly.h"

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "robot_controller_slamware.h"


#define  PI 3.1415925
#define  WAITTYPE 2

robot_controller_moduel::robot_controller_moduel()
{
	m_rci = NULL;
	m_init_ok = false;
	control_id = 0;
	m_type=	0;
	m_ml = common_mutex_create_threadlock();
}

int robot_controller_moduel::init(int id,int type)
{
	SAFE_DELETE(m_rci);
	if (type == 1)
	{
		m_type = type;
		m_rci = new robot_controller_ferly();	
	}
	if (type == 2)
	{
		m_type = type;
		m_rci = new robot_controller_slamware();
		m_detect_slam_moveobject.set_handler(m_rci);
	}
	else
	{
		m_type = 0;
		m_rci = new robot_controller_gaussian();

		
	m_dance_thread.set_handler(m_rci);
	m_detect_moveobject.set_handler(m_rci);
	m_speed_detect.set_handler(m_rci);
	m_checkmove_ctrl.set_handler(m_rci);
	m_checkmove_ctrl.start();
	m_mtc.set_handler(m_rci);
	}
	m_move_ctrl.set_handler(m_rci);
	m_move_ctrl.start();
	control_id = id;
	return 1;
}

robot_controller_moduel::~robot_controller_moduel()
{
	SAFE_DELETE(m_rci);
	if (state())quit();
	common_mutex_threadlock_release(m_ml);
}

bool robot_controller_moduel::message_handler(FootCtrlMessage &rcs)
{
	switch (rcs.messagetype)
	{
	case NAVIGATE_TO_POSTION_REQUEST:
	{

		grid_point_data grid_data;
		grid_data.x = rcs.detail.x;
		grid_data.y = rcs.detail.y;
		grid_data.angle = rcs.detail.r;
		rcs.error = m_rci->navigate_to(grid_data);
		LOGI("[robot navigation] to x:%d y:%d r:%d result %s", rcs.detail.x, rcs.detail.y, rcs.detail.r, rcs.error==0?"success":"failed");
		rcs.response = m_json.navigate_to(rcs.error);
	}
	break;
	case NAVIGATE_TO_CANCEL_REQUEST:
	{

		rcs.error = m_rci->navigate_cancel();
		LOGI("[robot navigation] cancel result %s", rcs.error == 0 ? "success" : "failed");
		rcs.response = m_json.navigate_cancel(rcs.error);
	}
	break;
	case NAVIGATE_GET_CURRENT_POSTION_REQUEST:
	{
		{
			special_point_data grid_data;
			std::string mapname;
			rcs.error = m_rci->get_current_point(grid_data.postion);

			if (rcs.error == 0)
			{
				if (!m_rci->get_current_map_name(mapname))rcs.detail.mapname = mapname;
			}
			rcs.detail.point_list.push_back(grid_data);
		}	
		rcs.response = m_json.navigate_curpostion(&rcs);
	}break;
	case NAVIGATE_INITIALIZE_REQUEST:
	{
		LOCK_HELPER_AUTO_HANDLER(m_ml);
		if (m_dance_thread.state())
			m_dance_thread.quit();

		if (!rcs.detail.mapname.empty())
		{
			special_point_data  data;
			data.name = rcs.detail.mapname;
			data.postion.x = rcs.detail.x;
			data.postion.y = rcs.detail.y;
			data.postion.angle = rcs.detail.r;
			data.type = rcs.detail.type;
			rcs.error = m_rci->initialize_robot(data);

			if (rcs.error==0)
			{
				pushq(3, rcs.fromguid);
				return false;
			}
		}
		else
			rcs.error = 1008;
		
		rcs.response = m_json.navigate_init(rcs.error);
	}
	break;
	case NAVIGATE_MOVE_REQUEST:
	{
		if (m_move_ctrl.state())
		{
			move_message * mm = new move_message();
			mm->as = rcs.detail.as;
			mm->ls = rcs.detail.ls;
			mm->guid = rcs.fromguid;
			m_move_ctrl.pushq(1, "", mm);
			return false;
		}
		else
		{
			rcs.error = 5015;
			rcs.response = m_json.navigate_move(rcs.error);
		}
		//rcs.error = m_rci->move(rcs.detail.ls, rcs.detail.as);
	}break;
	case NAVIGATE_ROTATE_REQUEST:
	{
		if (m_dance_thread.state())
			m_dance_thread.quit();
		if (rcs.detail.as == 0) rcs.detail.as = 0.4;

		if (m_wait_th[ROTATE_THREAD_ID].state())
			m_wait_th[ROTATE_THREAD_ID].quit();
	
		rcs.error = m_rci->rotate(rcs.detail.r, rcs.detail.as);

		if (rcs.error == 0)
		{
			m_wait_th[ROTATE_THREAD_ID].setFromer(rcs.fromguid);
			m_wait_th[ROTATE_THREAD_ID].setWaitHandle(new wait_rotate(m_rci));
			m_wait_th[ROTATE_THREAD_ID].clean_msgqueue();
			m_wait_th[ROTATE_THREAD_ID].start();
		}
		rcs.response = m_json.navigate_rotate(rcs.error);
	}break;

	case NAVIGATE_DEVICE_STATUS_REQUEST:
	{
		rcs.error = m_rci->get_device_status(rcs.detail.ds);
		rcs.response = m_json.navigate_device_status(rcs.error, rcs.detail.ds);

	}break;
	case NAVIGATE_START_MOVEDATA_REQUEST:
	{
		if (m_type == 2)
		{
			rcs.error = m_detect_slam_moveobject.start() ? 0 : 1;
		}	
		else
		{
		rcs.error = m_detect_moveobject.start() ? 0 : 1;
		}
		rcs.response = m_json.navigate_movedata_start(rcs.error);

	}break;
	case NAVIGATE_STOP_MOVEDATA_REQUEST:
	{
		if (m_type == 2)
		{
			if (m_detect_slam_moveobject.state())
				m_detect_slam_moveobject.quit();
		}
		else
		{
		if (m_detect_moveobject.state())
			m_detect_moveobject.quit();
		}
		rcs.error = 0;
		rcs.response = m_json.navigate_movedata_stop(rcs.error);
	}break;
	case NAVIGATE_GET_MAP_REQUEST:
	{
		get_map(rcs);
		rcs.response = m_json.navigate_getmap(&rcs);
	}	
	break;
	case NAVIGATE_TO_CHARGE_REQUEST:
	case NAVIGATE_TO_SPECIALPOINT_REQUEST:
	{
		if (m_dance_thread.state())
			m_dance_thread.quit();
		rcs.error = m_rci->navigate_to(rcs.detail.name);
		if (rcs.messagetype == NAVIGATE_TO_CHARGE_REQUEST)
			rcs.response = m_json.navigate_to_charge(rcs.error);
		if (rcs.messagetype == NAVIGATE_TO_SPECIALPOINT_REQUEST)
			rcs.response = m_json.navigate_to_special(rcs.error);
	}
	break;
	case NAVIGATE_GET_CHARGE_LIST_REQUEST:
	case NAVIGATE_GET_SPECIALPOINT_LIST_REQUEST:
	{
		if (rcs.messagetype == NAVIGATE_GET_CHARGE_LIST_REQUEST)
			rcs.detail.type = 1;
		std::vector<special_point_data> plist;
		rcs.error = m_rci->get_special_point_list(rcs.detail.mapname, plist, rcs.detail.type);
		rcs.detail.point_list = plist;
		
		if (rcs.messagetype == NAVIGATE_GET_SPECIALPOINT_LIST_REQUEST)
			rcs.response = m_json.navigate_special_point_list(&rcs);
		if (rcs.messagetype == NAVIGATE_GET_CHARGE_LIST_REQUEST)
			rcs.response = m_json.navigate_charge_point_list(&rcs);
	}
	break;
	case NAVIGATE_DEL_CHARGEPOINT_REQUEST:
	case NAVIGATE_DEL_SPECIALPOINT_REQUEST:
	{
		rcs.error = m_rci->delete_point(rcs.detail.mapname, rcs.detail.name);
		if (rcs.messagetype == NAVIGATE_DEL_CHARGEPOINT_REQUEST)
			rcs.response = m_json.navigate_del_charge_point(rcs.error);
		if (rcs.messagetype == NAVIGATE_DEL_SPECIALPOINT_REQUEST)
			rcs.response = m_json.navigate_del_special_point(rcs.error);

		
	}
	break;
	case NAVIGATE_APPEND_CHARGEPOINT_REQUEST:
	case NAVIGATE_APPEND_SPECIALPOINT_REQUEST:
	{
		if (rcs.messagetype == NAVIGATE_APPEND_CHARGEPOINT_REQUEST)
			rcs.detail.type = 1;
		if (rcs.detail.type < 0)
			rcs.error = 5019;
		else
		{
			special_point_data tmp;
			tmp.name = rcs.detail.name;
			tmp.type = rcs.detail.type;
			tmp.postion.x = rcs.detail.x;
			tmp.postion.y = rcs.detail.y;
			tmp.postion.angle = rcs.detail.r;
			rcs.error = m_rci->remember_point(rcs.detail.mapname, tmp);
		}
		if (rcs.messagetype == NAVIGATE_APPEND_CHARGEPOINT_REQUEST)
			rcs.response = m_json.navigate_append_chargepoint(rcs.error);
		if (rcs.messagetype == NAVIGATE_APPEND_SPECIALPOINT_REQUEST)
			rcs.response = m_json.navigate_append_specialpoint(rcs.error);
	}break;
	case	NAVIGATE_START_CHARGE_REQUEST:
	{
		rcs.error = m_rci->start_charge();
		rcs.response = m_json.navigate_start_charge(rcs.error);
	}
	break;
	case	NAVIGATE_STOP_CHARGE_REQUEST:
	{
		rcs.error = m_rci->stop_charge();
		rcs.response = m_json.navigate_stop_charge(rcs.error);
	}
	break;
	case	NAVIGATE_GET_STATE_CHARGE_REQUEST:
	{
		int charge_state = 0;
		rcs.error = m_rci->get_charge_state(charge_state);
		rcs.detail.state = charge_state;
		rcs.response = m_json.navigate_charge_state(&rcs);
	}
	break;
	case	NAVIGATE_SHUTDOWN_REQUEST:
	{
		rcs.error = m_rci->shutdown();
		rcs.response = m_json.navigate_shutdown(rcs.error);
	}
	break;
	case NAVIGATE_RESET_ROBOT_REQUEST:
	{
		rcs.error = m_rci->soft_reset_reboot();
		rcs.response = m_json.navigate_reset(rcs.error);
	}break;
	case NAVIGATE_MAP_LIST_REQUEST:
	{
		rcs.error = m_rci->get_map_list(rcs.detail.map_dat);
		if (rcs.error == 0 && rcs.detail.map_dat.size() == 0)
			rcs.error = 5000;
		rcs.response = m_json.navigate_maplist(&rcs);
	}
	break;
	case NAVIGATE_CHANGE_SPEED_REQUEST:
	{
		rcs.error = m_rci->set_navigation_speed(rcs.detail.x);
		rcs.response = m_json.navigate_change_speed(rcs.error);
	}break;
	case NAVIGATE_GET_PATH_REQUEST:
	{
		 get_path(rcs);
		 rcs.response = m_json.navigate_mappathlist(&rcs);
	}break;
	case NAVIGATE_START_DANCE:
	{
		if (!m_init_ok)
		{
			rcs.error = 5005;
			return true;
		}
		m_dance_thread.start();
	}break;
	case NAVIGATE_STOP_DANCE:
	{
		if (m_dance_thread.state())
			m_dance_thread.quit();
	}break;
	case  NAVIGATE_GREETER_REQUEST:
	{
		if (m_type == 0)
		{
			if (rcs.detail.name.compare("start") == 0)
			{
				std::string map_name;
				rcs.error = 5012;
				if (!m_rci->get_current_map_name(map_name))
					rcs.error = ((robot_controller_gaussian*)m_rci)->start_greeter(map_name, rcs.detail.greeter_name);
			}
			if (rcs.detail.name.compare("stop") == 0)
			{
				rcs.error = ((robot_controller_gaussian*)m_rci)->stop_greeter();
			}
			if (rcs.detail.name.compare("pause") == 0)
			{
				rcs.error = ((robot_controller_gaussian*)m_rci)->pause_greeter();
			}
			if (rcs.detail.name.compare("resume") == 0)
			{
				rcs.error = ((robot_controller_gaussian*)m_rci)->resume_greeter();
			}
		}
		else
		{
			rcs.error = 0;
		}
		rcs.response = m_json.navigate_greeter(&rcs);

	}break;
	case NAVIGATE_HEALTH_REQUEST:
	{
		rcs.response = m_rci->get_health_report();;
	}break;
	case NAVIGATE_RESUME_MOTOR_REQUEST:
	{
		rcs.error = ((robot_controller_gaussian*)m_rci)->resume_motor(-1);
		rcs.response = m_json.navigate_resume_motor(rcs.error);
	}break;
	case NAVIGATE_MOVETO_REQUEST:
	{
		if (m_dance_thread.state())
			m_dance_thread.quit();
		if (m_wait_th[MOVETO_THREAD_ID].state())
			m_wait_th[MOVETO_THREAD_ID].quit();
		
		rcs.error = m_rci->moveto(rcs.detail.ls, rcs.detail.as);
		if (rcs.error == 0)
		{
			m_wait_th[MOVETO_THREAD_ID].setFromer(rcs.fromguid);
			m_wait_th[MOVETO_THREAD_ID].setWaitHandle(new wait_moveto(m_rci));
			m_wait_th[MOVETO_THREAD_ID].clean_msgqueue();
			m_wait_th[MOVETO_THREAD_ID].start();
		}
		rcs.response = m_json.navigate_moveto(rcs.error);
	}break;
	case NAVIGATE_STOPMOVETO_REQUEST:
	{
		if (m_wait_th[MOVETO_THREAD_ID].state())m_wait_th[MOVETO_THREAD_ID].quit();
		rcs.error = m_rci->stop_moveto();
		rcs.response = m_json.navigate_stop_moveto(rcs.error);
	}break;
	case NAVIGATE_STOPROTATE_REQUEST:
	{
		if (m_wait_th[ROTATE_THREAD_ID].state())m_wait_th[ROTATE_THREAD_ID].quit();
		rcs.error = m_rci->stop_rotate();
		rcs.response = m_json.navigate_stop_rotate(rcs.error);
	}break;
	case NAVIGATE_MOTOR_REQUEST:
	{
		rcs.error = m_rci->motor(rcs.detail.fstat);
		rcs.response = m_json.navigate_motor(rcs.error);
	}
	break;
	case NAVIGATE_GET_SPEED_REQUEST:
	{
		rcs.error = m_rci->get_current_speed(rcs.detail.ls, rcs.detail.as);
		rcs.response = m_json.navigate_speed(rcs.error, rcs.detail.as, rcs.detail.ls);
	}
	break;
	case NAVIGATE_START_SPEED_REQUEST:
	{
		if (m_speed_detect.state())
		{
			if (m_speed_detect.client_name.compare(rcs.fromguid) == 0)
				rcs.error = 0;
			else rcs.error = 1001;
		}
		else
		{
			rcs.error = m_speed_detect.start() ? 0 : 1;
			m_speed_detect.client_name = rcs.fromguid;
		}
			
	}break;
	case NAVIGATE_STOP_SPEED_REQUEST:
	{
		if (m_speed_detect.state())
			m_speed_detect.quit();

		rcs.error = 0;
	}break;
	case NAVIGATE_SCAN_MAP_REQUEST:
	{
		if (m_dance_thread.state())
			m_dance_thread.quit();

		if (rcs.detail.mapname.empty())
			rcs.error = 5020;
		else
			rcs.error = m_rci->scan_map(rcs.detail.mapname);
		rcs.response = m_json.navigate_scan(rcs.error);
	}break;
	case NAVIGATE_SCAN_SAVEMAP_REQUEST:
	{
		rcs.error = m_rci->save_map();
		rcs.response = m_json.navigate_save_map(rcs.error);
	}break;
	case NAVIGATE_SCAN_CANCEL_REQUEST:
	{
		rcs.error = m_rci->cancel_scan();
		rcs.response = m_json.navigate_cancel_scanmap(rcs.error);
	}break;
	case NAVIGATE_SCAN_GET_MAP_REQUEST:
	{
		std::vector<char > img;
		if (m_type == 0)
		{
			rcs.error = m_rci->get_scan_map(img);
		if (!rcs.error)
		{
			rcs.detail.name = "scan_map.png";
			if (encode_map(rcs.detail.state, img, rcs.detail.name) == 0)
			{
				if (rcs.detail.state == 1)
					rcs.detail.greeter_name = "png";
			}
			else
				rcs.error = 2;
			}
		}
		if (m_type == 2)
		{
			std::string path;
			common_get_current_dir(&path);
			common_fix_path(&path);
			path.append("map\\");//sql
			if (!common_isDirectory(path.c_str()))
				common_create_dir(path.c_str());
			path.append("scan_map.bmp");
			img.assign(path.begin(), path.end());
			rcs.error = m_rci->get_scan_map(img);
			if (!rcs.error)
			{
				rcs.detail.name = path;
			}
		}
		rcs.response = m_json.navigate_get_scanmap(&rcs);
	}break;
	case NAVIGATE_DEL_MAP_REQUEST:
	{		
		if (rcs.detail.mapname.empty())
			rcs.error = 5020;
		else
			rcs.error = m_rci->del_map(rcs.detail.mapname);
		rcs.response = m_json.navigate_del_map(rcs.error);

	}break;
	case NAVIGATE_GET_VIRTUAL_WALL_REQUEST:
	{
		if (rcs.detail.mapname.empty())
			rcs.error = 5020;
		else
		{
			std::string out;
			rcs.error = m_rci->get_virtual_wall(rcs.detail.mapname, out);
			rcs.detail.name.clear();
			rcs.detail.name = out;
		}
		rcs.response = m_json.navigate_get_vwall(rcs.error, rcs.detail.name);

	}break;
	case NAVIGATE_SET_VIRTUAL_WALL_REQUEST:
	{
		if (rcs.detail.mapname.empty() || rcs.detail.greeter_name.empty())
			rcs.error = 5020;
		else
			rcs.error = m_rci->update_virtual_wall(rcs.detail.mapname, rcs.detail.greeter_name);
		rcs.response = m_json.navigate_set_vwall(rcs.error);
	}break;
	case NAVIGATE_GET_PROTECT_REQUEST:
	{
		rcs.error = m_rci->get_protector(rcs.detail.protector);
		rcs.response = m_json.navigate_get_protector(rcs.error, rcs.detail.protector);
	}break;
	case NAVIGATE_START_MASE_REQUEST:
	{		
		if (!m_mtc.state())
		{
			m_mtc.handledata.desangle = rcs.detail.rs;
			m_mtc.handledata.desx = rcs.detail.ls;
			m_mtc.handledata.desy = rcs.detail.as;
			m_mtc.handledata.times = 0;
			m_mtc.nowinfo.clean();
			m_mtc.handledata.times = common_get_longcur_time_stampms();
			m_mtc.clean_msgqueue();
			rcs.error = m_mtc.start() ? 0 : 1;
		}
		else
			rcs.error = 10;
		rcs.response = m_json.startmase(rcs.error);
	}break;
	case NAVIGATE_STOP_MASE_REQUEST:
	{	
		if (m_mtc.state() )
		{
			m_mtc.pushq(_EXIT_THREAD_);
			if (m_mtc.wait(5000))
				rcs.error = 0;
			else
				rcs.error = 11;
		}else
			rcs.error = 0;
		rcs.response = m_json.stopmase(rcs.error);
	}break;
	case	NAVIGATE_GET_GRAPHPATH_LIST_REQUEST:
	{
		if (m_type == 0)
		{
			rcs.error = ((robot_controller_gaussian*)m_rci)->get_graphlist(rcs.detail.mapname, rcs.response);
		}
		else
		{
			rcs.error = 0;
			LOGE("undefine function!!!");
			return false;
		}
	}break;
	case	NAVIGATE_GET_TASK_STATUS_REQUEST:
	{

	}break;
	case	NAVIGATE_APPEND_TASKQUEUE_REQUEST:
	{

	}break;
	case	NAVIGATE_GET_TASKQUEUE_REQUEST:
	{

	}break;
	case	NAVIGATE_DEL_TASKQUEUE_REQUEST:
	{

	}break;

	case	NAVIGATE_START_TASKQUEUE_REQUEST:
	{
		if (m_type == 0)
		{
			std::string mapname;
			if (rcs.detail.taskcall.mapname.empty())
			{
				rcs.error = (m_rci->get_current_map_name(mapname) ? 109 :0) ;
				if (rcs.error==0)
				rcs.detail.taskcall.mapname = mapname;
			}
			for (int i = 0; i < rcs.detail.taskcall.queueinfo.size(); i++)
			{
				if (rcs.detail.taskcall.queueinfo[i].mapname.empty())
					rcs.detail.taskcall.queueinfo[i].mapname = rcs.detail.taskcall.mapname;
			}
			if (rcs.error == 0)
			rcs.error = ((robot_controller_gaussian*)m_rci)->start_queue(rcs.detail.taskcall);
		}
		else
		{
			rcs.error = 0;
		}
		rcs.response = m_json.SimpleResponse("NAVIGATE_START_TASKQUEUE_RESPONSE", rcs.error);
	}break;
	case	NAVIGATE_STOP_ALLTASKQUEUE_REQUEST:
	{
		if (m_type == 0)
		{

			rcs.error = ((robot_controller_gaussian*)m_rci)->stop_queue();
		}
		else
		{
			rcs.error = 0;
		}
		rcs.response = m_json.SimpleResponse("NAVIGATE_STOP_TASKQUEUE_RESPONSE", rcs.error);
	}break;
	case	NAVIGATE_PAUSE_TASKQUEUE_REQUEST:
	{
		if (m_type == 0)
		{

			rcs.error = ((robot_controller_gaussian*)m_rci)->pause_queue();
		}
		else
		{
			rcs.error = 0;
		}
		rcs.response = m_json.SimpleResponse("NAVIGATE_PAUSE_TASKQUEUE_RESPONSE", rcs.error);
	}break;
	case	NAVIGATE_RESUME_TASKQUEUE_REQUEST:
	{
		if (m_type == 0)
		{

			rcs.error = ((robot_controller_gaussian*)m_rci)->resume_queue();
		}
		else
		{
			rcs.error = 0;
		}
		rcs.response = m_json.SimpleResponse("NAVIGATE_RESUME_TASKQUEUE_RESPONSE", rcs.error);
	}break;
	case	NAVIGATE_STOP_CURRENTTASK_REQUEST:
	{

	}break;
	case	NAVIGATE_IS_TASKQUEUE_FINISH_REQUEST:
	{
		if (m_type == 0)
		{
			rcs.error = ((robot_controller_gaussian*)m_rci)->isfinish_queue(rcs.detail.fstat);
		}
		else
		{
			rcs.error = 0;
		}
		rcs.response = m_json.istaskqueuefinsih(rcs.detail.fstat, rcs.error);
	}break;
	case NAVIGATE_MOVE_CHECK_RANGE_REQUEST:
	{
		if (m_checkmove_ctrl.state())
		{
			move_message * mm = new move_message();
			mm->as = rcs.detail.as;
			mm->ls = rcs.detail.ls;
			mm->guid = rcs.fromguid;
			mm->tims = common_get_longcur_time_stampms();
			m_checkmove_ctrl.pushq(1, "", mm);
			return false;
		}
		else
		{
			rcs.error = 5119;
			rcs.response = m_json.navigate_move_check(rcs.error);
		}
	}break;
	case  NAVIGATE_UPDATE_MOVE_WALL_REQUEST:
	{
		if (m_checkmove_ctrl.state())
		{
			m_checkmove_ctrl.pushq(2, rcs.fromguid);
			return false;
		}
		else
		{
			rcs.error = 5119;
			rcs.response = m_json.navigate_move_check_update(rcs.error);
		}
	}break;
	case NAVIGATE_GET_LASER_RAW_REQUEST:
	{
		std::vector<grid_point_data> laser_list;
		int maxSu;
		rcs.error = m_rci->get_laser_data(laser_list, maxSu);
		rcs.response = m_json.navigate_laser_raw(laser_list, rcs.error);
	}break;
	case NAVIGATE_CANCEL_INITIALIZE_REQUEST:
	{
		rcs.error = m_rci->initialize_cancel_robot();
		rcs.response = m_json.navigate_init_cancel(rcs.error);
		pushq(2);
	}break;
	case NAVIGATE_GET_TASK_LIST_REQUEST:
	{
		if (m_type == 0)
		{
			rcs.error = ((robot_controller_gaussian*)m_rci)->get_tasklist(rcs.detail.mapname, rcs.response);
		}
		else
		{
			rcs.error = 0;
			LOGE("undefine function!!!");
			return false;
		}		
	}break;
	default:
		LOGE("error message type :%d", rcs.messagetype);
	}
	return true;
}


void robot_controller_moduel::get_path(FootCtrlMessage &rcs)
{
	if (!rcs.detail.mapname.empty() && rcs.detail.point_list.size() == 2)
	{
		navigation_path_data path;
		rcs.error = m_rci->get_twopoint_navigate_path(rcs.detail.mapname, rcs.detail.point_list[0].postion, rcs.detail.point_list[1].postion, path);
		rcs.detail.point_list.clear();
		for (int i = 0; i < path.path_pos_list.size(); i++)
		{
			special_point_data temp;
			temp.postion = path.path_pos_list[i];
			rcs.detail.point_list.push_back(temp);
		}
		rcs.detail.map_dat.push_back(path.map_info);
	}
	else
	{
		if (!m_init_ok)
		{
			rcs.error = 5005;
			return ;
		}
		navigation_path_data path;
		rcs.error = m_rci->get_navigate_path(path);
		rcs.detail.point_list.clear();
		for (int i = 0; i < path.path_pos_list.size(); i++)
		{
			special_point_data temp;
			temp.postion = path.path_pos_list[i];
			rcs.detail.point_list.push_back(temp);
		}
		rcs.detail.map_dat.push_back(path.map_info);
	}
}

void robot_controller_moduel::stop()
{
	if (m_rci)m_rci->release();
	if (state())quit();
	for (int i = 0; i < MAX_WAITTHREAD; i++)
	{
		if (m_wait_th[i].state())
			m_wait_th[i].quit();
	}

	if (m_dance_thread.state())m_dance_thread.quit();
	if (m_detect_moveobject.state())m_detect_moveobject.quit();
	if (m_detect_moveobject.state())m_detect_moveobject.quit();
	if (m_speed_detect.state())m_speed_detect.quit();
	if (m_move_ctrl.state())m_move_ctrl.quit();
	if (m_checkmove_ctrl.state())m_checkmove_ctrl.quit();
	if (m_mtc.state())m_mtc.quit();

}

int robot_controller_moduel::encode_map(int mode, std::vector<char >&imgdata, std::string &dataorname)
{
	if (mode==0)
	{
		std::string path;
		common_get_current_dir(&path);
		common_fix_path(&path);
		path.append("map\\");

		if (!common_isDirectory(path.c_str()))
			common_create_dir(path.c_str());
		path.append(dataorname);
		FILE *fp = fopen(path.c_str(), "wb");
		if (fp)
		{
			fwrite(imgdata.data(), 1, imgdata.size(), fp);
			fclose(fp);
			dataorname = path;
			return 0;
		}
		return 1;
	}
	if (mode==1)
	{
		common_base64_encode_string((unsigned char *)imgdata.data(), imgdata.size(), &dataorname);
		return 0;
	}
	return 1;
}

void robot_controller_moduel::get_map(FootCtrlMessage &rcs)
{
	std::string mapname;
	rcs.error = 190;
	if (rcs.detail.mapname.empty())
		rcs.error = m_rci->get_current_map_name(mapname) ? 109 : 0;
	else
	{
		rcs.error = 0;
		mapname = rcs.detail.mapname;
	}
	rcs.detail.name.clear();

	if (!rcs.error)
	{
		if (m_type == 0)
		{
		std::vector<char > img;
		rcs.error = m_rci->get_map_picture(mapname, img);
		if (!rcs.error)
		{
			rcs.detail.name = "map.png";
			if (encode_map(rcs.detail.state, img, rcs.detail.name) == 0)
			{
				if (rcs.detail.state == 1)
				{
					rcs.detail.greeter_name = "png";
					}
				}
				else
				rcs.error = 2;

			}
		}
		else if(m_type == 2)
		{
			std::string path;
			common_get_current_dir(&path);
			common_fix_path(&path);
			path.append("map\\");//sql
			if (!common_isDirectory(path.c_str()))
				common_create_dir(path.c_str());
			path.append("map.bmp");
			std::vector<char > name;
			name.assign(path.begin(), path.end());
			rcs.error = m_rci->get_map_picture(mapname, name);
			if (!rcs.error)
				rcs.detail.name = path;
		}
	}
}

void robot_controller_moduel::run()
{
	int waitTime = 0;
	std::vector<move_object_detect> object_detect_list;
	int init_count = 0;
	FootCtrlMessage fcmsg;
	fcmsg.detail.state = 2010;
	std::string report = m_json.navigate_state_ntf(&fcmsg);

	fcmsg.detail.state = 2000;
	std::string initok = m_json.navigate_state_ntf(&fcmsg);

	std::string neterror = m_json.navigate_neterror_ntf();
	bool isInited = false;
	
	if (m_checkmove_ctrl.state())
		m_checkmove_ctrl.pushq(3);


	Timer_helper now_init;
	bool start_wait_init = false;
	std::string fromer_guid;

	while (1)
	{
		Message  msg = getq(waitTime);

		switch (msg.msgType)
		{
		case 3:
		{
			LOCK_HELPER_AUTO_HANDLER(m_ml);
			if (start_wait_init)
			{
				std::string res = m_json.navigate_init(10);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, res.c_str(), res.size(), fromer_guid.c_str());
			}
			now_init.update();
			waitTime = 500;
			start_wait_init = true;
			m_init_ok = false;
			fromer_guid = msg.message;
			if (m_checkmove_ctrl.state())
				m_checkmove_ctrl.pushq(3);
		}
			break;
		case 2:
		{
			if (start_wait_init)
			{
				std::string res = m_json.navigate_init(10);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, res.c_str(), res.size(), fromer_guid.c_str());
			}
			start_wait_init = false;
		}
		case  _EXIT_THREAD_:
			return;
		case  -1:
			break;
		default:
			;
		}
		
		if (start_wait_init)
		{		
			int rt = m_rci->check_init_finish();
			std::string res;
			if (rt == 0)
			{
				LOCK_HELPER_AUTO_HANDLER(m_ml);
				res = m_json.navigate_init(0);
				m_init_ok = true;
				if (m_checkmove_ctrl.state())
					m_checkmove_ctrl.pushq(2);
			}
				 
			if (rt == -1||rt==2)
			{
				LOGE("gaussian initialize failed");
				res = m_json.navigate_init(9);
			}
			
			if (now_init.isreach(120000))
			{
				now_init.update();
				LOGW("timeout wait for gaussian init failed");
				res = m_json.navigate_init(11);

			}
			if (!res.empty())
			{
				robot_PostMessage::getPostManager()->post(MODULE_NAME, res.c_str(), res.size(), fromer_guid.c_str());
				start_wait_init = false;
			}
		}
		else
		{
			waitTime = 10000;
			if (!m_init_ok&&init_count < 10)
			{
				int ret = m_rci->init();
				if (ret == 2)
				{
					robot_PostMessage::getPostManager()->post(MODULE_NAME, neterror.c_str(), neterror.size());
					continue;
				}
				if (ret==3)
					init_count = 10;
				if (ret == 0)
				{
					LOCK_HELPER_AUTO_HANDLER(m_ml);
					m_init_ok = true;
					init_count = 10;
					if (m_checkmove_ctrl.state())
						m_checkmove_ctrl.pushq(2);
				}
				init_count++;
			}
			else
			{
				std::string mapname;
				int ret = m_rci->get_current_map_name(mapname);

				if (ret == 2)robot_PostMessage::getPostManager()->post(MODULE_NAME, neterror.c_str(), neterror.size());
				if (ret == 1 || ret == -1)
				{
					robot_PostMessage::getPostManager()->post(MODULE_NAME, report.c_str(), report.size());
					if (m_init_ok)
					{
						LOCK_HELPER_AUTO_HANDLER(m_ml);
						m_init_ok = false;
						if (m_checkmove_ctrl.state())
							m_checkmove_ctrl.pushq(3);

					}
					waitTime = 3000;
				}
				if (ret == 0)
				{
					if (!m_init_ok)
					{
						robot_PostMessage::getPostManager()->post(MODULE_NAME, initok.c_str(), initok.size());
						LOCK_HELPER_AUTO_HANDLER(m_ml);
						m_init_ok = true;
						if (m_checkmove_ctrl.state())
							m_checkmove_ctrl.pushq(2);
					}
				}
			}
	
		}

	}
}

void dance_thread::run()
{
	if (m_controler == NULL)
		return;

	Timer_helper now;
	grid_point_data current_point;
	grid_point_data current_move_point;
	bool dance_stop = false;
	bool navi_state = false;

	if (m_controler->get_current_point(current_point))
	{
		LOGE("get current error");
		return;
	}

	m_controler->move(0, 0.8);
	common_thread_sleep(500);
	m_controler->move(0, 0.8);
	common_thread_sleep(500);
	m_controler->move(0, 0.8);
	common_thread_sleep(500);
	m_controler->move(0, 0.8);
	common_thread_sleep(500);
	m_controler->move(0, 0.8);
	common_thread_sleep(500);
	m_controler->move(0, 0.8);
	common_thread_sleep(500);

	srand(time(NULL));
	printf("dance thread 0\n");
	while (!dance_stop)
	{
		Message  msg = getq(500);
		switch (msg.msgType)
		{
		case  _EXIT_THREAD_:
			dance_stop = true;
			break;
		}
		if (m_controler->get_current_point(current_move_point))
		{
			//LOGE("get current error");
			common_thread_sleep(500);
			continue;
		}
		
		double f_in = 0;
		double z_in = 0;


		int _x = current_move_point.x - current_point.x;
		int _y = current_move_point.y - current_point.y;
		int dis = sqrt(_x*_x + _y*_y);
		printf("dis:%d\n",dis);
		if (dis <=5)//7
		{
			if (dis <= 3 && navi_state)
			{
				printf("dance thread navigate_cancel\n");
				m_controler->navigate_cancel();
			}
		}
		else
		{
			if (!navi_state)
			{
				printf("dance thread navigate_to\n");
				m_controler->navigate_to(current_point);		
			}
		}
		navi_state = m_controler->is_navi();;
		printf("dance thread 1\n");
		int f_in_t = rand();


		f_in += ((double)((double)(f_in_t % 120) - 120 / 2) / 100);
 		while (abs(f_in) > 0.7)
			f_in /= 2;

		int z_in_t = rand();

		printf("dance thread 2  %d %d\n", f_in_t, z_in_t);
		z_in = ((double)((double)(z_in_t % 120) - 120 / 2) / 100);
		printf("dance thread 2  %f %f\n", f_in, z_in);
		if (!navi_state)
		{
			if (now.isreach(500))
			{
				if (z_in_t % 3 == 0)
				{
					m_controler->move(0, 0.8);
					common_thread_sleep(500);
				}
				printf("dance thread 3 %f %f\n", f_in, z_in);
				m_controler->move(f_in, z_in);
				common_thread_sleep(300);
				m_controler->move(f_in, z_in);
				common_thread_sleep(300);
				now.update();
			}
		}


	}

	if (!navi_state)
		m_controler->navigate_to(current_point);

	do
	{
		common_thread_sleep(1000);
	} while (m_controler->is_navi());
}

void dance_thread::updata_stop_postion(std::vector<grid_point_data> &stop_pos, std::vector<grid_point_data> &laser_list, grid_point_data &current_point)
{
	int su = 0;
	if (m_controler->get_laser_data(laser_list, su))

	{
		LOGE("get laser data error");
		return;
	}
	for (int j = 0; j < stop_pos.size(); j++)
	{
		int move = 0;
		for (int i = 0; i < laser_list.size(); i++)
		{
			if (laser_list[i].x >= (current_point.x - 6) &&
				laser_list[i].x <= (current_point.x + 6) &&
				laser_list[i].y >= (current_point.y - 6) &&
				laser_list[i].y <= (current_point.y + 6))
			{
				if (laser_list[i].x >= (stop_pos[j].x - 1) &&
					laser_list[i].x <= (stop_pos[j].x + 1) &&
					laser_list[i].y >= (stop_pos[j].y - 1) &&
					laser_list[i].y <= (stop_pos[j].y + 1))
				{
					move = 1;
				}
			}

		}
		if (move == 0)
			stop_pos.erase(stop_pos.begin() + j);
	}
	for (int i = 0; i < laser_list.size(); i++)
	{
		int find = 0;
		for (int j = 0; j < stop_pos.size(); j++)
		{
			if (laser_list[i].x == (stop_pos[j].x) &&
				laser_list[i].y == (stop_pos[j].y))
				find = 1;

		}
		if (find == 0)
			stop_pos.push_back(laser_list[i]);
	}
}

void movedetect_thread::run()
{
	while (1)
	{
		Message  msg = getq(200);
		switch (msg.msgType)
		{
		case  _EXIT_THREAD_:
			return;
		}
		move_data_handler();
	}
}

void movedetect_thread::move_data_handler()
{
	if (m_controler == NULL /*|| m_p->is_navi() */)//TODO
		return;

	std::vector<move_object_detect> move_object;

	std::vector<move_object_detect> object_detect;

	int ret = m_controler->detect_move_data(object_detect);

	if (ret)return;


	for (int i = 0; i < object_detect.size(); i++)
	{
		if (object_detect[i].distance < 80)
		{
			bool new_object = true;
			if (new_object)
				move_object.push_back(object_detect[i]);

		}
	}
	if (!m_controler->check_init_finish())
		get_near_laser_data(move_object);
	//m_last_detectobject = object_detect;//×ó+ ÓÒ-

	object_detect.clear();

	bool end_nn = false;
	int cccc = 0;
	while (!end_nn)
	{
		end_nn = true;
		while (move_object.size() != 0)
		{
			auto mod = move_object.begin();
			bool nnd = false;
			for (int j = 0; j < object_detect.size(); j++)
			{
				int ofx = abs(object_detect[j].object_position.x - mod->object_position.x);
				int ofy = abs(object_detect[j].object_position.y - mod->object_position.y);
				if ((ofx + ofy) < 7)
				{
					nnd = true;
					if (mod->distance > object_detect[j].distance)
						object_detect[j] = *mod;
					end_nn = false;
				}
			}
			if (!nnd)
				object_detect.push_back(*mod);
			move_object.erase(mod);
			cccc++;
		}
		if (!end_nn)
		{
			move_object = object_detect;
			object_detect.clear();
		}

	}




//	if (object_detect.size() != 0)
//	{
		FootCtrlMessage fcmsg;
		fcmsg.detail.move_data = object_detect;
		fcmsg.response = m_json.navigate_detectmove_ntf(&fcmsg);
		robot_PostMessage::getPostManager()->post(MODULE_NAME, fcmsg.response.c_str(), fcmsg.response.size());
//	}

}

double movedetect_thread::angle_sub(const int x1, const int y1, const int x2, const int y2)
{
	double  dir, Define_V_dir = 0;
	int       Define_V_dr, Define_V_dc;
	{
		if ((y2) == (y1))
		{
			if ((x2) >= (x1))
			{
				Define_V_dir = 0;
			}
			else
			{
				Define_V_dir = 180;
			}
		}
		else if ((y2) > (y1))
		{
			if ((x2) == (x1))
			{
				Define_V_dir = 90;
			}
			else if ((x2) > (x1))
			{
				Define_V_dr = (y2)-(y1);
				Define_V_dc = (x2)-(x1);

				if (Define_V_dr > Define_V_dc)
				{
					Define_V_dir = 90 - atan2((double)Define_V_dc, (double)Define_V_dr) * 180 / 3.1415926;
				}
				else
				{
					Define_V_dir = atan2((double)Define_V_dr, (double)Define_V_dc) * 180 / 3.1415926;
				}
			}
			else
			{
				Define_V_dr = (y2)-(y1);
				Define_V_dc = (x1)-(x2);

				if (Define_V_dr > Define_V_dc)
				{
					Define_V_dir = 90 + atan2((double)Define_V_dc, (double)Define_V_dr) * 180 / 3.1415926;
				}
				else
				{
					Define_V_dir = 180 - atan2((double)Define_V_dr, (double)Define_V_dc) * 180 / 3.1415926;
				}
			}
		}
		else
		{
			if ((x2) == (x1))
			{
				Define_V_dir = 270;
			}
			else if ((x2) > (x1))
			{
				Define_V_dr = (y1)-(y2);
				Define_V_dc = (x2)-(x1);

				if (Define_V_dr > Define_V_dc)
				{
					Define_V_dir = 270 + atan2((double)Define_V_dc, (double)Define_V_dr) * 180 / 3.1415926;
				}
				else
				{
					Define_V_dir = 360 - atan2((double)Define_V_dr, (double)Define_V_dc) * 180 / 3.1415926;
					if (Define_V_dir == 360) Define_V_dir = 0;
				}
			}
			else
			{
				Define_V_dr = (y1)-(y2);
				Define_V_dc = (x1)-(x2);

				if (Define_V_dr > Define_V_dc)
				{
					Define_V_dir = 270 - atan2((double)Define_V_dc, (double)Define_V_dr) * 180 / 3.1415926;
				}
				else
				{
					Define_V_dir = 180 + atan2((double)Define_V_dr, (double)Define_V_dc) * 180 / 3.1415926;
				}
			}
		}
		(dir) = (Define_V_dir);
	}

	dir += 0.0005;
	dir = ((int)(dir * 1000)) / 1000.0;
	return dir;
}


void movedetect_thread::get_near_laser_data(std::vector<move_object_detect> &move_object)
{
	std::vector<grid_point_data> stop_pos;

	Timer_helper now;
	grid_point_data current_point;
	grid_point_data current_move_point;
	std::vector<grid_point_data> laser_list;
	if (m_controler->get_current_point(current_point))
	{
		LOGE("get current error");
		return;
	}
	int maxSu = 0;
	if (m_controler->get_laser_data(laser_list, maxSu))
	{
		LOGE("get laser data error");
		return;
	}

	std::hash_map<int, int>   has_map;
	//printf("laser size : %d\n", laser_list.size());
	for (int i = 0; i < laser_list.size(); i++)
	{
		if (has_map.find(laser_list[i].x*maxSu + laser_list[i].y) != has_map.end())
			continue;
		has_map[laser_list[i].x*maxSu + laser_list[i].y] = 1;
		int ofx = laser_list[i].x - current_point.x;
		int ofy = laser_list[i].y - current_point.y;
		if (ofy > 60 || ofx > 60)
			continue;
		double dis = sqrt((double)(ofx*ofx) + (double)(ofy*ofy));
		if (dis < 60 && dis>3)
		{
			double angle = angle_sub(current_point.x, current_point.y, laser_list[i].x, laser_list[i].y);
			move_object_detect  temp;

			if (angle > 180)
			{
				angle = 360 - angle;
				angle *= -1;
			}

			temp.object_position.x = laser_list[i].x;
			temp.object_position.y = laser_list[i].y;


			temp.distance = dis;
			temp.object_angle = angle - current_point.angle;

			if (temp.object_angle > 180)
			{
				temp.object_angle = 360 - temp.object_angle;
				temp.object_angle *= -1;
			}
			if (temp.object_angle < -180)
				temp.object_angle = 360 + temp.object_angle;

			int ana = 0;
			if (dis < 10)
				ana = 70;
			else
				ana = 60;

			if (abs(temp.object_angle) < ana)
				move_object.push_back(temp);



		}
	}

}

void Slammovedetect_thread::run()
{
	while (1)
	{
		Message  msg = getq(200);
		switch (msg.msgType)
		{
		case  _EXIT_THREAD_:
			return;
		}
		move_data_handler();
	}
}
void Slammovedetect_thread::move_data_handler()
{
	if (m_controler == NULL /*|| m_p->is_navi() */)//TODO
		return;
	std::vector<move_object_detect> object_detect;
	int ret = m_controler->detect_move_data(object_detect);
	if (ret)return;
	FootCtrlMessage fcmsg;
	fcmsg.detail.move_data = object_detect;
	fcmsg.response = m_json.navigate_detectmove_ntf(&fcmsg);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, fcmsg.response.c_str(), fcmsg.response.size());
}

void detectspeed_thread::run()
{
	while (1)
	{
		Message  msg = getq(500);
		switch (msg.msgType)
		{
		case  _EXIT_THREAD_:
			return;
		}
		if (m_controler == NULL)
			continue;

		double ls=0, as = 0;

		int ret = m_controler->get_current_speed(ls,as);

		if (ret)continue;
		std::string out = m_json.navigate_speed(ret, as, ls);
		robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.size(), client_name.c_str());

	}
}

void move_ctrol_thread::run()
{
	while (1)
	{
		Message  msg = getq();
		switch (msg.msgType)
		{
		case 1:
		{
			if (msg.msgObject == NULL)continue;
			move_message * mm = (move_message *)msg.msgObject;
			if (m_controler != NULL)
			{
				int ret = m_controler->move(mm->ls, mm->as);
				if (!mm->guid.empty())
				{
					std::string out = m_json.navigate_move(ret);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.size(), mm->guid.c_str());
				}
			}
			SAFE_DELETE(mm);
		}break;
		case  _EXIT_THREAD_:
			return;
		}
	}
}


move_check_thread::move_check_thread()
{
	m_controler = NULL; m_finish_update = false;
}

int move_check_thread::updateVirualMap()
{
	m_finish_update = false;
	if (m_controler->check_init_finish())
		return -1;
	std::string mapname;
	if (m_controler->get_current_map_name(mapname))
		return -2;

	std::vector<char> data;
	if (m_controler->get_map_picture(mapname, data)!=0)
		return -3;
	_datamap = cv::imdecode(data, CV_LOAD_IMAGE_COLOR);
	if (_datamap.empty())
		return  -4;

	_datamap = cv::Mat(_datamap.rows, _datamap.cols, CV_8UC1);
	memset(_datamap.data, 0xff, _datamap.rows*_datamap.cols);
	std::string virdata;
	if (m_controler->get_virtual_wall(mapname, virdata) != 0)
		return -5;
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(virdata.c_str(), json_object);
	if (b_json_read)
	{
		if (json_object.isMember("obstacles") && json_object["obstacles"].isObject())
		{
			Json::Value json_data = json_object["obstacles"];

			if (json_data.isMember("circles") && json_data["circles"].isArray())
			{
				Json::Value  circles = json_data["circles"];
				for (int i = 0; i < circles.size(); i++)
				{
					Json::Value info = circles[i];
					int x = 0, y = 0;
					double rate = 0.0;
					if (info.isMember("center") && info["center"].isObject())
					{
						Json::Value cinfo = info["center"];
						if (cinfo.isMember("x") && cinfo["x"].isInt())
							x = cinfo["x"].asInt();
						if (cinfo.isMember("y") && cinfo["y"].isInt())
							y = _datamap.rows - cinfo["y"].asInt();
					}
					if (info.isMember("radius") && info["radius"].isDouble())
					{
						rate = info["radius"].asDouble();
					}
					cv::circle(_datamap,
						cv::Point(x, y),
						rate,
						cv::Scalar(0, 0, 0),
						-1);
				}
			}
			if (json_data.isMember("lines") && json_data["lines"].isArray())
			{
				Json::Value  lines = json_data["lines"];
				for (int i = 0; i < lines.size(); i++)
				{
					Json::Value info = lines[i];
					int x = 0, y = 0, x1 = 0, y1 = 0;
					if (info.isMember("start") && info["start"].isObject())
					{
						Json::Value cinfo = info["start"];
						if (cinfo.isMember("x") && cinfo["x"].isInt())
							x = cinfo["x"].asInt();
						if (cinfo.isMember("y") && cinfo["y"].isInt())
							y = _datamap.rows - cinfo["y"].asInt();
					}
					if (info.isMember("end") && info["end"].isObject())
					{
						Json::Value cinfo = info["end"];
						if (cinfo.isMember("x") && cinfo["x"].isInt())
							x1 = cinfo["x"].asInt();
						if (cinfo.isMember("y") && cinfo["y"].isInt())
							y1 = _datamap.rows - cinfo["y"].asInt();
					}

					cv::line(_datamap,
						cv::Point(x, y),
						cv::Point(x1, y1),
						cv::Scalar(0, 0, 0), 5
						);
				}
			}

			if (json_data.isMember("polygons") && json_data["polygons"].isArray())
			{
				Json::Value  polygons = json_data["polygons"];
				std::vector< std::vector<cv::Point> >  all_polygons;
				for (int i = 0; i < polygons.size(); i++)
				{
					Json::Value info = polygons[i];

					if (info.isArray())
					{
						std::vector<cv::Point> pointarr;
						for (int i = 0; i < info.size(); i++)
						{

							if (info[i].isObject())
							{
								Json::Value cinfo = info[i];
								cv::Point temp;
								if (cinfo.isMember("x") && cinfo["x"].isInt())
									temp.x = cinfo["x"].asInt();
								if (cinfo.isMember("y") && cinfo["y"].isInt())
									temp.y = _datamap.rows - cinfo["y"].asInt();
								pointarr.push_back(temp);
							}

						}
						all_polygons.push_back(pointarr);

					}

				}
				if (all_polygons.size() != 0)
					cv::fillPoly(_datamap, all_polygons, cv::Scalar(0, 0, 0));
			}


			if (json_data.isMember("polylines") && json_data["polylines"].isArray())
			{
				Json::Value  polylines = json_data["polylines"];
				std::vector< std::vector<cv::Point> >  all_polygons;
				for (int i = 0; i < polylines.size(); i++)
				{
					Json::Value info = polylines[i];

					if (info.isArray())
					{
						std::vector<cv::Point> pointarr;
						for (int i = 0; i < info.size(); i++)
						{

							if (info[i].isObject())
							{
								Json::Value cinfo = info[i];
								cv::Point temp;
								if (cinfo.isMember("x") && cinfo["x"].isInt())
									temp.x = cinfo["x"].asInt();
								if (cinfo.isMember("y") && cinfo["y"].isInt())
									temp.y = _datamap.rows - cinfo["y"].asInt();
								pointarr.push_back(temp);
							}

						}
						all_polygons.push_back(pointarr);

					}

				}
				if (all_polygons.size() != 0)
					cv::polylines(_datamap, all_polygons, false, cv::Scalar(0, 0, 0), 5);
			}

			if (json_data.isMember("rectangles") && json_data["rectangles"].isArray())
			{
				Json::Value  rectangles = json_data["rectangles"];
				for (int i = 0; i < rectangles.size(); i++)
				{
					Json::Value info = rectangles[i];
					int x = 0, y = 0, x1 = 0, y1 = 0;
					if (info.isMember("start") && info["start"].isObject())
					{
						Json::Value cinfo = info["start"];
						if (cinfo.isMember("x") && cinfo["x"].isInt())
							x = cinfo["x"].asInt();
						if (cinfo.isMember("y") && cinfo["y"].isInt())
							y = _datamap.rows - cinfo["y"].asInt();
					}
					if (info.isMember("end") && info["end"].isObject())
					{
						Json::Value cinfo = info["end"];
						if (cinfo.isMember("x") && cinfo["x"].isInt())
							x1 = cinfo["x"].asInt();
						if (cinfo.isMember("y") && cinfo["y"].isInt())
							y1 = _datamap.rows - cinfo["y"].asInt();
					}

					cv::rectangle(_datamap,
						cv::Point(x, y),
						cv::Point(x1, y1),
						cv::Scalar(0, 0, 0), -1
						);
				}
			}

		}

		cv::flip(
			_datamap,
			_datamap,
			0);
		cv::imwrite("wall.jpg", _datamap);

	}
	else
		return -6;


	m_finish_update = true;;
	return 0;;
}

move_check_thread::~move_check_thread()
{
	if (state())quit();
}


double fixcossin(double value)
{
	if (value > 0)
		return value - 0.5;
	if (value < 0)
		return value + 0.5;
	return value;
}

void move_check_thread::run()
{
	grid_point_data current_position;
	while (1)
	{
		Message  msg = getq();
		switch (msg.msgType)
		{
		case 1:
		{
			if (msg.msgObject == NULL)continue;
			move_message * mm = (move_message *)msg.msgObject;
			if (m_controler != NULL)
			{
				int ret = -1;
				do 
				{
					unsigned long long timstart = common_get_longcur_time_stampms();
					int fangle = 0;
					double rate = 0;
					int tox = 0, toy = 0;
					bool canmove = true;

					if (!m_finish_update)break;
					if ((timstart-mm->tims) > 300)
						break;
					if (mm->as>-0.0001&&mm->as<0.0001)
					{
						ret = m_controler->get_current_point(current_position);
						if (ret) break;
						if (current_position.angle < 0)current_position.angle = 360 + current_position.angle;
						rate = abs(2 * mm->ls) * (7.5 - (pow(mm->ls*10,2)/50.0));
						if (mm->ls < -0.0001)
						{
							current_position.angle = current_position.angle + 180;
							if (current_position.angle  > 360)current_position.angle = current_position.angle - 360;
						}
						tox = current_position.x + (/*fixcossin*/((double)rate*cos((double)current_position.angle*(PI / 180.0)) ));
						toy = current_position.y + (/*fixcossin*/((double)rate*sin((double)current_position.angle*(PI / 180.0))));
					}
					else
					{
						if (mm->ls>0.0001)
						{
							ret = m_controler->get_current_point(current_position);
							if (ret) break;
							if (current_position.angle < 0)current_position.angle = 360 + current_position.angle;
							if (mm->as > 0.0001)
								fangle = current_position.angle + 90;
							if (mm->as < -0.0001)
								fangle = current_position.angle - 90;

						}
						else if (mm->ls <-0.0001)
						{
							ret = m_controler->get_current_point(current_position);
							if (ret) break;
							if (current_position.angle < 0)current_position.angle = 360 + current_position.angle;
							if (mm->as > 0.0001)
								fangle = current_position.angle - 90;
							if (mm->as < -0.0001)
								fangle = current_position.angle + 90;
						}
						else
						{
							goto move;
						}


						if (fangle > 360)fangle = fangle- 360;
						if (fangle < 0)fangle = 360 + fangle;
						rate = /*8.5 */7.6/ abs(mm->as);
						int cirx = 0, ciry = 0;
						cirx = current_position.x + (fixcossin((double)rate*cos((double)fangle*(PI / 180.0)) ));
						ciry = current_position.y + (fixcossin((double)rate*sin((double)fangle*(PI / 180.0)) ));
						double ranangle = (((double)(abs(mm->ls/* + 2.0 / (mm->ls*20)*/) * (/*21*/11 + abs(mm->as * 13))) / rate)*180.0) / PI;
						double anglea = (acos(((double)(cirx-current_position.x)) / rate)*180.0) / PI;
						double angleb = 0;
						if (fangle>180)
							 angleb = 180 - (360-fangle);
						else
							angleb = 360 - (180 - fangle);
						anglea = angleb;
						double angle = 0;
						if (mm->as > 0.0001)
							angle = ranangle + anglea;
						if (mm->as < -0.0001)
							angle = anglea - ranangle;

						if (angle < 0)angle = 360 + angle;
						if (angle > 360)angle = angle-360 ;
						tox = cirx + (((double)(rate-0.5)*cos((double)angle*(PI / 180.0)))+0.5);
						toy = ciry + (((double)(rate-0.5)*sin((double)angle*(PI / 180.0)))+0.5);

					}
					int dex = tox - current_position.x;
					int dey = toy - current_position.y;
					int maxde = max(abs(dex), abs(dey));
					if (maxde == 0)
						break;
					double zlx = ((double)dex) / maxde;
					double zly = ((double)dey) / maxde;
					int linex = current_position.x;
					int liney = current_position.y;
					int safe_range = 1;
					for (int i = 0; i < maxde;i++)
					{
						linex = ((double)linex + zlx) + 0.5;
						liney = ((double)liney + zly) + 0.5;


						int leftpointx=linex - safe_range;
						int leftpointy = liney - safe_range;
						if (leftpointx < 0)leftpointx = 0;
						if (leftpointy < 0)leftpointy = 0;
						int rightpointx = linex +safe_range;
						int rightpointy = liney + safe_range;
						if (rightpointx > _datamap.cols)rightpointx = _datamap.cols;
						if (rightpointy > _datamap.rows)rightpointy = _datamap.rows;

						safe_range++;
						if (safe_range > 7)safe_range = 7;

						for (int xi = leftpointx; xi < rightpointx;xi++)
						{
							for (int yi = leftpointy; yi < rightpointy; yi++)
							{
								char value = _datamap.at<char>(yi, xi);
								if (value==0)
								{
									canmove = false;
									
									goto move;
								}

								
							}
						}


					}

				move:



					if (canmove)
						ret =  m_controler->move(mm->ls, mm->as);
					else
						ret = 9023;

				} while (0);

				if (!mm->guid.empty())
				{
					std::string out = m_json.navigate_move_check(ret);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.size(), mm->guid.c_str());
				}
			}
			SAFE_DELETE(mm);
		}break;
		case 2:
		{
			int ret=updateVirualMap();

			if (!msg.message.empty())
			{
				std::string out = m_json.navigate_move_check_update(ret);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, out.c_str(), out.size(), msg.message.c_str());
			}
		}break;
		case 3:
		{
			m_finish_update = false;
		}break;
		case  _EXIT_THREAD_:
			return;
		}
	}
}


void wait_thread::run()
{
	if (m_runfun)
	{
		std::string response;
		Timer_helper now; bool ret = false;
		do
		{
			ret = m_runfun->check();
			if (ret)break;

			if (m_runfun->waittime > 0 && now.isreach(m_runfun->waittime))
			{
				response=m_runfun->onTimeout();
				break;
			}
			if (m_runfun->waittime == 0)
				break;
			Message msg = getq(m_runfun->sleeptime);
			if (msg.msgType == _EXIT_THREAD_)
				break;

		} while (1);

		if (ret)
			response=m_runfun->onSuccess();
		else
		{
			if (m_runfun->waittime == 0)
				response=m_runfun->onFailed();
		}

		if (!response.empty())robot_PostMessage::getPostManager()->post(MODULE_NAME, response.c_str(), response.size(), clinet.empty() ? NULL : clinet.c_str());

	}
	release();
}



