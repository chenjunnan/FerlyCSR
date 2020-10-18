#include "robot_face_template.h"
#include <fstream>
#include <iostream>
#include <fstream>
#include <string>
#include <iosfwd>
#include "common_helper.h"
#include "json\json.h"


robot_face_template::robot_face_template()
{
	m_l = common_mutex_create_threadlock();
	readfacedb();
}


robot_face_template * robot_face_template::getInstance()
{
	static robot_face_template _instance;
	return  &_instance;
}


void robot_face_template::add_feature_handler(Thread_helper * cb)
{
	LOCK_HELPER_AUTO_HANDLER(m_l);
	for (auto it = m_cb_list.begin(); it != m_cb_list.end(); it++)
	{
		if (*it == cb)
			return;
	}
	m_cb_list.push_back(cb);
	cb->rpushq(FACE_UPDATA_HANDLE_TASK);
}


void robot_face_template::remove_feature_handler(Thread_helper *cb)
{
	LOCK_HELPER_AUTO_HANDLER(m_l);
	for (auto it = m_cb_list.begin(); it != m_cb_list.end(); it++)
	{
		if (*it == cb)
		{
			m_cb_list.erase(it);
			return;
		}
	}
}

void robot_face_template::UpdateTemplate(std::map<std::string, FaceDatabaseFormat> &facelist)
{
	LOCK_HELPER_AUTO_HANDLER(m_l);
	facelist.clear();
	facelist = m_facelist;
}

void robot_face_template::setTemplate(std::string guid, FaceDatabaseFormat &data)
{
	LOCK_HELPER_AUTO_HANDLER(m_l);
	m_facelist[guid] = data;
	for (auto it = m_cb_list.begin(); it != m_cb_list.end(); it++)
		(*it)->rpushq(FACE_UPDATA_HANDLE_TASK);
	writefacedb();
}


void robot_face_template::delTemplatebyName(std::string name)
{
	LOCK_HELPER_AUTO_HANDLER(m_l);
	if (m_facelist.find(name) != m_facelist.end())
	{
		m_facelist.erase(name);

	}
	else
	{
		auto it = m_facelist.begin();
		for (; it != m_facelist.end(); )
		{
			if (it->second.name.compare(name) == 0)
				it=m_facelist.erase(it);
			else
				it++;
		}
		
	}
	for (auto it = m_cb_list.begin(); it != m_cb_list.end(); it++)
		(*it)->rpushq(FACE_UPDATA_HANDLE_TASK);
	writefacedb();
}

void robot_face_template::clearTemplate()
{
	LOCK_HELPER_AUTO_HANDLER(m_l);
	m_facelist.clear();
	for (auto it = m_cb_list.begin(); it != m_cb_list.end(); it++)
		(*it)->rpushq(FACE_UPDATA_HANDLE_TASK);
	writefacedb();
}

void robot_face_template::readfacedb()
{
	LOCK_HELPER_AUTO_HANDLER(m_l);
	m_facelist.clear();
	std::ifstream ifi;
	ifi.open("facetemplate.db", std::ios::in);
	if (ifi.fail())
		return;


	std::string readInfo;
	while (std::getline(ifi, readInfo))
	{
		Json::Value   json_object;
		Json::Reader  json_reader;
		std::string guid;

		bool b_json_read = json_reader.parse(readInfo.c_str(), json_object);
		if (b_json_read)
		{

			FaceDatabaseFormat face_database;

			if (json_object.isMember("guid") && json_object["guid"].isString())
				guid = json_object["guid"].asString();
			if (json_object.isMember("name") && json_object["name"].isString())
				face_database.name = json_object["name"].asString();
			if (json_object.isMember("sex") && json_object["sex"].isInt())
				face_database.sex = json_object["sex"].asInt();
			if (json_object.isMember("age") && json_object["age"].isInt())
				face_database.age = json_object["age"].asInt();
			if (json_object.isMember("userdata") && json_object["userdata"].isString())
				face_database.userdata = json_object["userdata"].asString();
			if (json_object.isMember("id") && json_object["id"].isInt())
				face_database.id = json_object["id"].asInt();



			if (json_object.isMember("features") && json_object["features"].isArray())
			{
				Json::Value json_feature = json_object["features"];

				for (int i = 0; i < json_feature.size(); i++)
				{
					if (json_feature[i].isObject())
					{
						FaceFeatureData temp;

						if (json_feature[i].isMember("typename") && json_feature[i]["typename"].isString())
							temp.algtype = json_feature[i]["typename"].asString();

						if (json_feature[i].isMember("cameraid") && json_feature[i]["cameraid"].isInt())
							temp.cameraid = json_feature[i]["cameraid"].asInt();

						if (json_feature[i].isMember("feature") && json_feature[i]["feature"].isString())
						{
							std::vector<unsigned char > buffer;
							std::string  features_fstr = json_feature[i]["feature"].asString();
							int ret = common_base64_decode_string(&features_fstr, &buffer);
							if (ret != 0)
							{
								for (int i = 0; i < buffer.size(); i++)
									temp.face_feature.push_back((char)buffer[i]);

								if (!temp.algtype.empty())
								{
									if (temp.algtype.compare("yuncong") == 0 && temp.face_feature.size() == 521)
										temp.face_feature.pop_back();

									face_database.face_feature.push_back(temp);
								}
									

							}

						}

					}

				}
			}

			if (face_database.face_feature.size() != 0 && !guid.empty())
				m_facelist[guid] = face_database;
		}

	}
	ifi.close();
}


void robot_face_template::writefacedb()
{
	int tempn = 0;
	if (!common_config_get_field_int("Face", "regfacememory", &tempn))
		tempn = 0;
	if (tempn)
		return;

	std::ofstream ifi;
	ifi.open("facetemplate.db", std::ios::out);
	if (ifi.fail())
		return;

	auto it = m_facelist.begin();

	for (; it != m_facelist.end(); it++)
	{
		Json::Value   json_object;
		Json::FastWriter json_writer;

		json_object["guid"] = Json::Value(it->first);
		json_object["name"] = Json::Value(it->second.name);
		json_object["sex"] = Json::Value(it->second.sex);
		json_object["age"] = Json::Value(it->second.age);
		json_object["userdata"] = Json::Value(it->second.userdata);
		json_object["id"] = Json::Value(it->second.id);
		for (int i = 0; i < it->second.face_feature.size(); i++)
		{
			Json::Value   json_person;
			json_person["typename"] = Json::Value(it->second.face_feature[i].algtype);
			json_person["cameraid"] = Json::Value(it->second.face_feature[i].cameraid);
			std::string  out;
			common_base64_encode_string((unsigned char*)it->second.face_feature[i].face_feature.data(), it->second.face_feature[i].face_feature.size(), &out);
			json_person["feature"] = Json::Value(out);
			json_object["features"].append(json_person);
		}
		std::string out = json_writer.write(json_object).c_str();
		out.append("\n");

		ifi << out;
	}
	ifi.close();
}

robot_face_template::~robot_face_template()
{
	common_mutex_threadlock_release(m_l);
}
