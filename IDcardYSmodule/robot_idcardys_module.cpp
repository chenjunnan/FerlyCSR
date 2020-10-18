#include "robot_idcardys_module.h"
#include "robot_PostMessage.h"

extern const char *MODULE_NAME;
#define  IDCARD_TEMP_PATH    "idcard_tmp\\"
#define VERSION_MODULE "02.12"
#define MODULE_CLASS 0
#define SUBVERSION 0

robot_idcardys_module::robot_idcardys_module()
{
	DevState = 0;
	timeout = 0;
}


robot_idcardys_module::~robot_idcardys_module()
{
	m_idysreader.Send_CloseDev();
}


bool robot_idcardys_module::message_handler(idcardys_message& rcs)
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
			if (msgtype.compare("IDCARD_DEVSTATE_INFO_REQUEST") == 0)
			{
				rcs.tasktype = 2;
			}
		}
	}
	idcardys_message *ptr = new idcardys_message;
	*ptr = rcs;
	pushq(rcs.tasktype, "", ptr);
	return true;
}

char* WcharToChar(wchar_t* wc)
{
	int len = WideCharToMultiByte(CP_ACP, 0, wc, wcslen(wc), NULL, 0, NULL, NULL);
	char* m_char = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, wc, wcslen(wc), m_char, len, NULL, NULL);
	m_char[len] = '\0';
	return m_char;
}


int robot_idcardys_module::init()
{
	if (!common_config_get_field_string("idcard", "port", &m_port))
		m_port = "5";

	m_idysreader.Init_Dll();
	return 0;
}


void robot_idcardys_module::get_version(char *version)
{
	if (version == NULL)return;
	SYSTEMTIME st;
	char szDate[1024];
	ZeroMemory(szDate, sizeof(szDate));
	ZeroMemory(&st, sizeof(SYSTEMTIME));
	sprintf_s(szDate, 1024, "%s", __DATE__);

	st.wYear = atoi(szDate + 7);

	st.wDay = szDate[5] - '0';
	if (szDate[4] != ' ')
	{
		st.wDay += (szDate[4] - '0') * 10;
	}

	switch (szDate[0])
	{
	case 'A':
		if (szDate[1] == 'p')
		{
			st.wMonth = 4;
		}
		else
		{
			st.wMonth = 8;
		}
		break;

	case 'D':
		st.wMonth = 12;
		break;

	case 'F':
		st.wMonth = 2;
		break;

	case 'J':
		if (szDate[1] == 'a')
		{
			st.wMonth = 1;
		}
		else
		{
			if (szDate[2] == 'n')
			{
				st.wMonth = 6;
			}
			else
			{
				st.wMonth = 7;
			}
		}
		break;

	case 'M':
		if (szDate[2] == 'r')
		{
			st.wMonth = 3;
		}
		else
		{
			st.wMonth = 5;
		}
		break;

	case 'N':
		st.wMonth = 11;
		break;

	case 'O':
		st.wMonth = 10;
		break;

	case 'S':
		st.wMonth = 9;
		break;
	}

	ZeroMemory(szDate, sizeof(szDate));
	sprintf_s(szDate, 1024, "%s", __TIME__);

	st.wHour = (szDate[0] - '0') * 10 + (szDate[1] - '0');
	st.wMinute = (szDate[3] - '0') * 10 + (szDate[4] - '0');
	st.wSecond = (szDate[6] - '0') * 10 + (szDate[7] - '0');
	sprintf_s(version, 1024, "%s.%02d.%04d.%d%02d%02d%02d%02d%02d", VERSION_MODULE, MODULE_CLASS, SUBVERSION, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}


void robot_idcardys_module::run()
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

			idcardys_message *rsmsg = (idcardys_message *)msg.msgObject;

			if (has_task != 0)
			{
				now.update();
				if (m_idysreader.Send_OpenDev(m_port))
				{
					rsmsg->return_state = 4012;
					std::string errtemp = m_idysreader.get_error();
					common_gbk_2utf8(&errtemp, &rsmsg->ErrorInfo);
					DevState = -1;
				}
				else
				{
					has_info = false;
					task_client_guid = rsmsg->fromguid;
					rsmsg->return_state = 0;
					has_task = 0;
					timeout = rsmsg->timeout;
					DevState = 1;
				}

			}
			else
				rsmsg->return_state = 0;

			rsmsg->content = m_json.idcardys_wait(*rsmsg);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, rsmsg->content.c_str(), rsmsg->content.length(), rsmsg->fromguid.c_str());
			SAFE_DELETE(rsmsg);
		}
		break;
		case	1:
		{
			if (!msg.msgObject) break;

			idcardys_message *rsmsg = (idcardys_message *)msg.msgObject;

			has_task = -1;
			has_info = false;

			m_idysreader.Send_CloseDev();
			DevState = 0;

			rsmsg->return_state = 0;
			rsmsg->content = m_json.idcardys_cancel(rsmsg->return_state);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, rsmsg->content.c_str(), rsmsg->content.length(), rsmsg->fromguid.c_str());
			SAFE_DELETE(rsmsg);
		}
		break;
		case 2:
		{
			if (!msg.msgObject) break;

			if (DevState == -1 || DevState == 0)
			{
				if (m_idysreader.Send_OpenDev(m_port))
					DevState = -1;
				else
				{
					DevState = 0;
					m_idysreader.Send_CloseDev();
				}
			}

			idcardys_message *rsmsg = (idcardys_message *)msg.msgObject;

			rsmsg->return_state = 0;
			rsmsg->DevState = get_devstate();
			rsmsg->content = m_json.stateinfo_result(*rsmsg);
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
			DevState = 2;
			if (!m_idysreader.Send_FindCard())
			{
				if (!m_idysreader.Send_ChooseCard())
				{
					if (!m_idysreader.Send_ReadIDData())
					{
						has_info = true;
					}
						
				}
			}

			if (has_info || now.isreach(timeout * 1000))
			{
				idcardys_message rsmsg;
				rsmsg.fromguid = task_client_guid;
				if (has_info)
				{
					if (strlen(m_idysreader.PRTDInfo.szCnName) != 0)
					{
						rsmsg.Address = m_idysreader.PRTDInfo.szAddress;
						rsmsg.EnName = m_idysreader.PRTDInfo.szEnName;
						rsmsg.IDNum = m_idysreader.PRTDInfo.szIDNum;
						rsmsg.IdType = m_idysreader.PRTDInfo.szType;
						rsmsg.IdVersion = m_idysreader.PRTDInfo.szVersion;
						rsmsg.CnName = m_idysreader.PRTDInfo.szCnName;
						rsmsg.Issued = m_idysreader.PRTDInfo.szIssued;
						rsmsg.BornDay = m_idysreader.PRTDInfo.szBornDay;
						rsmsg.BeginValidity = m_idysreader.PRTDInfo.szBeginValidity;
						rsmsg.EndValidity = m_idysreader.PRTDInfo.szEndValidity;
						rsmsg.Age = m_idysreader.PRTDInfo.Age;
						rsmsg.isForeigners = true;

						if (strcmp(m_idysreader.PRTDInfo.szSex, "ÄÐ") == 0)
							rsmsg.Sex = 1;
						if (strcmp(m_idysreader.PRTDInfo.szSex, "Å®") == 0)
							rsmsg.Sex = 2;
					}
					if (strlen(m_idysreader.GATInfo.szName) != 0)
					{
						rsmsg.Address = m_idysreader.GATInfo.szAddress;
						rsmsg.IDNum = m_idysreader.GATInfo.szIDNum;
						rsmsg.IdType = m_idysreader.GATInfo.szIdType;
						rsmsg.CnName = m_idysreader.GATInfo.szName;
						rsmsg.Issued = m_idysreader.GATInfo.szIssued;
						rsmsg.BornDay = m_idysreader.GATInfo.szBornDay;
						rsmsg.BeginValidity = m_idysreader.GATInfo.szBeginValidity;
						rsmsg.EndValidity = m_idysreader.GATInfo.szEndValidity;
						rsmsg.Age = m_idysreader.GATInfo.Age;
						rsmsg.isForeigners = true;

						if (strcmp(m_idysreader.GATInfo.szSex, "ÄÐ") == 0)
							rsmsg.Sex = 1;
						if (strcmp(m_idysreader.GATInfo.szSex, "Å®") == 0)
							rsmsg.Sex = 2;

						rsmsg.FigLen = m_idysreader.GATInfo.FingerLen;

						common_base64_encode_string((unsigned char*)m_idysreader.GATInfo.IDFinger, 1024, &rsmsg.FigData);
					}
					else
					{
						rsmsg.Address = WcharToChar(m_idysreader.IDInfo.szAddress);
						rsmsg.IDNum = WcharToChar(m_idysreader.IDInfo.szIDNum);
						rsmsg.CnName = WcharToChar(m_idysreader.IDInfo.szName);
						rsmsg.Nation = m_idysreader.IDInfo.szNation;
						rsmsg.Issued = WcharToChar(m_idysreader.IDInfo.szIssued);
						rsmsg.BornDay = WcharToChar(m_idysreader.IDInfo.szBornDay);
						rsmsg.BeginValidity = WcharToChar(m_idysreader.IDInfo.szBeginValidity);
						rsmsg.EndValidity = WcharToChar(m_idysreader.IDInfo.szEndValidity);
						rsmsg.Age = m_idysreader.IDInfo.Age;
						rsmsg.isForeigners = false;


						if (strcmp(m_idysreader.IDInfo.szSex, "ÄÐ") == 0)
							rsmsg.Sex = 1;
						if (strcmp(m_idysreader.IDInfo.szSex, "Å®") == 0)
							rsmsg.Sex = 2;

						rsmsg.FigLen = m_idysreader.IDInfo.FingerLen;

						common_base64_encode_string((unsigned char*)m_idysreader.IDInfo.IDFinger, 1024, &rsmsg.FigData);
					}

					common_trim(&rsmsg.Address);
					common_trim(&rsmsg.CnName);
					common_trim(&rsmsg.Nation);
					common_trim(&rsmsg.IDNum);
					common_trim(&rsmsg.EnName);
					common_trim(&rsmsg.Issued);
					common_trim(&rsmsg.IdType);
					common_trim(&rsmsg.IdVersion);
					common_trim(&rsmsg.NewAdd);

					common_gbk_2utf8(&rsmsg.Address, &rsmsg.Address);
					common_gbk_2utf8(&rsmsg.CnName, &rsmsg.CnName);
					common_gbk_2utf8(&rsmsg.IDNum, &rsmsg.IDNum);
					common_gbk_2utf8(&rsmsg.IdType, &rsmsg.IdType);
					common_gbk_2utf8(&rsmsg.IdVersion, &rsmsg.IdVersion);
					common_gbk_2utf8(&rsmsg.EnName, &rsmsg.EnName);
					common_gbk_2utf8(&rsmsg.Nation, &rsmsg.Nation);
					common_gbk_2utf8(&rsmsg.Issued, &rsmsg.Issued);
					common_gbk_2utf8(&rsmsg.NewAdd, &rsmsg.NewAdd);

				}
				else
				{
					if (m_idysreader.get_error().size() != 0)
					{
						rsmsg.ErrorInfo = m_idysreader.get_error();
					}
					else
						rsmsg.ErrorInfo = "timeout";
				}
				rsmsg.return_state = has_info ? 0 : 4022;
				rsmsg.content = m_json.idcardys_result(rsmsg);
				robot_PostMessage::getPostManager()->post(MODULE_NAME, rsmsg.content.c_str(), rsmsg.content.length(), rsmsg.fromguid.c_str());
				has_task = -1;
				has_info = false;
				m_idysreader.Send_CloseDev();
				DevState = 0;
			}
		}

	}
}


std::string robot_idcardys_module::get_temp_path()
{
	std::string path;
	common_get_current_dir(&path);
	common_fix_path(&path);
	path.append(IDCARD_TEMP_PATH);//sql

	if (!common_isDirectory(path.c_str()))
		common_create_dir(path.c_str());
	return path;
}

int robot_idcardys_module::get_devstate()
{
	return DevState;
}

