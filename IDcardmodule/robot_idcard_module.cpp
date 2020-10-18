#include "robot_idcard_module.h"
#include "json\writer.h"
#include "json\reader.h"
#include "robot_PostMessage.h"
#include "common_helper.h"
#include "robot_json_rsp.h"
#define  IDCARD_TEMP_PATH    "idcard_tmp\\"
extern const char *MODULE_NAME;
robot_idcard_module::robot_idcard_module()
{
	timeout = 0;
}


robot_idcard_module::~robot_idcard_module()
{
	m_idreader.Close_dev();
	if (state())quit();
}


bool robot_idcard_module::message_handler(idcard_message& rcs)
{
	Json::Value   json_object;
	Json::Reader  json_reader;
	bool b_json_read = json_reader.parse(rcs.content, json_object);
	if (b_json_read)
	{
		std::string gbkstring;
		//common_utf8_2gbk(&rcs.content, &gbkstring);
		//LOGD("Receive face message request %s", gbkstring.c_str());
		if (json_object.isMember("msg_type") && json_object["msg_type"].isString())
		{
			std::string msgtype = json_object["msg_type"].asString();
			common_trim(&msgtype);
			if (msgtype.compare("IDCARD_WAIT_REQUEST") == 0)
			{
				if (json_object.isMember("timeout") && json_object["timeout"].isInt())
					rcs.timeout = json_object["timeout"].asInt();
				rcs.tasktype = 0;
			}
			if (msgtype.compare("IDCARD_CANCEL_REQUEST") == 0)
			{
				rcs.tasktype = 1;
			}
		}
	}
	idcard_message *ptr = new idcard_message;
	*ptr = rcs;
	pushq(rcs.tasktype, "", ptr);
	return true;
}


void robot_idcard_module::run()
{
	int has_task = -1;
	Timer_helper now;
	clean_msgqueue();
	bool has_info = false;
	bool init_ok = false;
	int timeout = 0;
	while (1)
	{
		Message  msg = getq(has_task);
		switch (msg.msgType)
		{
		case 0:
		{
			if (!msg.msgObject) break;

			idcard_message *rsmsg = (idcard_message *)msg.msgObject;

			if (has_task != 0)
			{
				now.update();
				if (m_idreader.Open_dev(1001) && m_idreader.Open_dev(2401))
				{
					rsmsg->return_state = 9012;
					std::string errtemp = m_idreader.get_error();
					common_gbk_2utf8(&errtemp, &rsmsg->name);
				}
				else
				{
					has_info = false;
					task_client_guid = rsmsg->fromguid;
					rsmsg->return_state = 0;
					has_task = 0;
					timeout = rsmsg->timeout;
				}
							
			}
			else
				rsmsg->return_state = 0;

			rsmsg->content = m_json.idcard_wait(*rsmsg);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, rsmsg->content.c_str(), rsmsg->content.length(), rsmsg->fromguid.c_str());
			SAFE_DELETE(rsmsg);
		}
			break;
		case	1:
		{
			if (!msg.msgObject) break;

			idcard_message *rsmsg = (idcard_message *)msg.msgObject;

			has_task = -1;
			has_info = false;
			
			m_idreader.Close_dev();

			rsmsg->return_state = 0;
			rsmsg->content = m_json.idcard_cancel(rsmsg->return_state);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, rsmsg->content.c_str(), rsmsg->content.length(), rsmsg->fromguid.c_str());
			SAFE_DELETE(rsmsg);
		}		
			break;
		case  _EXIT_THREAD_:
			return;
		case  -1:
			break;
		default:
			LOGE("unknow idcard msg type %d", msg.msgType);
		}


		if (has_task == 0)
		{
			if (m_idreader.Read_IDCard(get_temp_path()))
				has_info = true;	

			if (has_info || now.isreach(timeout * 1000))
			{
				idcard_message rsmsg;
				rsmsg.fromguid = task_client_guid;
				if (has_info)
				{
					rsmsg.address = m_idreader.info.address;
/*					printf("%s\n", m_idreader.info.address);*/
					rsmsg.chnname = m_idreader.info.enname;
					rsmsg.idno = m_idreader.info.IDno;
					rsmsg.idtype = m_idreader.info.IDtype;
					rsmsg.idversion = m_idreader.info.IDversion;
					rsmsg.name = m_idreader.info.name;
					rsmsg.nation = m_idreader.info.nation;
					rsmsg.sign_gov = m_idreader.info.Iss_aut;
					rsmsg.newaddress = m_idreader.info.new_add;

					rsmsg.age = m_idreader.info.old;
					std::string path = get_temp_path().append("back.jpg");
					common_gbk_2utf8(&path, &rsmsg.back_image_path);
					

					rsmsg.birthday = m_idreader.info.B_data;
					rsmsg.start_sign = m_idreader.info.sta_data;
					rsmsg.end_sign = m_idreader.info.end_data;

					path = get_temp_path().append("front.jpg");
					common_gbk_2utf8(&path, &rsmsg.front_image_path);

					rsmsg.isForeigners = m_idreader.info.isf;
					path = get_temp_path().append("ZP.BMP");
					common_gbk_2utf8(&path, &rsmsg.person_image_path);

					if (strcmp(m_idreader.info.sex,"ÄÐ")==0)
						rsmsg.sex = 1;
					if (strcmp(m_idreader.info.sex, "Å®") == 0)
						rsmsg.sex = 2;
					
 					common_trim(&rsmsg.address);
					common_trim(&rsmsg.chnname);
					common_trim(&rsmsg.nation);
					common_trim(&rsmsg.idno);
					common_trim(&rsmsg.name);
					common_trim(&rsmsg.sign_gov);
					common_trim(&rsmsg.idtype);
					common_trim(&rsmsg.idversion);
					common_trim(&rsmsg.newaddress);

					common_gbk_2utf8(&rsmsg.address, &rsmsg.address);
					common_gbk_2utf8(&rsmsg.chnname, &rsmsg.chnname);
					common_gbk_2utf8(&rsmsg.idno, &rsmsg.idno);
					common_gbk_2utf8(&rsmsg.idtype, &rsmsg.idtype);
					common_gbk_2utf8(&rsmsg.idversion, &rsmsg.idversion);
					common_gbk_2utf8(&rsmsg.name, &rsmsg.name);
					common_gbk_2utf8(&rsmsg.nation, &rsmsg.nation);
					common_gbk_2utf8(&rsmsg.sign_gov, &rsmsg.sign_gov);
					common_gbk_2utf8(&rsmsg.newaddress, &rsmsg.newaddress);

					common_base64_encode_string((unsigned char*)m_idreader.info.fig_tmp, 1024, &rsmsg.finger);
				}
				else
				{
					rsmsg.name = "timeout";
				}
				rsmsg.return_state = has_info ? 0 : 9022;
				rsmsg.content = m_json.idcard_result(rsmsg);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, rsmsg.content.c_str(), rsmsg.content.length(), rsmsg.fromguid.c_str());
				has_task = -1;
				has_info = false;
				m_idreader.Close_dev();
			}
		}

	}
}


std::string robot_idcard_module::get_temp_path()
{
	std::string path;
	common_get_current_dir(&path);
	common_fix_path(&path);
	path.append(IDCARD_TEMP_PATH);//sql

	if (!common_isDirectory(path.c_str()))
		common_create_dir(path.c_str());
	return path;
}

