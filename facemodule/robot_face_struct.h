#pragma  once
#include <string>
#include <vector>
#include "opencv2\core\core.hpp"
#include "common_helper.h"
#include "robot_face_detect_data.h"

#define  HAS_YUNCONG
#define  HAS_ARCSOFT
#define  HAS_READFACE

#define  FACE_IN_PULLUP   150
#define  FACE_IN_PULLDOWN  5000
#define  IMAGE_SAVE_PATH    "face_image\\"


#define  FACE_REGISTER_MESSAGE			100
#define  FACE_REGISTER_HANDLE_TASK		101
#define  FACE_IDENTIFY_HANDLE_TASK		102
#define  FACE_SNAPSHOT_MESSAGE			103
#define  FACE_CANCELSNAPSHOT_MESSAGE	104
#define  FACE_UPDATA_HANDLE_TASK		105
#define  CAMERA_SNAPSHOT_MESSAGE        106

class robot_face_detect_interface;
class robot_face_features;
/*class robot_face_live_interface;*/

enum FACEREGTYPE
{
	MEMORY_FACE_REGISTER = -1,
	FILE_FACE_REGISTER,
	BASE64_FACE_REGISTER
};

struct FaceRect
{
	int			valid;
	int			x;
	int			y;
	int			width;
	int			height;
	double		score;
	double		xoffset;
	double		yoffset;
	double		roll;
	double		yaw;
	double		pitch;
	int         age;
	int			sex; //0 unknow  1 man 2 woman
	int         smile;
	int         trackId;
	bool		isLive;
	bool		has3D;
	bool        takephoto;
	int			beauty;
	robot_face_detect_data data;
	FaceRect()
	{
		x = 0; y = 0;
		has3D = false;
		score = 0;
		width = 0; height = 0;
		age = 0;
		sex = 0;
		smile = 0;
		xoffset = 0;
		yoffset = 0;
		valid = 0;
		trackId = -1;
		isLive = false;
		takephoto = false;
		roll=0;
		yaw = 0;
		pitch = 0;
		beauty = 0;
	}
};


struct RealtimeIdentifyFaceInfo
{
	std::string name;
	std::string data;
	std::string userdata;
	FaceRect   face_reltimes;
	int         confidence;
	int         id;
	std::string guid;
	RealtimeIdentifyFaceInfo()
	{
		confidence = 0;
		id = -1;
	}
};
struct FacesPositionInfo
{
	std::vector<FaceRect> faces;
	int cameraid;
	FacesPositionInfo()
	{
		cameraid = 0;
	}
};


struct FacesIdentifyInfo
{
	std::vector<RealtimeIdentifyFaceInfo> faces;
	int cameraid;
	FacesIdentifyInfo()
	{
		cameraid = 0;
	}
};

struct  FaceMessage
{
	int appid;
	std::string fromguid;
	std::string content;
	std::vector<RealtimeIdentifyFaceInfo>  list;
	bool               face_state;
	int				   timeout;
	int				   savetype;
	int				   reg_type;
	int				   error;
	std::vector<int>   cameralist;
	FaceMessage()
	{
		appid = 0;
		face_state = false;
		timeout = 30;
		savetype = 0;
		reg_type = MEMORY_FACE_REGISTER;
		error = 0;
	}
};


struct FaceDetectData
{
	bool status;
	cv::Mat face_img;
	cv::Mat nis_img;
	std::vector<FaceRect> face_list;
	unsigned int      detect_time;
	int cameraid;

	FaceDetectData()
	{
		cameraid = 0;
		status = false;
		detect_time = 0;
	}
};


struct FaceTask
{
	std::vector<FaceDetectData>  input_image_list;

	FaceDetectData   realtime_image_cache_info;

	FaceMessage   task_message;

};



struct FaceFeatureData
{
	std::string		algtype;
	std::vector<char> face_feature;
	int            cameraid;
	FaceFeatureData()
	{
		cameraid = 0;
	}
};

struct FaceDatabaseFormat
{
	int		    id;
	std::string name;
	std::string userdata;
	int			sex; //0 unknow  1 man 2 woman
	int			age;
	std::vector<FaceFeatureData> face_feature;
	FaceDatabaseFormat()
	{
		id = -1;
		sex = 0;
		age = 0;

	}
};

struct FaceDetecter
{
	std::string  algname;
	robot_face_detect_interface* detect;
	//robot_face_detect_interface* imagedetect;
	FaceDetectData  m_face_in;
	FaceDetectData  m_face_out;
	bool	   m_face_status;
	bool	   m_inntf_status;
	bool	   m_outntf_status;
	int		   down_face_pull;
	int        identifyfps;
	//int      detect_wait_time;
	int			facetrack;

	FaceDetecter()
	{
		detect = NULL;
		//imagedetect = NULL;
		m_face_status = false;
		m_inntf_status = false;
		m_outntf_status = false;
		down_face_pull = FACE_IN_PULLDOWN;
		identifyfps=100;
		facetrack = 0;
	}
};


struct  PersonStateCache
{
	unsigned long long count;
	double sum;
	unsigned long long livecount;
	unsigned long long livesum;

	PersonStateCache()
	{
		sum = 0.0;
		count = 0;
		livecount = 0;
		livesum = 0;
	}

};

struct FaceFeaturer
{
	std::string  algname;
	robot_face_features* thread;
	FaceFeaturer()
	{
		thread = NULL;
	}
};


struct face_process_content
{
	int cameraid;
	int enable_identify;
	int direction;
	std::string camera_readername;
	face_process_content()
	{
		cameraid = 0;
		enable_identify = 0;
		direction = 0;
	}
};
struct face_status_info
{
	bool run_state;
	int  error;
	bool has_person;
	face_status_info()
	{
		run_state = false;
		error = 0;
		has_person = false;
	}
};


enum FaceMessageHandler
{
	FACES_POSITION_INFO = 0,
	FACE_DETECT_NTF,
	FACE_DETECT_STRONG_NTF,
};

struct CameraCacheInfo
{
	std::vector<FaceRect> realtime_position;

	Timer_helper  vaild_times;

	std::vector<RealtimeIdentifyFaceInfo> identify_info;

	std::map<int, int>     mapid;

	int     direction;

	bool	face_status;
	CameraCacheInfo()
	{
		direction = 0;
		face_status = false;
	}
};



#define SEETAFACE_DETECT_MODEL_PATH ".//model//seeta_fd_frontal_v1.0.bin"
#define SEETAFACE_ALIGNMENT_MODEL_PATH ".//model//seeta_fa_v1.1.bin"
#define SEETAFACE_IDENTIFY_MODEL_PATH ".//model//seeta_fr_v1.0.bin"

#define CANMERA_HEIGHT  480
#define CANMERA_WIDTH   640   //opencv default  vga 


////////////////yuncong///////////////////////////////////////
#define LICENCE ""
#define DETECT_YUNCONG_MODLE "./CWModels/_configs_frontend_x86_arm.xml"
//#define DETECT_YUNCONG_MODLE "./CWModels/_configs_dl_traditional.xml"
#define FACECONFIG_YUNCONG_MODLE "./CWModels/CWR_Config_1_1.xml"
#define FACECONFIG_YUNCONG_MODLE_N "./CWModels/CWR_Config_1_n.xml"
#define FACEATTRICONFIG_YUNCONG_MODLE "./CWModels/CWR_Config_attri.xml"
#define NISLIVE_YUNCONG_MODLE "./CWModels/nirLivDet_model.bin"
#define MATRIX_YUNCONG_MODLE "./CWModels/matrix_para.xml"
#define MAX_NUM_FACES 20    //最多检测的人脸数

#ifdef HAS_ARCSOFT
#define ARCSOFT_APPID "3kTZMA9S9eNMPJB2pq4Ef64GqxjwomCGXKqZG8mLHJoV" 
//#define  ARCSOFT_APPID "3kTZMA9S9eNMPJB2pq4Ef64Q1N1AyxPUNYXJMpsnEqRz"
#define ARCSOFT_SDKKey "6Nc5XZk2m37RmVTLjN65Bgkf17MChrLfEH33dcZrVUBh"  
//#define ARCSOFT_SDKKey "9TTzZQH1FFzJVrKLioXYvFWAj1sZFemdgg3CkdZXcio1"  
#endif // HAS_ARCSOFT


#ifdef HAS_READFACE
#define READFACE_CFGPATH "Readfacecfg"
#define READFACE_DB_FILE_PATH   READFACE_CFGPATH##"/readsense.db"
#define READFACE_CFG_PIC_PATH   READFACE_CFGPATH##"/piccfg.ini"

#define READFACE_CFG_INIT_LICENSE_PATH   READFACE_CFGPATH##"/initLicnese.lic"
#define READFACE_CFG_INIT_AUTH_CONTENT_PATH   READFACE_CFGPATH##"/initAuthContent.lic"

#endif // HAS_READFACE



extern const char *MODULE_NAME;


enum FACE_ERROR
{
	FACE_REGISTER_FAILED = 1010,
	FACE_NOALL_REGISTER_FAILED ,
	VIS_DEVICE_CONFIG_ERROR=1101,
	NIS_DEVICE_CONFGI_ERROR=1102,
	LIVE_ALL_DEVICE_OPEN_ERROR=1103,
	LIVE_VIS_DEVICE_OPEN_ERROR=1104,
	LIVE_NIS_DEVICE_OPEN_ERROR=1105,
	DEVICE_OPEN_ERROR=1106,
	FACE_MEMORY_OPEN_ERROR=1107,
	LIVE_CREATE_ERROR=1108,
	SERVICE_CLOSEING=1109,

	SERVICE_DISABLE_IDENTIFY=1110,
	INPUT_PARA_IVAILD=1111,
	CACHE_NOFACE=1112,
	WAIT_OTHER_MODULE=1113,

};