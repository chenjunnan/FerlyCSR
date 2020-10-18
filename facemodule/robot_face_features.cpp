#include <fstream>
#include <iostream>
#include <fstream>
#include <string>
#include <iosfwd>
#include "robot_face_features.h"
#include "opencv2\highgui\highgui.hpp"
#include "common_helper.h"
#include "json\json.h"
#include "robot_face_template.h"
#include "robot_PostMessage.h"


robot_face_features::robot_face_features()
{
	m_feature_object = NULL;
	m_idastate=true;
	identifyinv = 0;
}

robot_face_features::~robot_face_features()
{
	m_cameraid = 0;
	if (state())quit();
	SAFE_DELETE(m_feature_object);
}

bool robot_face_features::init(int id, std::string alg, robot_face_feature_interface * featureobject)
{
	if (featureobject == NULL)return false;

	SAFE_DELETE(m_feature_object);
	m_feature_object = featureobject;

	if (!common_config_get_field_int(alg.c_str(), "threshold", &m_face_th))
		m_face_th = 760;


	char name_buffer[1024] = { 0 };
	sprintf_s(name_buffer, 1024, "FaceConfig_%d", id);


	if (!common_config_get_field_int(name_buffer, "identifyinv", &identifyinv))
		identifyinv = 0;

	m_cameraid = id;


	return initFaceAlgorithm (alg);

}

bool robot_face_features::can_idenify()
{
	return m_idastate;//TODO syn
}

bool robot_face_features::initFaceAlgorithm (std::string &alg)
{
	if (m_feature_object->init())
	{
		LOGE("%s init failed", alg.c_str());
		SAFE_DELETE(m_feature_object);
		return false;
	}
	return true;
}

bool robot_face_features::features(FaceDetectData  &ptr, FaceDatabaseFormat &f_info, int index/*=-1*/)
{
	if (ptr.face_img.empty())
	{
		LOGE("input face image empty");
		return false;
	}

	FaceFeatureData feature_tempdata;

	if (!m_feature_object->getFeatures(ptr, feature_tempdata, &index))
		return false;


	feature_tempdata.cameraid = m_cameraid;

	//add not default face alg feature???  TODO

	f_info.face_feature.push_back(feature_tempdata);


	f_info.sex = ptr.face_list[index].sex;
	f_info.age = ptr.face_list[index].age;

	return true;
}

void robot_face_features::run()
{
	robot_face_template::getInstance()->add_feature_handler(this);

	while (true)
	{
		m_idastate = true;
		Message msg = getq();
		m_idastate = false;
		switch (msg.msgType)
		{
		case FACE_REGISTER_HANDLE_TASK:
		{
			FaceTask*ptr = (FaceTask*)msg.msgObject;
			face_registerimp(ptr);
			SAFE_DELETE(ptr)
		}
		break;
		case FACE_IDENTIFY_HANDLE_TASK:
		{
			FaceDetectData*ptr = (FaceDetectData*)msg.msgObject;
			face_identifyimp(ptr);
			SAFE_DELETE(ptr)
			
		}
 		break;
		case FACE_UPDATA_HANDLE_TASK:
			UpdateFaceTemplate();
		break;
		case _EXIT_THREAD_:
			robot_face_template::getInstance()->remove_feature_handler(this);
			return;
		case -1:
			break;
		default:
			LOGE("unknow face features message type  %d", msg.msgType);
			break;
		}
	}


}

void robot_face_features::UpdateFaceTemplate()
{
	robot_face_template::getInstance()->UpdateTemplate(m_facedb);
	m_id_guid.clear();
	m_template_cache.clear();
	auto it = m_facedb.begin();
	for (; it != m_facedb.end(); it++)
	{
		for (int j = 0; j < it->second.face_feature.size(); j++)
		{
			if (it->second.face_feature[j].algtype.compare(m_feature_object->m_algname) == 0)
			{
				m_id_guid[m_template_cache.size()] = it->first;
				m_template_cache.push_back(it->second.face_feature[j].face_feature);
			}
		}
	}
}

void robot_face_features::face_registerimp(FaceTask* ptr)
{
	if (ptr == NULL || ptr->task_message.fromguid.empty())
		return;

	std::vector<std::string> failedlist;

	std::map<std::string, std::string > guid_name;

	switch (ptr->task_message.reg_type)
	{
	case MEMORY_FACE_REGISTER:
	case FILE_FACE_REGISTER:
	case BASE64_FACE_REGISTER:
	{
		for (int i = 0; i < ptr->task_message.list.size(); i++)
		{
			FaceDatabaseFormat finfo_temp;
			bool ok = false;
			finfo_temp.name = ptr->task_message.list[i].name;

			if (ptr->input_image_list.size() <= i)
			{
				LOGE("miss image register face index: %d",i);
				break;
			}

			ok = features(ptr->input_image_list[i], finfo_temp, ptr->task_message.list[i].id);

			

			if (ptr->task_message.list[i].face_reltimes.age != 0)
				finfo_temp.age = ptr->task_message.list[i].face_reltimes.age;

			if (ptr->task_message.list[i].face_reltimes.sex != 0)
				finfo_temp.sex = ptr->task_message.list[i].face_reltimes.sex;

			finfo_temp.userdata = ptr->task_message.list[i].userdata;

			std::string guidt;
			if (ok)
				ok = face_insert_map(finfo_temp,guidt);

			if (!ok)
				failedlist.push_back(ptr->task_message.list[i].name);
			else
				guid_name[guidt] = ptr->task_message.list[i].name;
		}

	}
	break;
	default:
		LOGE("unknow face register metho %d", ptr->task_message.reg_type);
	}

	ptr->task_message.list.clear();

	if (failedlist.size() == 0)
		ptr->task_message.error = 0;
	else if (guid_name.size() == 0)
		ptr->task_message.error = FACE_REGISTER_FAILED;
	else
		ptr->task_message.error = FACE_NOALL_REGISTER_FAILED;

	ptr->task_message.content = json_request.reg_face_begin(ptr->task_message.error, failedlist, guid_name);
	robot_PostMessage::getPostManager()->post(MODULE_NAME, ptr->task_message.content.c_str(), ptr->task_message.content.size(), ptr->task_message.fromguid.c_str());
}

bool robot_face_features::face_insert_map(FaceDatabaseFormat &finfo, std::string &oguid)
{
	RealtimeIdentifyFaceInfo  f_i;
	int  score = 0;

	if (check_face_identify_exist(finfo, m_facedb,f_i))
	{
		std::string out;
		common_utf8_2gbk(&f_i.name, &out);
		LOGW("exist similarity face %s", out.c_str());

		bool ret = false;
		if (finfo.name.compare(f_i.name) == 0)
		{
			std::vector<char> features_f2;

			for (int i = 0; i < m_facedb[f_i.data].face_feature.size(); i++)
			{
				if (m_facedb[f_i.data].face_feature[i].algtype.compare(m_feature_object->m_algname) == 0)
				{
					for (int j = 0; j < finfo.face_feature.size();j++)
					{
						if (finfo.face_feature[j].algtype.compare(m_feature_object->m_algname) == 0)
						{
							ret = m_feature_object->MergeFeatures(finfo.face_feature[j].face_feature, m_facedb[f_i.data].face_feature[i].face_feature);
							m_facedb[f_i.data].userdata = finfo.userdata;
						}
					}
				}
					
			}

			
		}
		if (ret)
		{
			robot_face_template::getInstance()->setTemplate(f_i.data, m_facedb[f_i.data]);
			oguid = f_i.data;
			LOGI("name :%s face upgrade success", f_i.data.c_str());
		}
		else
		{
			LOGE("name :%s face insert failed", f_i.data.c_str());
		}
		return ret;
	}
	else
	{
		std::string guid;
		common_get_guid(&guid);
		int i = 0;
		while (1)
		{
			bool unuse = true;
			auto it = m_facedb.begin();
			for (; it != m_facedb.end(); it++)
			{
				if (it->second.id == i)
				{
					unuse = false; break;
				}
			}
			if (unuse) break;
			i++;
		}
		finfo.id = i;// m_face_reglist.size();//sql
		std::string out;
		common_utf8_2gbk(&f_i.name, &out);
		robot_face_template::getInstance()->setTemplate(guid, finfo);
		oguid = guid;
		LOGD("%s register successed [%s--%d]", guid.c_str(), out.c_str(), i);
	}

	return true;
}

bool robot_face_features::check_face_identify_exist(FaceDatabaseFormat &face_feature, std::map<std::string, FaceDatabaseFormat> &reg_facelist, RealtimeIdentifyFaceInfo &face_result)
{
	if (face_feature.face_feature.size() == 0)
		return false;

	bool  match_result = false;
	for (int i = 0; i < face_feature.face_feature.size();i++)
	{
		if (face_feature.face_feature[i].algtype.compare(m_feature_object->m_algname) == 0)
		{
			std::vector<char > temp_c;
			if (!m_feature_object->Features2Identify(face_feature.face_feature[i].face_feature, temp_c))
				continue;
			int maxscore = 0;

			int index = 0;

			double score = m_feature_object->IdentifyTemplate(temp_c, m_template_cache, index);
			int sco = normal_score(score);

			if (sco > m_face_th&&m_id_guid.find(index) != m_id_guid.end())
			{
				face_result.name = reg_facelist[m_id_guid[index]].name;// it->second.name;
				face_result.data = m_id_guid[index];// it->first;
				face_result.confidence = sco;
				maxscore = sco;
			}

			if (maxscore > m_face_th)
				match_result = true;

		}
	}


	return match_result;
	
}

bool robot_face_features::identify(std::vector<std::vector<char> >templecache, std::vector<char>&identifyfeature, int &which ,int &scoreout)
{
 	if (identifyfeature.size() == 0)
 		return false;

	int index = -1;

	double score = m_feature_object->IdentifyTemplate(identifyfeature, templecache, index);
	scoreout = normal_score(score);
	;//printf("-------%d \n", scoreout);
	if (scoreout > m_face_th&&index >= 0 && index<templecache.size())
	{
		which = index;
		return true;
	}
 	return false;
 }
 
 void robot_face_features::face_identifyimp(FaceDetectData* ptr)
 {
	 FaceMessage  fmsg;

 	unsigned int btime = common_get_cur_time_stampms();

 	std::vector<std::string>  now_face_list;
	std::vector<int>		 now_face_list_id;
 	std::vector<std::vector<char>> faceidentifyfeaturelist;

	if (m_feature_object->getAllIdentifyFeatures(*ptr, faceidentifyfeaturelist))
 	{
 		for (int i = 0; i < faceidentifyfeaturelist.size(); i++)
 		{
			if (faceidentifyfeaturelist[i].size() == 0)continue;

			;//printf("enter identify \n");
			RealtimeIdentifyFaceInfo   rtface_info;
			if (identifyinv == 0 || m_identify_cache.find(ptr->face_list[i].trackId) == m_identify_cache.end())
			{
				int which = -1, score = 0;
				if (identify(m_template_cache, faceidentifyfeaturelist[i], which, score))
				{
					if (m_id_guid.find(which) != m_id_guid.end())
					{
/*						rtface_info.id = m_facedb[m_id_guid[which]].id;*/
						rtface_info.name = m_facedb[m_id_guid[which]].name;
						rtface_info.face_reltimes.age = m_facedb[m_id_guid[which]].age;
						rtface_info.face_reltimes.sex = m_facedb[m_id_guid[which]].sex;
						rtface_info.confidence = score;
						rtface_info.userdata = m_facedb[m_id_guid[which]].userdata;
						rtface_info.guid = m_id_guid[which];

						rtface_info.face_reltimes.smile = ptr->face_list[i].smile;
						rtface_info.id = ptr->face_list[i].trackId;
						fmsg.list.push_back(rtface_info);
						now_face_list.push_back(m_id_guid[which]);
						now_face_list_id.push_back(ptr->face_list[i].trackId);

						if (identifyinv != 0)
						{
							if (ptr->face_list[i].trackId != -1)
							{
								m_identify_cache[ptr->face_list[i].trackId].guid = m_id_guid[which];
								m_identify_cache[ptr->face_list[i].trackId].inserttime = common_get_cur_time_stamps();
							}
						}

						std::string name;
						common_utf8_2gbk(&rtface_info.name, &name);
						LOGD("find face name :%s ,confidence :%d", name.c_str(), rtface_info.confidence);
					}
					else
						;//printf("bad which");
				}
				
				else
				{

					if (identifyinv != 0)
					{
						if (ptr->face_list[i].trackId != -1)
						{
							m_identify_cache[ptr->face_list[i].trackId].inserttime = common_get_cur_time_stamps();
							LOGD("cant find face ID:%d ,will find %d (s) later..", ptr->face_list[i].trackId, identifyinv);
						}
						else
							;// printf("-1 id try faster\n");

					}
						
				}
			}
			else
			{
				;//printf("has index history\n");
				if (!m_identify_cache[ptr->face_list[i].trackId].guid.empty())
				{
					std::string uid = m_identify_cache[ptr->face_list[i].trackId].guid;
					rtface_info.face_reltimes.smile = ptr->face_list[i].smile;
					rtface_info.id = ptr->face_list[i].trackId;
					rtface_info.face_reltimes.age = m_facedb[uid].age;
					rtface_info.name = m_facedb[uid].name;
					rtface_info.face_reltimes.sex = m_facedb[uid].sex;
					rtface_info.userdata = m_facedb[uid].userdata;
					rtface_info.guid = uid;
					rtface_info.confidence = 1000;
					//m_identify_cache[ptr->face_list[i].trackId].inserttime = common_get_cur_time_stamps();
					fmsg.list.push_back(rtface_info);
					now_face_list_id.push_back(rtface_info.id);
					now_face_list.push_back(uid);
				}
			}

 		}
 		bool has_new_infomation = false;

 		for (int i = 0; i < now_face_list.size();i++)
 		{	
			if (m_exist_person_now.find(now_face_list[i]) == m_exist_person_now.end() ||
				m_exist_person_now[now_face_list[i]] != now_face_list_id[i])
 				has_new_infomation = true;
 		}
 
 
		if ((m_exist_update_time.isreach(1000)|| (has_new_infomation) )&& fmsg.list.size() != 0)
 		{
			fmsg.error = 0;
			fmsg.cameralist.push_back(m_cameraid);
			fmsg.content = json_request.identify_face_ntf(fmsg);
			robot_PostMessage::getPostManager()->post(MODULE_NAME, fmsg.content.c_str(), fmsg.content.size());
			m_exist_update_time.update();
 		}
 
 	}
 	m_exist_person_now.clear();

	for (auto it = m_identify_cache.begin(); it != m_identify_cache.end();)
	{
		int times_check = identifyinv;
		if (!it->second.guid.empty())
			times_check = times_check * 6;
		if ((common_get_cur_time_stamps() - it->second.inserttime) > (identifyinv))
			it=m_identify_cache.erase(it);
		else
			it++;
	}
 
 	for (int i = 0; i < now_face_list.size(); i++)
		m_exist_person_now[now_face_list[i]] = now_face_list_id[i];

	if (m_exist_person_now.size()!=0)m_exist_update_time.update();

//	LOGD("face identify  cost time :%d", common_get_cur_time_stampms() - btime);
 	
 	return;
 }

int robot_face_features::normal_score(double s)
{
	//LOGD("score:%.5f,", s);
	int th = 1000;
	if (m_feature_object->m_algname.compare("seetaface") == 0 || 
		m_feature_object->m_algname.compare("yuncong") == 0)
		th = 1000;
	int out = s * th;
	return out;
}

