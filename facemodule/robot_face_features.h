#pragma once
#include "common_helper.h"
#include <set>
#include "robot_json_rsp.h"
#include "robot_face_feature_interface.h"

struct identify_info
{
	unsigned long long inserttime;
	std::string guid;
	identify_info()
	{
		inserttime = 0;
	}
};


class robot_face_features :
	public Thread_helper
{
public:
	robot_face_features();

	bool	init(int id, std::string alg, robot_face_feature_interface *);

	bool	can_idenify();

	bool	can_register(cv::Mat &img, std::vector<FaceRect> &facelist);

	
	virtual ~robot_face_features();
private:

	bool initFaceAlgorithm (std::string &alg);

 	virtual void run();

	void UpdateFaceTemplate();

	bool features(FaceDetectData &ptr, FaceDatabaseFormat &f_info, int index=-1);

	void face_registerimp(FaceTask* ptr);

	void face_identifyimp(FaceDetectData* ptr);

	bool check_face_identify_exist(FaceDatabaseFormat &face_feature, std::map<std::string, FaceDatabaseFormat> &reg_facelist, RealtimeIdentifyFaceInfo &face_result);

	//bool identify(std::map<std::string, FaceDatabaseFormat> &reg_facelist, std::vector<char>&identifyfeature, std::string &face_guid, RealtimeIdentifyFaceInfo &face_result);
	bool identify(std::vector<std::vector<char> >templecache, std::vector<char>&identifyfeature, int &which, int &scoreout);

	int	 normal_score(double s);

	bool face_insert_map(FaceDatabaseFormat &finfo,std::string &guid);

	robot_face_feature_interface *m_feature_object;

	std::map<std::string,int>						m_exist_person_now;
	Timer_helper                                    m_exist_update_time;

	robot_json_rsp json_request;
	int      m_defaultalg_type;
	int      m_face_th;
	int      m_cameraid;
	bool     m_idastate;
	int      identifyinv;

	std::map<std::string, FaceDatabaseFormat> m_facedb;
	std::map<int, std::string> m_id_guid;
	std::vector<std::vector<char> > m_template_cache;


	std::map<int, identify_info>     m_identify_cache;
};

