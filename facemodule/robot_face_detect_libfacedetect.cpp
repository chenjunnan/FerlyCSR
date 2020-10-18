#include "robot_face_detect_libfacedetect.h"
#include "libfacedetect\facedetect-dll.h"
#include "opencv2\imgproc\imgproc.hpp"
#include "xCommon.h"
#include "robot_face_struct.h"


robot_face_detect_libfacedetect::robot_face_detect_libfacedetect()
{
	fun_list["facedetect_frontal"] = &facedetect_frontal;
	fun_list["facedetect_multiview"] = &facedetect_multiview;
	fun_list["facedetect_multiview_reinforce"] = &facedetect_multiview_reinforce;
	fun_list["facedetect_frontal_surveillance"] = &facedetect_frontal_surveillance;
	setMode("facedetect_multiview_reinforce");
	max_yaw = 40;
}


void robot_face_detect_libfacedetect::setMode(std::string name)
{
	if (name.empty())
	{
		std::string temp;
		if (!common_config_get_field_string("libface", "algname", &temp))
			temp = "facedetect_multiview_reinforce";
		common_trim(&temp);
		if (fun_list.find(temp) != fun_list.end())
			m_curfun = temp;
		else
			m_curfun = "facedetect_multiview_reinforce";
	}
	else
	{
		if (fun_list.find(name) != fun_list.end())
			m_curfun = name;
		else
			m_curfun = "facedetect_multiview_reinforce";
	}
		
}

bool robot_face_detect_libfacedetect::DetectFace(cv::Mat &frame, FaceRect minface, std::vector <FaceRect> &face_postion, std::string ext)
{
	if (frame.empty()||frame.data == nullptr || frame.rows == 0 || frame.cols == 0)
		return false;

	cv::Mat gray;
	cvtColor(frame, gray, CV_BGR2GRAY);

	face_postion.clear();
	int * pResults = NULL;
	int minface_th = (minface.height + minface.width) / 2;

	unsigned long  startt = common_get_cur_time_stampms();
	memset(m_result_buffer, 0x00, DETECT_BUFFER_SIZE);
	pResults = fun_list[m_curfun](m_result_buffer,gray.data, gray.cols, gray.rows, gray.step, 1.2f, 2, minface_th, 0,0);
	
	int facecnt = 0;
	for (int i = 0; i < (pResults ? *pResults : 0); i++)
	{
		FaceRect faces;
		short * p = ((short*)(pResults + 1)) + 142 * i;
		faces.x = p[0];
		faces.y = p[1];
		faces.width = p[2];
		faces.height = p[3];

		int neighbors = p[4];
		int angle = p[5];
		//printf("neighbors %d\tangle:%d\n", neighbors,angle);
		faces.width = (faces.x + faces.width)>frame.cols ? (frame.cols - faces.x - 1) : faces.width;
		faces.height = (faces.y + faces.height) > frame.rows ? (frame.rows - faces.y - 1) : faces.height;


		if (faces.x<0 || faces.y<0 ||
			faces.width == 0 || faces.height == 0 ||
			faces.x + faces.width>frame.cols ||
			faces.y + faces.height>frame.rows)
		{
			continue;
		}
		faces.has3D = true;
		faces.yaw = (max_yaw - neighbors);
		if (faces.yaw < 0)
		{
			max_yaw = neighbors;
			faces.yaw = 0;
		}

		int image_xcentor = frame.cols / 2;
		int image_ycentor = frame.rows / 2;

		double x_centor = 0, y_centor = 0;
		x_centor = faces.x + faces.width / 2;
		y_centor = faces.y + faces.height / 2;

		x_centor = x_centor / image_xcentor - 1;
		y_centor = y_centor / image_ycentor - 1;

		faces.xoffset = x_centor;
		faces.yoffset = y_centor;

		if (neighbors>10)
			faces.takephoto = true;

		faces.trackId = i;
		faces.score = 1.0;
		faces.age = 0;
		faces.valid = 1;

		face_postion.push_back(faces);
		facecnt++;
	}

	//LOGI("cost %d\tface %d ", common_get_cur_time_stampms() - startt, face_postion.size());
	if (facecnt > 0)
		return true;
	return false;
}

robot_face_detect_libfacedetect::~robot_face_detect_libfacedetect()
{
}
