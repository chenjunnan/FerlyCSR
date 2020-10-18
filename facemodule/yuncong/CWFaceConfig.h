#ifndef CWFaceDefine_H
#define CWFaceDefine_H


#ifdef  CWFACESDK_EXPORTS
#define CW_FACE_API  __declspec(dllexport)
#else
#define CW_FACE_API
#endif


	/***************
	* ��⹦�ܿ���ѡ��
	*/
#define CW_OP_DET                 0		  // (1<<0)����������⣬��������������λ�ã�Ĭ�Ͽ���
#define CW_OP_TRACK               2		  // (1<<1)�����������٣��������������ٵ�ID
#define CW_OP_KEYPT               4		  // (1<<2)���������ؼ����⣬�����������ϵĹؼ���������Ϣ
#define CW_OP_ALIGN               8		  // (1<<3)��������ͼ����룬�����ض���������ͼ��������ȡ����
#define CW_OP_QUALITY             16      // (1<<4)���������������أ�������������������ļ������ã�
#define CW_OP_ALL                 30      // �����п����ۺϣ��ܿ���


	/***************
	* ͨ�ô�����
	*/
typedef enum cw_errcode 
{ 
	CW_SDKLIT_OK = 0,					// �ɹ� or �Ϸ�

	CW_UNKNOWN_ERR = 20000,		        // δ֪����
	CW_DETECT_INIT_ERR,					// ��ʼ�����������ʧ��:�����ģ��ʧ�ܵ�
	CW_KEYPT_INIT_ERR,					// ��ʼ���ؼ�������ʧ�ܣ������ģ��ʧ�ܵ�
	CW_QUALITY_INIT_ERR,			    // ��ʼ��������ʧ�ܣ������ģ��ʧ�ܵ�

	CW_DET_ERR,						    // ���ʧ��
	CW_TRACK_ERR,						// ����ʧ��
	CW_KEYPT_ERR,						// ��ȡ�ؼ���ʧ��
	CW_ALIGN_ERR,						// ��������ʧ��
	CW_QUALITY_ERR,					    // ��������ʧ��

	CW_EMPTY_FRAME_ERR,		            // ��ͼ��
	CW_UNSUPPORT_FORMAT_ERR,			// ͼ���ʽ��֧��
	CW_ROI_ERR,						    // ROI����ʧ��
	CW_UNINITIALIZED_ERR,				// ��δ��ʼ��
	CW_MINMAX_ERR,						// ��С�����������ʧ��
	CW_OUTOF_RANGE_ERR,                 // ���ݷ�Χ����
	CW_UNAUTHORIZED_ERR,				// δ��Ȩ
	CW_METHOD_UNAVAILABLE,			    // ������Ч
	CW_PARAM_INVALID,                   // ������Ч����ģ��·������
	CW_BUFFER_EMPTY,					// ��������

	CW_FILE_UNAVAILABLE,                // �ļ������ڣ�����ص�ģ�Ͳ����ڵ�.
	CW_DEVICE_UNAVAILABLE,    			// �豸������
	CW_DEVICE_ID_UNAVAILABLE, 		    // �豸id������
	CW_EXCEEDMAXHANDLE_ERR,		        // ������Ȩ�������

	CW_RECOG_FEATURE_MODEL_ERR,		    // ��������ʶ��ģ��ʧ��   
	CW_RECOG_ALIGNEDFACE_ERR,		    // ����ͼƬ���ݴ���
	CW_RECOG_MALLOCMEMORY_ERR,          // Ԥ���������ռ䲻��  
	CW_RECOG_FILEDDATA_ERR,		        // ����ע����������ݴ���
	CW_RECOG_PROBEDATA_ERR,		        // ���ڼ������������ݴ���
	CW_RECOG_EXCEEDMAXFEASPEED,		    // ������Ȩ����������ٶ�
	CW_RECOG_EXCEEDMAXCOMSPEED,		    // ������Ȩ���ȶ��ٶ�
	CW_ATTRI_AGEGENDER_MODEL_ERR,       // ���������Ա�ģ��ʧ�� 
	CW_ATTRI_EVAL_AGEGENDER_ERR,        // �����Ա�ʶ��ʧ��

	CW_SERVICE_REGISTID_EXIST,          // ע������id�Ѵ���
	CW_SERVICE_UNREGISTID_NOEXIST,      // ��ע��������id������
	CW_SERVICE_RECOG_GROUP_EMPTY,       // �����ĵ׿�Ϊ��

	CW_LICENCE_ACCOUNT_TIMEOUT,         // ��׿������Ȩ�˺Ź���
	CW_LICENCE_HTTP_ERROR,              // ������Ȩhttp����
	CW_LICENCE_MALLOCMEMORY_ERR,        // ������Ȩ�ڴ���䲻��

	CW_INSTALL_KEY_ENCRYPT_ERR,         // ����ʧ��
	CW_INSTALL_KEY_EXIST_ERR,           // ������Ȩ�����Ѵ�����ʽ��Ȩ
	CW_INSTALL_KEY_GETC2V_ERR,          // ������Ȩ���󣺻�ȡC2V�豸�ļ������Ƿ�װ������
	CW_INSTALL_KEY_GETV2C_ERR,          // ������Ȩ���󣺻�ȡV2C��Ȩ�ļ�����
	CW_INSTALL_KEY_INSTALL_ERR,         // ������Ȩ���󣺰�װV2C��Ȩ�ļ������ڵ�ǰĿ¼�ҵ�V2C��Ȩ�ļ���ʹ�ù��߽��а�װ��
} cw_errcode_t; 


	/***************
	* �����ּ�������
	*/
typedef enum cw_quality_errcode
{
	CW_QUALITY_OK      = 0,				// ������������Ч
	CW_QUALITY_NO_DATA = 20150,		    // ������������Ч��ԭ����δ���
	CW_QUALITY_ERROR_UNKNOWN,           // δ֪����
} cw_quality_errcode_t;


	/***************
	* �ӿڹ��ܲ���
	*/
typedef struct cw_det_param
{
	int roiX;								// roi, Ĭ����֡ͼ��0, 0, 0, 0 ������Ϊ�쳣ֵ���׶ν��ָ�Ĭ��
	int roiY;
	int roiWidth;
	int roiHeight;

	int minSize;							// ��������ߴ緶Χ�� pc��Ĭ��[48,600];�ƶ���Ĭ��[100,400]
	int maxSize;

	int globleDetFreq;						// ȫ�ּ��Ƶ�ʣ� pc��Ĭ��12���ƶ���Ĭ��21�������3�ı���
	int frameNumForLost;				    // �����뿪�ж���ֵ��һ�������������˳����ٵ�֡����Ĭ��75
	const char* pConfigFile;                // �ڲ����������ļ�·�����˲���ֻ������(set);�Ӿ���ڲ���ȡ������һ����Ч
} cw_det_param_t;


	/***************
	* ͼ����ת�Ƕȣ���ʱ�룩
	*/
typedef enum cw_img_angle
{ 
	CW_IMAGE_ANGLE_0 = 0,       
	CW_IMAGE_ANGLE_90,       
	CW_IMAGE_ANGLE_180,       
	CW_IMAGE_ANGLE_270       
} cw_img_angle_t; 


	/***************
	* ͼ����
	*/
typedef enum cw_img_mirror
{ 
	CW_IMAGE_MIRROR_NONE = 0,        // ������  
	CW_IMAGE_MIRROR_HOR,             // ˮƽ����
	CW_IMAGE_MIRROR_VER,             // ��ֱ����
	CW_IMAGE_MIRROR_HV               // ��ֱ��ˮƽ����
} cw_img_mirror_t; 


	/***************
	* ͼ���ʽ
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
	* ͼ��
	*/
typedef struct cw_img
{
	long long			frameId;           // ֡��
	char*				data;			   // ͼ�����ݣ�����Ԥ�����㹻�Ŀռ䣩
	int					dataLen;		   // ���ݳ��ȣ�CW_IMAGE_BINARY��ʽ�������ã�������ʽ�ɲ���
	int					width;			   // ��, CW_IMAGE_BINARY��ʽ�ɲ��裬������ʽ��������
	int					height;			   // ��, CW_IMAGE_BINARY��ʽ�ɲ��裬������ʽ��������
	cw_img_form_t		format;			   // ͼ���ʽ
	cw_img_angle_t	    angle;			   // ��ת�Ƕ�
	cw_img_mirror_t	    mirror;            // ����
} cw_img_t;


	/***************
	* ��������
	*/
typedef struct cw_facepos_rect
{
	int    x;
	int    y;
	int    width;
	int    height;
} cw_facepos_rect_t;


	/***************
	* ��������ͼ��ṹ
	*/
#define CW_ALIGNED_SIZE 128
typedef struct cw_aligned_face
{
	char   data[CW_ALIGNED_SIZE * CW_ALIGNED_SIZE];		       // ��������ͼ������
	int    width;											   // ��
	int    height;											   // ��
	int    nChannels;										   // ͼ��ͨ��
} cw_aligned_face_t;

	/***************
	* �ؼ�����Ϣ
	*/
#define CW_MAX_KEYPT_NUM 68
typedef struct cw_point
{
	float x;				
	float y;
} cw_point_t;

typedef struct cw_keypt
{
	cw_point_t	    points[CW_MAX_KEYPT_NUM];					    // �ؼ���
	int				nkeypt;											// �ؼ������
	float			keyptScore;										// �ؼ���÷�,�Ƽ���ֵΪ0.7
} cw_keypt_t;


	/***************
	* ����������
	*/
#define FACE_QUALITY_MAX_COUNT 32
typedef struct cw_quality
{
	cw_quality_errcode_t errcode;		                       // ��������������
	float                scores[FACE_QUALITY_MAX_COUNT];       /* �����ַ�������庬�壨���������±�˳��:
															   * 0 - ���������ܷ֣�0.65-1.0
															   * 1 - �����ȣ�Խ���ʾԽ�������Ƽ���Χ0.65-1.0�������õ�16��mog�������ܷ�ʱ���˷���Ϊ����1.0������ԣ�
															   * 2 - ���ȣ�Խ���ʾԽ�����Ƽ���Χ0.2-0.8
															   * 3 - �����Ƕȣ���תΪ������תΪ��
															   * 4 - �����Ƕȣ�̧ͷΪ������ͷΪ��
															   * 5 - �����Ƕȣ�˳ʱ��Ϊ������ʱ��Ϊ��
															   * 6 - ����ת�̶ȣ�Խ���ʾ�Ƕ�Խ�����Ƽ���Χ0.5-1.0
															   * 7 - ̧��ͷ�̶ȣ�Խ���ʾ�Ƕ�Խ��,�Ƽ���Χ0.5-1.0
															   * 8 - ��ɫ�ӽ����˷�ɫ�̶ȣ�Խ���ʾԽ��ʵ���Ƽ���Χ0.5-1.0
															   * 9 - ��������� Խ���ʾԽ�������죬�Ƽ���Χ0.0-0.5
															   * 10 - �������۷����� Խ���ʾ����Խ���������ۣ��Ƽ���Χ0.5-1.0
															   * 11 - �������۷����� Խ���ʾ����Խ���������ۣ��Ƽ���Χ0.5-1.0
															   * 12 - ���ڿ��۾����Ŷȣ�Խ���ʾ���ڿ��۾��Ŀ�����Խ���Ƽ���Χ0.0-0.5
															   * 13 - ��ī�������ŷ֣�Խ���ʾ��ī���Ŀ�����Խ���Ƽ���Χ0.0-0.5
															   * 14 - �����۾����ڵ������Ŷȣ�Խ���ʾ�۾�Խ���ܱ��ڵ���Ŀǰֻ�����۷�С��0.5ʱ�����壬�Ƽ���Χ0.0-0.5
															   * 15 - �����۾����ڵ������Ŷȣ�Խ���ʾ�۾�Խ���ܱ��ڵ���Ŀǰֻ�����۷�С��0.5ʱ�����壬�Ƽ���Χ0.0-0.5
															   * 16 - mog�����ȣ�����0.0~1.0�ķ�����Խ��Խ��������ֵ����0.100001
															   * 17~31 - ����
															   */
} cw_quality_t;


	/***************
	* �����ۺ���Ϣ
	*/
typedef struct cw_face_res
{
	long long			 frameId;           // ��������֡��

	int					 detected;			// 0: ���ٵ�������; 1: ��⵽������; 2:��⵽�����ᱻ���к�������(�ؼ���)������; 
	                                        // 3: �����Ǿ�̬����4:��Ƕ�����; 5:�ؼ������; 6:�����ٴ����������ֻ�б��Ϊ1���������ؼ��㡢
	                                        // ���롢�����ֲ���Ч������0֮�������������п��ַ֣�7:������Ϊ����������

	int					 trackId;			// ����ID��ID<0��ʾû�н�����٣�

	cw_facepos_rect_t	 faceRect;			// ������

	cw_keypt_t		     keypt;             // �ؼ���

	cw_aligned_face_t	 faceAligned;		// ��������

	cw_quality_t	     quality;			// ��������

} cw_face_res_t;


	/***************
	* ����ʶ����ģʽ
	*/
typedef enum cw_recog_pattern
{
	CW_FEATURE_EXTRACT = 0,                //  ������ȡ 
	CW_RECOGNITION     = 1                 //  ʶ    �� 
} cw_recog_pattern_t;


//////////////////////////////////////////////////////////////////////////�������

   /***************
	* �������������ͷ��Ŀ����
	*/
typedef enum cw_nirliv_camera_type 
{
	CW_NIR_LIV_DET_MONOCULAR = 0,		// ���ⵥĿ����ͷ�����⣬ֻʹ�ú�������ͷ������
	CW_NIR_LIV_DET_BINOCULAR		    // ����˫Ŀ����ͷ�����⣬ʹ��˫Ŀ����ͷ������
}cw_nirliv_camera_type_t;

   /***************
	* ���������ģʽ����������������/���˼��
	*/
typedef enum cw_nirliv_det_type
{
	CW_NIR_LIV_MODE_NEARST_FACE = 0,   // ��ͼ�е������������������
	CW_NIR_LIV_MODE_MULTI_FACES        // ��ͼ�е����з���Ҫ����������л�����
} cw_nirliv_det_type_t;

   /***************
	* ���������������ֵ
	*/
typedef enum cw_nirliv_det_rst
{
	CW_NIR_LIV_DET_LIVE = 0,				// ����ֵ0.5�ж�Ϊ����
	CW_NIR_LIV_DET_UNLIVE,				    // ����ֵ0.5�ж�Ϊ�ǻ���
	CW_NIR_LIV_DET_DIST_FAILED,				// ����������δͨ��
	CW_NIR_LIV_DET_SKIN_FAILED,				// ������ɫ���δͨ��
	CW_NIR_LIV_DET_NO_PAIR_FACE,			// δƥ�䵽����
	CW_NIR_LIV_DET_IS_INIT					// �������������ʼֵ
}cw_nirliv_det_rst_t;

   /***************
	* ��������������
	*/
typedef enum cw_nirliveness_err
{
	CW_NIRLIV_OK = 0,						// �ɹ�����
	CW_NIRLIV_ERR_CREATE_HANDLE = 26000,	// ���������������ʧ��
	CW_NIRLIV_ERR_FREE_HANDLE,				// �ͷź����������ʧ��
	CW_NIRLIV_ERR_FACE_PAIR,	            // ����ƥ���ʼ��ʧ��
	CW_NIRLIV_ERR_CREAT_LOG_DIR,	        // ������־·��ʧ��
	CW_NIRLIV_ERR_MODEL_NOTEXIST,			// ����ģ�Ͳ�����
	CW_NIRLIV_ERR_MODEL_FAILED,			    // ����ģ�ͳ�ʼ��ʧ��
	CW_NIRLIV_ERR_INPUT_UNINIT,			    // ����δ��ʼ��
	CW_NIRLIV_ERR_NIR_NO_FACE,				// �������ͼƬû������
	CW_NIRLIV_ERR_VIS_NO_FACE,				// ����ɼ���ͼƬû������
	CW_NIRLIV_ERR_NO_PAIR_FACE,			    // ����ɼ���ͺ���ͼƬ����δ��ƥ��
	CW_NIRLIV_ERR_PUSH_DATA,		        // ��������ʧ��
	CW_NIRLIV_ERR_NUM_LANDMARKS,			// ����ɼ���ͼƬ�ͺ���ͼƬ�ؼ����������
	CW_NIRLIV_ERR_NO_LANDMARKS,			    // �������ͼƬû�������ؼ���
	CW_NIRLIV_ERR_INPUT_IMAGE,		        // �������ͼƬ���߿ɼ���ͼƬ���Ƕ�ͨ��
	CW_NIRLIV_ERR_UNAUTHORIZED,			    // û��license��δ��Ȩ��
	CW_NIRLIV_ERR_FACE_NUM_ERR,			    // δ��������ƥ�俪��ʱ���ɼ�������ͼ����������1
	CW_NIRLIV_ERR_CAM_UNCW,		            // ���ƴӶ�������ͷ
	CW_NIRLIV_ERR_UNKNOWN,					// δ֪���
	CW_NIRLIV_ERR_MAXHANDLE,			    // ������������������Ȩ�����
	CW_NIRLIV_ERR_NIRIMAGE,			        // �������ͼƬ���ݴ���
	CW_NIRLIV_ERR_VISIMAGE,			        // ����ɼ���ͼƬ���ݴ���
} cw_nirliveness_err_t;

	/***************
	* �������ͼ������
	*/
typedef struct cw_nirliv_img
{
	char*				data;			   // ͼ�����ݣ�����Ԥ�����㹻�Ŀռ䣩
	int					dataLen;		   // ���ݳ��ȣ�ֻ��CW_IMAGE_BINARY��ʽ��Ҫ����
	int					width;			   // ��, CW_IMAGE_BINARY��ʽ�ɲ����ã�������ʽ��������
	int					height;			   // ��, CW_IMAGE_BINARY��ʽ�ɲ����ã�������ʽ��������
	cw_img_form_t		format;			   // ͼ���ʽ�������cw_img_form_t
} cw_nirliv_img_t;


   /***************
	* ����������������Ϣ
	*/
typedef struct cw_nirliv_face_param
{
	float                      fSkinScore;		// �����ɫ���ŷ�
	float                      fKeyScore;		// ����ؼ������ŷ�
	cw_point_t                 *pKeypoints;		// ����ؼ�����Ϣ
} cw_nirliv_face_param_t;


	/***************
	* ������������������ͼƬ����
	*/
typedef struct cw_nirliv_detinfo
{
	int   nLandmarks;							// �ؼ������

	cw_nirliv_img_t *pNirImg;                   // NIR����ͼƬ��Ϣ
	cw_nirliv_img_t *pVisImg;                   // VIS�ɼ���ͼƬ��Ϣ�����Ϊ��Ŀ����ͷ��ʽ����ֵ��NULL����

	int nirFaceNum;								// NIR������������
	int visFaceNum;								// VIS�ɼ�����������
	cw_nirliv_face_param_t *pNirInfo;			// NIR����������Ϣ���飨nirFaceNum*sizeof(cw_nirliv_face_param_t)��
	cw_nirliv_face_param_t *pVisInfo;			// VIS�ɼ���������Ϣ���飬���Ϊ��Ŀ����ͷ��ʽ����ֵ��NULL����
} cw_nirliv_detinfo_t;


   /***************
	* �����������
	*/
typedef struct cw_nirliv_res
{
	cw_nirliv_det_rst_t   livRst;			// ������������������ֵ
	float                 score;			// ������������÷֣��ǻ����ʱ��Ϊ0
} cw_nirliv_res_t;


	/***************
	* ��װ��Ȩ��־
	*/
typedef enum cw_install_key_flag
{
	CW_RETERROR_IFEXIST = 0,                //  ���������ʽ��Ȩ���򲻼�����װ�����ش����루����ʹ�ô˱�־����ֹ�����Ȩ��
	CW_INSTALLSTILL_IFEXIST = 1             //  �����Ƿ������ʽ��Ȩ����Ȼ��װ����Ȩ
} cw_install_key_flag_t;


	/***************
	* ��װ��Ȩʱ�����ļ���־
	*/
typedef enum cw_save_key_flag
{
	CW_SAVE_ALL = 0,                //  ��Ȩ�ļ����豸�ļ������棬�ļ������ڵ�ǰ��������Ŀ¼
	CW_SAVE_ONLY_V2C = 1,           //  ������V2C��Ȩ�ļ�
	CW_SAVE_ONLY_C2V = 2,           //  ������C2V�豸�ļ�
	CW_SAVE_NONE = 3                //  �������棬����������ļ�����Ȩ�ޣ�����ʹ�øò�������ֹ����
} cw_save_key_flag_t;


#endif

