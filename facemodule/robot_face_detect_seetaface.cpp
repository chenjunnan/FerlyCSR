#include "robot_face_detect_seetaface.h"
#include "opencv2\imgproc\imgproc.hpp"
#include "SeetaFace\FaceDetection\include\common.h"
#include "common_helper.h"
#include "robot_face_struct.h"


robot_face_detect_seetaface::robot_face_detect_seetaface()
{
	m_detector = NULL;
}


void robot_face_detect_seetaface::setMode(std::string name)
{
	if (!common_check_file_exist(SEETAFACE_DETECT_MODEL_PATH))
	{
		LOGF("face detect model file miss!!!!");
		return;
	}
	m_detector = new seeta::FaceDetection(SEETAFACE_DETECT_MODEL_PATH);


	m_detector->SetScoreThresh(2.f);
	//m_detector->SetImagePyramidScaleFactor(0.8f);
	m_detector->SetWindowStep(4, 4);

	max_yaw = 20;
}

bool robot_face_detect_seetaface::DetectFace(cv::Mat &frame, FaceRect minface, std::vector <FaceRect> &face_postion, std::string ext)
{
	if (frame.empty() || frame.data == nullptr || frame.rows == 0 || frame.cols == 0)
		return false;
	if (!m_detector) return false;

	cv::Mat gray;
	cvtColor(frame, gray, CV_BGR2GRAY);
	face_postion.clear();

	int minface_th = (minface.height + minface.width) / 2;
	m_detector->SetMinFaceSize(minface_th);

	seeta::ImageData img_data;
	img_data.data = gray.data;
	img_data.width = gray.cols;
	img_data.height = gray.rows;
	img_data.num_channels = 1;

	std::vector<seeta::FaceInfo> faceslist = m_detector->Detect(img_data);

	int facecnt = 0;
	for (unsigned int i = 0; i < faceslist.size(); i++) {
		FaceRect face;
		face.x = faceslist[i].bbox.x;
		face.y = faceslist[i].bbox.y;
		face.width = faceslist[i].bbox.width;
		face.height = faceslist[i].bbox.height;

		face.width = (face.x + face.width)>frame.cols ? (frame.cols - face.x-1) : face.width;
		face.height = (face.y + face.height) > frame.rows ? (frame.rows - face.y-1) : face.height;
		if (face.x<0 || face.y<0 ||
			face.width == 0 || face.height == 0 ||
			face.x + face.width>frame.cols ||
			face.y + face.height>frame.rows)
			continue;

		int image_xcentor = frame.cols / 2;
		int image_ycentor = frame.rows / 2;

		double x_centor = 0, y_centor = 0;
		x_centor = face.x + face.width / 2;
		y_centor = face.y + face.height / 2;

		x_centor = x_centor / image_xcentor - 1;
		y_centor = y_centor / image_ycentor - 1;

		face.xoffset = x_centor;
		face.yoffset = y_centor;


		face.score = ((double)faceslist[i].score / 15.0);
		if (face.score > 1)
			face.score = 1.0;

		if (faceslist[i].score > 10)
			face.takephoto = true;

		face.trackId = i;
		face.has3D = true;
		face.yaw = (max_yaw - faceslist[i].score);
		if (face.yaw < 0)
		{
			max_yaw = face.score;
			face.yaw = 0;
		}
		face.age = 0;
		face.valid = 1;
		face.data.setDetectData("seetaface", &faceslist[i], sizeof(seeta::FaceInfo));
		facecnt++;
		face_postion.push_back(face);
	}
	if (facecnt > 0)
		return true;
	return false;
}

robot_face_detect_seetaface::~robot_face_detect_seetaface()
{
	SAFE_DELETE(m_detector);
}
