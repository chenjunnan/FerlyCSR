#pragma once
#include <string>
#include "json\json.h"
#include "robot_face_struct.h"
class robot_json_rsp
{
public:
	robot_json_rsp(){}
	~robot_json_rsp(){}
	std::string robot_json_rsp::start_face_service(int errorcode,int cameraid)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("START_FACE_DETECT_SERVICE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		json_object["cameraid"] = Json::Value(cameraid);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::stop_face_service(int errorcode, int cameraid)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("STOP_FACE_DETECT_SERVICE_RESPONSE");
		json_object["cameraid"] = Json::Value(cameraid);
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::face_detect_ntf(std::map<int, CameraCacheInfo> &data, bool state)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_DETECT_NOTIFY");
		json_object["hasperson"] = Json::Value(state);

		auto it = data.begin();

		for (; it != data.end(); it++)
		{
			Json::Value  json_face;
			json_face["cameraid"] = it->first;
			json_face["hasperson"] = it->second.face_status;
			json_object["list"].append(json_face);
		}
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::face_snapshot(FaceMessage &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_SNAPSHOT_RESPONSE");
		json_object["error"] = Json::Value(data.error);
		json_object["savetype"] = Json::Value(data.savetype);
		if (data.cameralist.size()!=0)
			json_object["cameraid"] = Json::Value(data.cameralist[0]);
		if (data.savetype != 0)
		{
			for (int i = 0; i < data.list.size(); i++)
			{
				Json::Value onface;
				onface["id"] = data.list[i].id;
				onface["data"] = Json::Value(data.list[i].data);
				json_object["snapresult"].append(onface);
			}
		}

		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::live_snapshot(FaceMessage &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("CAMERA_SNAPSHOT_RESPONSE");
		json_object["error"] = Json::Value(data.error);
		if (data.error == 0)
		{
			json_object["savetype"] = Json::Value(data.savetype);
			if (data.cameralist.size() != 0)
				json_object["cameraid"] = Json::Value(data.cameralist[0]);
			if (data.savetype != 0 && data.list.size()>0)
				json_object["snapresult"] = Json::Value((data.list[0].data));
		}


		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::cancel_face_snapshot(int errorcode,int cameraid)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_SNAPSHOT_CANCEL_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		json_object["cameraid"] = Json::Value(cameraid);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::identify_face_begin(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_IDENTIFY_BEGIN_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::identify_face_stop(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_IDENTIFY_STOP_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::reg_face_begin(FaceMessage &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_REGISTER_RESPONSE");
		json_object["error"] = Json::Value(data.error);


		for (int i = 0; i < data.list.size(); i++)
			json_object["failed"].append(Json::Value(data.list[i].name));
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::reg_face_begin(int error, std::vector<std::string> &flist, std::map<std::string, std::string > &slist)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_REGISTER_RESPONSE");
		json_object["error"] = Json::Value(error);


		for (auto it = slist.begin(); it != slist.end(); it++)
		{
			Json::Value   json_object_one;
			json_object_one["name"] = Json::Value(it->second);
			json_object_one["guid"] = Json::Value(it->first);
			json_object["success"].append(json_object_one);
		}
			

		for (int i = 0; i < flist.size(); i++)
			json_object["failed"].append(Json::Value(flist[i]));

		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::identify_face_ntf(FaceMessage &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_IDENTIFY_NOTIFY");
		json_object["error"] = Json::Value(data.error);
		if (data.cameralist.size() != 0)
			json_object["cameraid"] = Json::Value(data.cameralist[0]);

		for (int i = 0; i < data.list.size(); i++)
		{
			Json::Value  json_face;
			json_face["id"] = Json::Value(data.list[i].id);
			if (data.list[i].face_reltimes.age/* != 1*/)
			{
				json_face["age"] = Json::Value(data.list[i].face_reltimes.age);
				json_face["sex"] = Json::Value(data.list[i].face_reltimes.sex);
				json_face["smile"] = Json::Value(data.list[i].face_reltimes.smile);
			}	
			if (!data.list[i].userdata.empty())
				json_face["userdata"] = Json::Value(data.list[i].userdata);
			json_face["confidence"] = Json::Value(data.list[i].confidence);
			json_face["name"] = Json::Value(data.list[i].name);		
			json_face["guid"] = Json::Value(data.list[i].guid);
			json_object["face_list"].append(json_face);
		}

		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::debug_face_show(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_DEBUG_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::face_info_begin(int errorcode, int cameraid)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_FACEINFO_BEGIN_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		json_object["cameraid"] = Json::Value(cameraid);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::face_info_stop(int errorcode, int cameraid)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_FACEINFO_STOP_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		json_object["cameraid"] = Json::Value(cameraid);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::face_info_notify(int error, std::map<int, CameraCacheInfo> &data)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_FACEINFO_NOTIFY");
		json_object["error"] = Json::Value(error);

		auto it = data.begin();

		for (; it != data.end();it++)
		{
			Json::Value  json_face;
			json_face["cameraid"] = it->first;
			for (int i = 0; i < it->second.realtime_position.size(); i++)
			{
				Json::Value  json_face_one;
				if (it->second.mapid.find(it->second.realtime_position[i].trackId) != it->second.mapid.end())
					json_face_one["id"] = Json::Value(it->second.mapid[it->second.realtime_position[i].trackId]);
				else
					json_face_one["id"] = Json::Value(it->second.realtime_position[i].trackId);
				

				if (it->second.realtime_position[i].age/* > 1*/)
				{
					json_face_one["age"] = Json::Value(it->second.realtime_position[i].age);
					json_face_one["sex"] = Json::Value(it->second.realtime_position[i].sex);
					json_face_one["smile"] = Json::Value(it->second.realtime_position[i].smile);
				}

				if (/*it->second.realtime_position[i].valid&&*/it->second.realtime_position[i].has3D)
				{
					json_face_one["roll"] = Json::Value(it->second.realtime_position[i].roll);
					json_face_one["pitch"] = Json::Value(it->second.realtime_position[i].pitch);
					json_face_one["yaw"] = Json::Value(it->second.realtime_position[i].yaw);
				}


				json_face_one["score"] = Json::Value(it->second.realtime_position[i].score);
				json_face_one["islive"] = Json::Value(it->second.realtime_position[i].isLive);
				json_face_one["x"] = Json::Value(it->second.realtime_position[i].x);
				json_face_one["y"] = Json::Value(it->second.realtime_position[i].y);
				json_face_one["width"] = Json::Value(it->second.realtime_position[i].width);
				json_face_one["height"] = Json::Value(it->second.realtime_position[i].height);
				json_face_one["xoffset"] = Json::Value(it->second.realtime_position[i].xoffset);
				json_face_one["yoffset"] = Json::Value(it->second.realtime_position[i].yoffset);
				json_face_one["weight"] = Json::Value(it->second.realtime_position[i].height*it->second.realtime_position[i].width );
				json_face["face_list"].append(json_face_one);
			}
			json_object["list"].append(json_face);
		}

		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::remove_face(int errorcode)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_REMOVE_RESPONSE");
		json_object["error"] = Json::Value(errorcode);
		return json_writer.write(json_object);
	}
	std::string robot_json_rsp::get_face_namelist(std::map<std::string, FaceDatabaseFormat>& list)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;
		json_object["msg_type"] = Json::Value("FACE_NAMELIST_RESPONSE");

		for (auto it = list.begin(); it != list.end(); it++)
		{
			Json::Value  json_face;
			json_face["name"] = Json::Value(it->second.name);
			json_face["guid"] = Json::Value(it->first);
			if (it->second.age > 1)
			{
				json_face["age"] = Json::Value(it->second.age);
				json_face["sex"] = Json::Value(it->second.sex);
			}
			if (!it->second.userdata.empty())
				json_face["userdata"] = Json::Value(it->second.userdata);

			for (int i = 0; i < it->second.face_feature.size();i++)
			{
				Json::Value  json_feature;
				json_feature["algorithmtype"] = Json::Value(it->second.face_feature[i].algtype);
				json_feature["cameraid"] = Json::Value(it->second.face_feature[i].cameraid);
				json_face["feature"].append(json_feature);
			}
			json_object["name_list"].append(json_face);
		}
		json_object["error"] = Json::Value(0);
		return json_writer.write(json_object);
	}
}; 