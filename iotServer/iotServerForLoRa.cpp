#include "iotServerForLoRa.h"
#include "iot.h"
#include "robot_PostMessage.h"


iotServerForLoRa::iotServerForLoRa() :_exe(this)
{
	_current_task = 0;
	_state = low;
}


iotServerForLoRa::~iotServerForLoRa()
{
}

void iotServerForLoRa::AppendPortList(Json::Value &json_object_base)
{
	std::map<std::string, std::string> islid;
	_exe.getid(islid);
	for (auto it = islid.begin(); it != islid.end(); it++)
	{
		Json::Value   json_box;
		json_box["id"] = Json::Value(it->first);
		unsigned char temp = 0;
		if (it->second.size() > 0)
			temp = it->second[0];
		json_box["fid"] = Json::Value(temp);
		json_box["access"] = Json::Value("LoRa mode");
		json_object_base["box_list"].append(json_box);
	}
}

std::string iotServerForLoRa::getIotlist(std::map<std::string, std::string > &IDList)
{
	Json::Value   json_object_post;
	Json::FastWriter json_writer;
	json_object_post["msg_type"] = Json::Value("IOT_GETID_LIST_RESPONSE");
	appenderrormessage(json_object_post, 0);
	for (auto it = IDList.begin(); it != IDList.end(); it++)
	{
		Json::Value   json_box;
		//if (it->second.type == DBOX)
		//	json_box["type"] = Json::Value("box");
		//if (it->second.type == DFRAME)
		//	json_box["type"] = Json::Value("frame");
		json_box["id"] = Json::Value(it->first);
		int temp = 0;
		if (it->second.size() > 0)
			temp = it->second[0];
		json_box["fid"] = Json::Value(temp);
		json_object_post["box_list"].append(json_box);
	}
	return json_writer.write(json_object_post);
}

void iotServerForLoRa::run()
{
	bool loop = true;

	clean_msgqueue();

	int waiti = -1;
	bool isOpen = _exe.start();
	if (!isOpen)
	{
		waiti = 1000;
	}

	while (loop)
	{
		Message  msg = getq(waiti);

		switch (msg.msgType)
		{
		case	GETLIST:
		{
			msg_context *msgc = (msg_context *)msg.msgObject;
			if (!msgc)continue;
			std::map<std::string, std::string> islid;
			_exe.getid(islid);
			std::string result = getIotlist(islid);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, result.c_str(), result.length(), msgc->guid.c_str());
			SAFE_DELETE(msgc);
		}break;
		case	OPENDOOR:
		{
			if (_state == high)
			{
				if (highlist.size() <= 20)
					highlist.push_back(msg);
				else
				{
					msg_context *msgc = (msg_context *)msg.msgObject;
					if (!msgc)continue;
					std::string result = JsonPackage(msgc->id, msg.msgType, 5);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, result.c_str(), result.length(), msgc->guid.c_str());
					SAFE_DELETE(msgc);
				}
			}
			else
			{
				_exe.pushq(msg);
				_state = high;
				_current_task++;
			}
		}
		break;
		case	CLOSEDOOR:
		case	GETDOORSTATUS:
		case	GETPAPERSTATUS:
		case	GETBOXBATTERY:
		{
			if (_state != low)
			{
				if (midlist.size() <= 20)
					midlist.push_back(msg);
				else
				{
					msg_context *msp = (msg_context *)msg.msgObject;
					if (!msp)continue;
					std::string result = JsonPackage(msp->id, msg.msgType, 5);
					robot_PostMessage::getPostManager()->post(MODULE_NAME, result.c_str(), result.length(), NULL);
					SAFE_DELETE(msp);
				}
			}
			else
			{
				_exe.pushq(msg);
				_state = mid;
				_current_task++;
			}

		}
		break;
		case 10024:
		case 10026:
		{
			_current_task--;
			if (highlist.size() != 0)
			{
				_exe.pushq(*highlist.begin());
				highlist.pop_front();
				_state = high;
				_current_task++;
			}
			else
			{
				_state = mid;
				if (_current_task == 0)
				{
					if (midlist.size() != 0)
					{
						_exe.pushq(*midlist.begin());
						midlist.pop_front();
						_state = mid;
						_current_task++;
					}
					else
						_state = low;
				}
			}


		}
		break;
		case 10025:
		{
			_current_task--;
			if (_state != high)
			{
				if (_current_task == 0)
				{
					if (midlist.size() != 0)
					{
						_exe.pushq(*midlist.begin());
						midlist.pop_front();
						_state = mid;
						_current_task++;
					}
					else
						_state = low;
				}
			}
		}
		break;

		case _EXIT_THREAD_:
			loop = false;
			continue;
			break;
		default:
			LOGBE("ERROR msg type :%d", msg.msgType);
			break;
		}

		if (_state == low)
		{
			if (highlist.size() != 0)
			{
				_exe.pushq(*highlist.begin());
				highlist.pop_front();
				_state = high;
				_current_task++;
			}
			else
			{
				if (midlist.size() != 0)
				{
					_exe.pushq(*midlist.begin());
					midlist.pop_front();
					_state = mid;
					_current_task++;
				}
			}
		}
		LOGBI("task 状态 :%d  opendoor wait list :%d normal wait list:%d  当前数量 ：%d", _state, highlist.size(), midlist.size(), _current_task);
		
	}
	_exe.quit();
}
