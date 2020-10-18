#include "iotServerForLoRaSerial.h"
#include "iot.h"
#include "robot_PostMessage.h"


iotServerForLoRaSerial::iotServerForLoRaSerial(Thread_helper * t)
{
	m_isopen = false;
	_parent = t;
}

void iotServerForLoRaSerial::getid(std::map<std::string, std::string> &list)
{
	map_lk.lock();
	list = IDList;
	map_lk.unlock();
}

int iotServerForLoRaSerial::Read(std::string &data, int timeout)
{
	long first, last;
	int interva;
	data.clear();
	bool isEnd = false;
	first = GetTickCount();
	while (1)
	{
		last = GetTickCount();
		interva = last - first;
		if (interva > timeout)
			break;

		while (1)
		{
			char temp[2] = { 0 };
			if (!m_com.ReadChar(*temp))
				break;
			if (temp[0] == 0x20)
			{
				isEnd = true;
				m_com.ReadChar(*temp);//remove end 0x00
				break;
			}
			data.push_back(temp[0]);
		}
		if (isEnd)
			break;
	}
	Sleep(200);
	if (isEnd)
		return 0;

	return 1;
}

iotServerForLoRaSerial::~iotServerForLoRaSerial()
{
}

void iotServerForLoRaSerial::run()
{
	int has_task = 0;
	Timer_helper now;
	clean_msgqueue();

	std::string result;
	int ret = 0;
	int i = 0;
	std::map<std::string, std::string> temp;
	std::map<std::string, int> temp_id;
	while (1)
	{
		Message  msg = getq(has_task);

		msg_context *rsmsg = (msg_context *)msg.msgObject;

		switch (msg.msgType)
		{
		case GETLIST:
		{
			if (!rsmsg)break;
			LOGE("bad message");
			SAFE_DELETE(rsmsg);
			break;
		}
		case OPENDOOR:
		{
			ret = -1;
			if (!rsmsg)break;
			if (rsmsg->door_list.size() >0)
				ret = OpenDoor(rsmsg->id, rsmsg->door_list);
			result = JsonPackage(rsmsg->id, msg.msgType, ret);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, result.c_str(), result.length(), rsmsg->guid.c_str());
			SAFE_DELETE(rsmsg);
			if (_parent)_parent->pushq(10024);
			break;
		}
		case GETDOORSTATUS:
		{
			ret = -1;
			if (!rsmsg)break;
			std::vector<std::string> state;
			ret = Get_DoorState(rsmsg->id, state);
			result = JsonPackage(rsmsg->id, msg.msgType, state, ret);

			robot_PostMessage::getPostManager()->post(MODULE_NAME, result.c_str(), result.length(), rsmsg->guid.c_str());
			SAFE_DELETE(rsmsg);
			if (_parent)_parent->pushq(10025);
			break;
		}
		case GETPAPERSTATUS:
		{
			ret = -1;
			if (!rsmsg)break;
			std::vector<std::string> state;
			ret = Get_PaperState(rsmsg->id, state);
			result = JsonPackage(rsmsg->id, msg.msgType, state, ret);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, result.c_str(), result.length(), rsmsg->guid.c_str());
			SAFE_DELETE(rsmsg);
			if (_parent)_parent->pushq(10025);
			break;
		}
		case GETBOXBATTERY:
		{
			ret = -1;
			if (!rsmsg)break;
			int battery = 0;
			ret = Get_Bat(rsmsg->id, &battery);
			result = JsonPackage(rsmsg->id, msg.msgType, battery, ret);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, result.c_str(), result.length(), rsmsg->guid.c_str());
			SAFE_DELETE(rsmsg);
			if (_parent)_parent->pushq(10025);
			break;
		}
		case -1:
		{
			if (i != -1)
			{
				std::string boxid, wifiid,typedop;
				ret = Get_IDList(i, typedop, boxid, wifiid);
				if (ret == 0)
				{
					temp_id[boxid] = i;
					temp[boxid] = wifiid;
					i++;
				}
				else if (ret == 2)
				{
					i++;
				}
				else
				{
					has_task = 1000;
					i = -1;
				}
					

				if (i == 6)
				{
					has_task = 1000;
					i = -1;
					map_lk.lock();
					IDList = temp;
					map_lk.unlock();
					for (auto it = temp.begin(); it != temp.end(); it++)
					{
						IDListState[it->first].wifiid = it->second;
						IDListState[it->first].id = temp_id[it->first];
					}
					temp.clear();
					temp_id.clear();
				}
			}	
			break;
		}
		case _EXIT_THREAD_:
		{			
			CloseCom();
			return;
			break;
		}
		default:
		{
			LOGE("unknow Lora msg type %d", msg.msgType);
			if (_parent)_parent->pushq(10026);
		}
		}

		if (now.isreach(2*60*1000))
		{
			has_task = 0; i = 0;
			now.update();
		}
		
			
	}
}

void iotServerForLoRaSerial::CloseCom()
{
	if (!m_isopen)
		return;
	m_com.closePort();

	m_isopen = false;
}

void iotServerForLoRaSerial::insert_head(char * send_data, std::string &widifid)
{
	send_data[0] = 0x00;
	if (widifid.size()>0)
		send_data[1] = widifid.data()[0];
}

bool iotServerForLoRaSerial::opencom()
{
	if (m_isopen)return m_isopen;

	int port;
	if (!common_config_get_field_int("as30m", "port", &port))
		port = 5;

	m_isopen = m_com.InitPort(port, 115200);

	if (!m_isopen)
		LOGBE("LoRa module open failed.");
	return m_isopen;
}

int iotServerForLoRaSerial::try_open_wifiid(std::string id, char * send_data)
{
	std::string widifid;

	if (!m_com.isOpen() && !opencom())
		return 9;

	if (IDList.find(id) == IDList.end())
		return 3;

	widifid = IDList[id];

	if (widifid.empty())
		return 3;
	insert_head(send_data, widifid);
	return 0;
}

int iotServerForLoRaSerial::write_wait_read(std::string &id, char * buffer, int len, std::string &rersult)
{
	rersult.clear();
	int ret = m_com.WriteData(buffer, len);
	if (!ret)
	{
		LOGBD("%s WriteData error %d", id.c_str(),ret);
		IDListState[id].failedcount++;
		CloseCom();
		return 1;
	}

	ret = Read(rersult);
	if (ret != 0)
	{
		LOGBD("%s ReadData timeOut ", id.c_str());
		IDListState[id].failedcount++;
		return 2;
	}
	return 0;
}

void iotServerForLoRaSerial::errorindex(int index)
{
	for (auto it = IDListState.begin(); it != IDListState.end(); it++)
	{
		if (it->second.id == index)it->second.failedcount++;
	}
}
void iotServerForLoRaSerial::successindex(int index)
{
	for (auto it = IDListState.begin(); it != IDListState.end(); it++)
	{
		if (it->second.id == index)it->second.succcesscount++;
	}
}
void iotServerForLoRaSerial::successid(std::string id)
{
	for (auto it = IDListState.begin(); it != IDListState.end(); it++)
	{
		if (it->second.wifiid == id)it->second.succcesscount++;
	}
}
void iotServerForLoRaSerial::errorid(std::string id)
{
	for (auto it = IDListState.begin(); it != IDListState.end(); it++)
	{
		if (it->second.wifiid == id)it->second.failedcount++;
	}
}

int iotServerForLoRaSerial::Get_IDList(int index, std::string &type,std::string & boxid, std::string &widifid)
{
	if (!m_com.isOpen() && !opencom())
		return 9;
	type.clear();
	boxid.clear();
	widifid.clear();
	//char boxID[10] = { 0 };
	std::string recvdata;
	
	char send_data[10] = { 0x00, 0x00, 'G', 'e', 't', 'I', 'D', 0x00 };

	send_data[1] = (char)(0xa0 + (char)index);

	if (!m_com.WriteData(send_data,8))
	{
		LOGBD("%d Get_IDList write failed", send_data[1]);
		errorindex(index);
		CloseCom();
		return 1;
	}

	int ret = Read(recvdata, 1000);
	if (ret)
	{
		LOGBD("%d Get_IDList ReadData timeOut ", send_data[1]);
		errorindex(index);
		return 2;
	}
	if (recvdata.size()<6)
	{
		LOGBD("%d Get_IDList break data failed", send_data[1]);
		errorindex(index);
		return 3;
	}

	successindex(index);

	//LOGBD("Get_IDList RecvData %s", recvdata.c_str() + 2);
	//00 A0 42 31 30 31 20 00
	//memcpy(boxID, recvdata.data() + 3, 3);

	char widi = recvdata.data()[1];
	char types = recvdata.data()[2];
	type.push_back(types);
	widifid.push_back(widi);
	printf("wifiid:%02x\n", (char)widi);
	//boxid = boxID;
	boxid.assign(recvdata.data() + 3, recvdata.data() + 6);
	return 0;
}

int iotServerForLoRaSerial::OpenDoor(std::string id, std::list<std::pair<int, int>> & openlist)
{
	char send_data[20] = { 0x00 };
	int ret = try_open_wifiid(id, send_data);
	if (ret)return ret;

	strcat(send_data + 2, "OpenDoor");

	int index = 10;

	char mixRect[BOX_MAX_LAYER][2];
	memset(mixRect, '0', 2 * BOX_MAX_LAYER*sizeof(char));
	for (auto it = openlist.begin(); it != openlist.end(); it++)
	{
		if (it->first < BOX_MAX_LAYER)
			mixRect[it->first][it->second] = '1';
		else
			LOGBE("over max limit %d---%d", it->first, it->second);
	}

	for (int i = 0; i < BOX_MAX_LAYER; i++)
	{
		*(send_data + index++) = '-';
		*(send_data + index++) = mixRect[i][0];
		*(send_data + index++) = mixRect[i][1];
	}
	*(send_data + index++) = '\0';

	LOGBD("Open_door WriteData %d  %s", (unsigned char)send_data[1], send_data+2);

	std::string  RecvData;
	ret = write_wait_read(id, send_data, index, RecvData);
	if (ret)return ret;

	if (RecvData.size() < 4)
	{
		LOGBD("Open_door failed %d", RecvData.size());
		IDListState[id].failedcount++;
		return 3;
	}
	LOGBD("Open_door RecvData %d   %s", (unsigned char)RecvData[1], RecvData.data() + 2);
	if (memcmp(RecvData.data() + 2, "OK", 2) != 0)
	{
		LOGBD("Open_door failed %s", RecvData.data() + 2);
		IDListState[id].failedcount++;
		return 6;
	}
	IDListState[id].succcesscount++;
	return 0;
}

int iotServerForLoRaSerial::Get_DoorState(std::string id, std::vector<std::string> &state)
{
	char send_data[20] = { 0x00 };
	int ret = try_open_wifiid(id, send_data);
	if (ret)return ret;

	strcat(send_data + 2, "GetDoor\00");
	LOGBD("Get DoorState WriteData %d   %s", (unsigned char)send_data[1], send_data + 2);

	std::string  RecvData;
	ret = write_wait_read(id, send_data, 10, RecvData);
	if (ret)return ret;
	//00 a0 D-00-00 20 00
	if (RecvData.size() < 3)
	{
		LOGBD("Get DoorState failed %d", RecvData.size());
		IDListState[id].failedcount++;
		return 3;
	}
	LOGBD("Get DoorState RecvData %u   %s", (unsigned char)RecvData[1], RecvData.data() + 2);
	RecvData.erase(0,2);
	state = split(RecvData, '-');
	if (state.size() <2)
	{
		IDListState[id].failedcount++;
		LOGBD("Get DoorState failed ");
		return 3;
	}
	IDListState[id].succcesscount++;
	return 0;
}

int iotServerForLoRaSerial::Get_PaperState(std::string id, std::vector<std::string> &state)
{
	char send_data[20] = { 0x00 };
	int ret = try_open_wifiid(id, send_data);
	if (ret)return ret;

	strcat(send_data + 2, "GetPaper\00");
	LOGBD("Get_PaperState WriteData %d  %s", (unsigned char)send_data[1], send_data + 2);

	std::string  RecvData;
	ret = write_wait_read(id, send_data, 11, RecvData);
	if (ret)return ret;
	//00 a0 P-0-0 20 00
	if (RecvData.size() < 3)
	{
		LOGBD("Get PaperState failed %d", RecvData.size());
		IDListState[id].failedcount++;
		return 3;
	}
	LOGBD("Get PaperState RecvData %d   %s", (unsigned char)RecvData[1], RecvData.data() + 2);
	RecvData.erase(0, 2);
	state = split(RecvData, '-');
	if (state.size() <2)
	{
		IDListState[id].failedcount++;
		LOGBD("Get PaperState failed ");
		return 3;
	}
	IDListState[id].succcesscount++;

// 	std::string sData = "";
// 	unsigned char data[3] = { 0 };
// 	memcpy(data, RecvData.data() + 3, 1);
// 	sData = (char*)data;
// 	state.push_back(sData);
// 	memcpy(data, RecvData.data() + 5, 1);
// 	sData = (char*)data;
// 	state.push_back(sData);
// 
// 	LOGBD("Get_PaperState RecvData %d  %s", RecvData[1], RecvData.data() + 2);
// 	IDListState[id].succcesscount++;
	return 0;
}

int iotServerForLoRaSerial::Get_Bat(std::string id, int *battery)
{
	char send_data[20] = { 0x00,0xa0 };
 	int ret = try_open_wifiid(id, send_data);
	if (ret)return ret;

	strcat(send_data + 2, "GetBat\00");

	LOGBD("Get_Bat WriteData %d  %s", (unsigned char)send_data[1], send_data + 2);

	std::string  RecvData;
	ret = write_wait_read(id, send_data, 9, RecvData);
	if (ret || RecvData.size()<5)
	{
		LOGBD("Get bat failed %d", RecvData.size());
		IDListState[id].failedcount++;
		return 3;
	}
	LOGBD("Get bat RecvData %d   %s", (unsigned char)RecvData[1], RecvData.data() + 2);
	RecvData.erase(0, 2);
	std::vector<std::string> bat = split(RecvData, '-');
	if (bat.size() < 2)
	{
		IDListState[id].failedcount++;
		LOGBD("Get bat failed ");
		return 3;
	}
	bat.erase(bat.begin());
	*battery = atoi(bat[0].c_str());

	IDListState[id].succcesscount++;
// 
// 	char data[10] = { 0 };
// 	memcpy(data, RecvData.data() + 4, RecvData.length()-4);
//
// 
// 	LOGBD("Get_Bat RecvData %02x  %s", RecvData[1], RecvData.data() + 2);
// 	IDListState[id].succcesscount++;
	return 0;
}

