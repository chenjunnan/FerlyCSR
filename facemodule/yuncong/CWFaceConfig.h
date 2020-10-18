#ifndef CWFaceDefine_H
#define CWFaceDefine_H


#ifdef  CWFACESDK_EXPORTS
#define CW_FACE_API  __declspec(dllexport)
#else
#define CW_FACE_API
#endif


	/***************
	* 检测功能开关选项
	*/
#define CW_OP_DET                 0		  // (1<<0)进行人脸检测，并返回人脸矩形位置，默认开启
#define CW_OP_TRACK               2		  // (1<<1)进行人脸跟踪，并返回人脸跟踪的ID
#define CW_OP_KEYPT               4		  // (1<<2)进行人脸关键点检测，并返回人脸上的关键点坐标信息
#define CW_OP_ALIGN               8		  // (1<<3)进行人脸图像对齐，并返回对齐后的人脸图像，用来提取特征
#define CW_OP_QUALITY             16      // (1<<4)人脸质量评估开关（质量分子项开关在配置文件中配置）
#define CW_OP_ALL                 30      // （所有开关综合）总开关


	/***************
	* 通用错误码
	*/
typedef enum cw_errcode 
{ 
	CW_SDKLIT_OK = 0,					// 成功 or 合法

	CW_UNKNOWN_ERR = 20000,		        // 未知错误
	CW_DETECT_INIT_ERR,					// 初始化人脸检测器失败:如加载模型失败等
	CW_KEYPT_INIT_ERR,					// 初始化关键点检测器失败：如加载模型失败等
	CW_QUALITY_INIT_ERR,			    // 初始化跟踪器失败：如加载模型失败等

	CW_DET_ERR,						    // 检测失败
	CW_TRACK_ERR,						// 跟踪失败
	CW_KEYPT_ERR,						// 提取关键点失败
	CW_ALIGN_ERR,						// 对齐人脸失败
	CW_QUALITY_ERR,					    // 质量评估失败

	CW_EMPTY_FRAME_ERR,		            // 空图像
	CW_UNSUPPORT_FORMAT_ERR,			// 图像格式不支持
	CW_ROI_ERR,						    // ROI设置失败
	CW_UNINITIALIZED_ERR,				// 尚未初始化
	CW_MINMAX_ERR,						// 最小最大人脸设置失败
	CW_OUTOF_RANGE_ERR,                 // 数据范围错误
	CW_UNAUTHORIZED_ERR,				// 未授权
	CW_METHOD_UNAVAILABLE,			    // 方法无效
	CW_PARAM_INVALID,                   // 参数无效，如模型路径错误
	CW_BUFFER_EMPTY,					// 缓冲区空

	CW_FILE_UNAVAILABLE,                // 文件不存在：如加载的模型不存在等.
	CW_DEVICE_UNAVAILABLE,    			// 设备不存在
	CW_DEVICE_ID_UNAVAILABLE, 		    // 设备id不存在
	CW_EXCEEDMAXHANDLE_ERR,		        // 超过授权最大句柄数

	CW_RECOG_FEATURE_MODEL_ERR,		    // 加载特征识别模型失败   
	CW_RECOG_ALIGNEDFACE_ERR,		    // 对齐图片数据错误
	CW_RECOG_MALLOCMEMORY_ERR,          // 预分配特征空间不足  
	CW_RECOG_FILEDDATA_ERR,		        // 用于注册的特征数据错误
	CW_RECOG_PROBEDATA_ERR,		        // 用于检索的特征数据错误
	CW_RECOG_EXCEEDMAXFEASPEED,		    // 超过授权最大提特征速度
	CW_RECOG_EXCEEDMAXCOMSPEED,		    // 超过授权最大比对速度
	CW_ATTRI_AGEGENDER_MODEL_ERR,       // 加载年龄性别模型失败 
	CW_ATTRI_EVAL_AGEGENDER_ERR,        // 年龄性别识别失败

	CW_SERVICE_REGISTID_EXIST,          // 注册人脸id已存在
	CW_SERVICE_UNREGISTID_NOEXIST,      // 反注册人脸的id不存在
	CW_SERVICE_RECOG_GROUP_EMPTY,       // 检索的底库为空

	CW_LICENCE_ACCOUNT_TIMEOUT,         // 安卓网络授权账号过期
	CW_LICENCE_HTTP_ERROR,              // 网络授权http错误
	CW_LICENCE_MALLOCMEMORY_ERR,        // 网络授权内存分配不足

	CW_INSTALL_KEY_ENCRYPT_ERR,         // 加密失败
	CW_INSTALL_KEY_EXIST_ERR,           // 网络授权错误：已存在正式授权
	CW_INSTALL_KEY_GETC2V_ERR,          // 网络授权错误：获取C2V设备文件错误（是否安装驱动）
	CW_INSTALL_KEY_GETV2C_ERR,          // 网络授权错误：获取V2C授权文件错误
	CW_INSTALL_KEY_INSTALL_ERR,         // 网络授权错误：安装V2C授权文件错误（在当前目录找到V2C授权文件，使用工具进行安装）
} cw_errcode_t; 


	/***************
	* 质量分检测错误码
	*/
typedef enum cw_quality_errcode
{
	CW_QUALITY_OK      = 0,				// 质量分数据有效
	CW_QUALITY_NO_DATA = 20150,		    // 质量分数据无效，原因：尚未检测
	CW_QUALITY_ERROR_UNKNOWN,           // 未知错误
} cw_quality_errcode_t;


	/***************
	* 接口功能参数
	*/
typedef struct cw_det_param
{
	int roiX;								// roi, 默认整帧图像0, 0, 0, 0 若设置为异常值检测阶段将恢复默认
	int roiY;
	int roiWidth;
	int roiHeight;

	int minSize;							// 检测人脸尺寸范围： pc端默认[48,600];移动端默认[100,400]
	int maxSize;

	int globleDetFreq;						// 全局检测频率： pc端默认12，移动端默认21，最好是3的倍数
	int frameNumForLost;				    // 人脸离开判定阈值：一个人脸跟丢到退出跟踪的帧数，默认75
	const char* pConfigFile;                // 内部参数配置文件路径，此参数只能设置(set);从句柄内部获取出来的一律无效
} cw_det_param_t;


	/***************
	* 图像旋转角度（逆时针）
	*/
typedef enum cw_img_angle
{ 
	CW_IMAGE_ANGLE_0 = 0,       
	CW_IMAGE_ANGLE_90,       
	CW_IMAGE_ANGLE_180,       
	CW_IMAGE_ANGLE_270       
} cw_img_angle_t; 


	/***************
	* 图像镜像
	*/
typedef enum cw_img_mirror
{ 
	CW_IMAGE_MIRROR_NONE = 0,        // 不镜像  
	CW_IMAGE_MIRROR_HOR,             // 水平镜像
	CW_IMAGE_MIRROR_VER,             // 垂直镜像
	CW_IMAGE_MIRROR_HV               // 垂直和水平镜像
} cw_img_mirror_t; 


	/***************
	* 图像格式
	*/
typedef enum cw_img_form 
{ 
	CW_IMAGE_GRAY8 = 0,
	CW_IMAGE_BGR888,
	CW_IMAGE_BGRA8888,
	CW_IMAGE_RGB888,
	CW_IMAGE_RGBA8888,
	CW_IMAGE_YUV420P, 
	CW_IMAGE_YV12,
	CW_IMAGE_NV12, 
	CW_IMAGE_NV21,
	CW_IMAGE_BINARY,
} cw_img_form_t; 


	/***************
	* 图像
	*/
typedef struct cw_img
{
	long long			frameId;           // 帧号
	char*				data;			   // 图像数据（必须预分配足够的空间）
	int					dataLen;		   // 数据长度，CW_IMAGE_BINARY格式必须设置，其他格式可不设
	int					width;			   // 宽, CW_IMAGE_BINARY格式可不设，其他格式必须设置
	int					height;			   // 高, CW_IMAGE_BINARY格式可不设，其他格式必须设置
	cw_img_form_t		format;			   // 图像格式
	cw_img_angle_t	    angle;			   // 旋转角度
	cw_img_mirror_t	    mirror;            // 镜像
} cw_img_t;


	/***************
	* 人脸区域
	*/
typedef struct cw_facepos_rect
{
	int    x;
	int    y;
	int    width;
	int    height;
} cw_facepos_rect_t;


	/***************
	* 对齐人脸图像结构
	*/
#define CW_ALIGNED_SIZE 128
typedef struct cw_aligned_face
{
	char   data[CW_ALIGNED_SIZE * CW_ALIGNED_SIZE];		       // 对齐人脸图像数据
	int    width;											   // 宽
	int    height;											   // 高
	int    nChannels;										   // 图像通道
} cw_aligned_face_t;

	/***************
	* 关键点信息
	*/
#define CW_MAX_KEYPT_NUM 68
typedef struct cw_point
{
	float x;				
	float y;
} cw_point_t;

typedef struct cw_keypt
{
	cw_point_t	    points[CW_MAX_KEYPT_NUM];					    // 关键点
	int				nkeypt;											// 关键点个数
	float			keyptScore;										// 关键点得分,推荐阈值为0.7
} cw_keypt_t;


	/***************
	* 人脸质量分
	*/
#define FACE_QUALITY_MAX_COUNT 32
typedef struct cw_quality
{
	cw_quality_errcode_t errcode;		                       // 质量分析错误码
	float                scores[FACE_QUALITY_MAX_COUNT];       /* 质量分分数项，具体含义（根据数据下标顺序）:
															   * 0 - 人脸质量总分，0.65-1.0
															   * 1 - 清晰度，越大表示越清晰，推荐范围0.65-1.0（在启用第16项mog分数的总分时，此分数为常数1.0，请忽略）
															   * 2 - 亮度，越大表示越亮，推荐范围0.2-0.8
															   * 3 - 人脸角度，左转为正，右转为负
															   * 4 - 人脸角度，抬头为正，低头为负
															   * 5 - 人脸角度，顺时针为正，逆时针为负
															   * 6 - 左右转程度，越大表示角度越正，推荐范围0.5-1.0
															   * 7 - 抬低头程度，越大表示角度越正,推荐范围0.5-1.0
															   * 8 - 肤色接近真人肤色程度，越大表示越真实，推荐范围0.5-1.0
															   * 9 - 张嘴分数， 越大表示越可能张嘴，推荐范围0.0-0.5
															   * 10 - 左眼睁眼分数， 越大表示左眼越可能是睁眼，推荐范围0.5-1.0
															   * 11 - 右眼睁眼分数， 越大表示右眼越可能是睁眼，推荐范围0.5-1.0
															   * 12 - 戴黑框眼镜置信度，越大表示戴黑框眼镜的可能性越大，推荐范围0.0-0.5
															   * 13 - 戴墨镜的置信分，越大表示戴墨镜的可能性越大，推荐范围0.0-0.5
															   * 14 - 左眼眼睛被遮挡的置信度，越大表示眼睛越可能被遮挡，目前只在睁眼分小于0.5时有意义，推荐范围0.0-0.5
															   * 15 - 右眼眼睛被遮挡的置信度，越大表示眼睛越可能被遮挡，目前只在睁眼分小于0.5时有意义，推荐范围0.0-0.5
															   * 16 - mog清晰度，返回0.0~1.0的分数，越大越清晰，阈值建议0.100001
															   * 17~31 - 备用
															   */
} cw_quality_t;


	/***************
	* 人脸综合信息
	*/
typedef struct cw_face_res
{
	long long			 frameId;           // 人脸所在帧号

	int					 detected;			// 0: 跟踪到的人脸; 1: 检测到的人脸; 2:检测到但不会被进行后续计算(关键点)的人脸; 
	                                        // 3: 可能是静态误检框；4:大角度人脸; 5:关键点错误; 6:不需再处理的人脸（只有标记为1的人脸，关键点、
	                                        // 对齐、质量分才有效；但除0之外其他都可能有口罩分）7:被估计为低质量人脸

	int					 trackId;			// 人脸ID（ID<0表示没有进入跟踪）

	cw_facepos_rect_t	 faceRect;			// 人脸框

	cw_keypt_t		     keypt;             // 关键点

	cw_aligned_face_t	 faceAligned;		// 对齐人脸

	cw_quality_t	     quality;			// 人脸质量

} cw_face_res_t;


	/***************
	* 人脸识别句柄模式
	*/
typedef enum cw_recog_pattern
{
	CW_FEATURE_EXTRACT = 0,                //  特征提取 
	CW_RECOGNITION     = 1                 //  识    别 
} cw_recog_pattern_t;


//////////////////////////////////////////////////////////////////////////红外活体

   /***************
	* 红外活体检测摄像头数目类型
	*/
typedef enum cw_nirliv_camera_type 
{
	CW_NIR_LIV_DET_MONOCULAR = 0,		// 红外单目摄像头活体检测，只使用红外摄像头检测活体
	CW_NIR_LIV_DET_BINOCULAR		    // 红外双目摄像头活体检测，使用双目摄像头检测活体
}cw_nirliv_camera_type_t;

   /***************
	* 红外活体检测模式：距离最近人脸检测/多人检测
	*/
typedef enum cw_nirliv_det_type
{
	CW_NIR_LIV_MODE_NEARST_FACE = 0,   // 对图中的最近进行人脸活体检测
	CW_NIR_LIV_MODE_MULTI_FACES        // 对图中的所有符合要求的人脸进行活体检测
} cw_nirliv_det_type_t;

   /***************
	* 红外活体检测结果返回值
	*/
typedef enum cw_nirliv_det_rst
{
	CW_NIR_LIV_DET_LIVE = 0,				// 以阈值0.5判断为活体
	CW_NIR_LIV_DET_UNLIVE,				    // 以阈值0.5判断为非活体
	CW_NIR_LIV_DET_DIST_FAILED,				// 人脸距离检测未通过
	CW_NIR_LIV_DET_SKIN_FAILED,				// 人脸肤色检测未通过
	CW_NIR_LIV_DET_NO_PAIR_FACE,			// 未匹配到人脸
	CW_NIR_LIV_DET_IS_INIT					// 红外活体检测结果初始值
}cw_nirliv_det_rst_t;

   /***************
	* 红外活体检测错误码
	*/
typedef enum cw_nirliveness_err
{
	CW_NIRLIV_OK = 0,						// 成功返回
	CW_NIRLIV_ERR_CREATE_HANDLE = 26000,	// 创建红外活体检测句柄失败
	CW_NIRLIV_ERR_FREE_HANDLE,				// 释放红外活体检测句柄失败
	CW_NIRLIV_ERR_FACE_PAIR,	            // 人脸匹配初始化失败
	CW_NIRLIV_ERR_CREAT_LOG_DIR,	        // 创建日志路径失败
	CW_NIRLIV_ERR_MODEL_NOTEXIST,			// 输入模型不存在
	CW_NIRLIV_ERR_MODEL_FAILED,			    // 输入模型初始化失败
	CW_NIRLIV_ERR_INPUT_UNINIT,			    // 输入未初始化
	CW_NIRLIV_ERR_NIR_NO_FACE,				// 输入红外图片没有人脸
	CW_NIRLIV_ERR_VIS_NO_FACE,				// 输入可见光图片没有人脸
	CW_NIRLIV_ERR_NO_PAIR_FACE,			    // 输入可见光和红外图片人脸未能匹配
	CW_NIRLIV_ERR_PUSH_DATA,		        // 输入数据失败
	CW_NIRLIV_ERR_NUM_LANDMARKS,			// 输入可见光图片和红外图片关键点个数不等
	CW_NIRLIV_ERR_NO_LANDMARKS,			    // 输入红外图片没有人脸关键点
	CW_NIRLIV_ERR_INPUT_IMAGE,		        // 输入红外图片或者可见光图片不是多通道
	CW_NIRLIV_ERR_UNAUTHORIZED,			    // 没有license（未授权）
	CW_NIRLIV_ERR_FACE_NUM_ERR,			    // 未开启人脸匹配开关时，可见光或红外图像人脸大于1
	CW_NIRLIV_ERR_CAM_UNCW,		            // 非云从定制摄像头
	CW_NIRLIV_ERR_UNKNOWN,					// 未知结果
	CW_NIRLIV_ERR_MAXHANDLE,			    // 超过最大红外活体最大授权句柄数
	CW_NIRLIV_ERR_NIRIMAGE,			        // 输入红外图片数据错误
	CW_NIRLIV_ERR_VISIMAGE,			        // 输入可见光图片数据错误
} cw_nirliveness_err_t;

	/***************
	* 红外活体图像数据
	*/
typedef struct cw_nirliv_img
{
	char*				data;			   // 图像数据（必须预分配足够的空间）
	int					dataLen;		   // 数据长度，只有CW_IMAGE_BINARY格式需要设置
	int					width;			   // 宽, CW_IMAGE_BINARY格式可不设置，其他格式必须设置
	int					height;			   // 高, CW_IMAGE_BINARY格式可不设置，其他格式必须设置
	cw_img_form_t		format;			   // 图像格式，详情见cw_img_form_t
} cw_nirliv_img_t;


   /***************
	* 红外活体人脸相关信息
	*/
typedef struct cw_nirliv_face_param
{
	float                      fSkinScore;		// 输入肤色置信分
	float                      fKeyScore;		// 输入关键点置信分
	cw_point_t                 *pKeypoints;		// 输入关键点信息
} cw_nirliv_face_param_t;


	/***************
	* 红外活体检测输入的人脸图片数据
	*/
typedef struct cw_nirliv_detinfo
{
	int   nLandmarks;							// 关键点个数

	cw_nirliv_img_t *pNirImg;                   // NIR红外图片信息
	cw_nirliv_img_t *pVisImg;                   // VIS可见光图片信息，如果为单目摄像头方式，该值填NULL即可

	int nirFaceNum;								// NIR红外人脸个数
	int visFaceNum;								// VIS可见光人脸个数
	cw_nirliv_face_param_t *pNirInfo;			// NIR红外人脸信息数组（nirFaceNum*sizeof(cw_nirliv_face_param_t)）
	cw_nirliv_face_param_t *pVisInfo;			// VIS可见光人脸信息数组，如果为单目摄像头方式，该值填NULL即可
} cw_nirliv_detinfo_t;


   /***************
	* 红外活体检测结果
	*/
typedef struct cw_nirliv_res
{
	cw_nirliv_det_rst_t   livRst;			// 输出红外活体检测结果返回值
	float                 score;			// 输出红外活体检测得分，非活体的时候为0
} cw_nirliv_res_t;


	/***************
	* 安装授权标志
	*/
typedef enum cw_install_key_flag
{
	CW_RETERROR_IFEXIST = 0,                //  如果存在正式授权，则不继续安装，返回错误码（建议使用此标志，防止多次授权）
	CW_INSTALLSTILL_IFEXIST = 1             //  不管是否存在正式授权，仍然安装新授权
} cw_install_key_flag_t;


	/***************
	* 安装授权时保存文件标志
	*/
typedef enum cw_save_key_flag
{
	CW_SAVE_ALL = 0,                //  授权文件和设备文件均保存，文件保存在当前程序运行目录
	CW_SAVE_ONLY_V2C = 1,           //  仅保存V2C授权文件
	CW_SAVE_ONLY_C2V = 2,           //  仅保存C2V设备文件
	CW_SAVE_NONE = 3                //  都不保存，如果机器无文件保存权限，建议使用该参数，防止出错
} cw_save_key_flag_t;


#endif

