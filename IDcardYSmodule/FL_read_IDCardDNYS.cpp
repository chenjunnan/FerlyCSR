/*******************************************************
一所读身份证可联网查询
********************************************************/
#include "FL_read_IDCardDNYS.h"
#include "SerialPort.h"
#include <map>
#include <time.h>
#include "xCommon.h"

#include <GdiPlus.h>
using namespace Gdiplus;
#pragma comment(lib,"gdiplus.lib")

using namespace std;

std::map<std::string, std::string> Nation;//民族map
std::map<std::string, std::string> Sex;//性别map
std::map<std::string, std::string>::iterator findData;
int Port;

CSerialPort comDNYS;

void Read_IDCardDNYS::PopErrMsg(char ret)
{
	if (ret == 0xffffff10)
	{
		strcpy(error_buffer, "接收业务终端数据的校验和错.");
	}
	else if (ret == 0xffffff11)
	{
		strcpy(error_buffer, "接收业务终端数据的长度错.");
	}
	else if (ret == 0xffffff21)
	{
		strcpy(error_buffer, "接收业务终端的命令错误，包括命令中的各种数值或逻辑搭配错误.");
	}
	else if (ret == 0xffffff24)
	{
		strcpy(error_buffer, "无法识别的错误.");
	}
	else if (ret == 0xffffff31)
	{
		strcpy(error_buffer, "卡认证机具失败.");
	}
	else if (ret == 0xffffff32)
	{
		strcpy(error_buffer, "机具认证卡失败.");
	}
	else if (ret == 0xffffff33)
	{
		strcpy(error_buffer, "信息验证错误.");
	}
	else if (ret == 0xffffff40)
	{
		strcpy(error_buffer, "错误的卡.");
	}
	else if (ret == 0xffffff41)
	{
		strcpy(error_buffer, "读卡操作失败.");
	}
	else if (ret == 0xffffff47)
	{
		strcpy(error_buffer, "取随机数失败.");
	}
	else if (ret == 0xffffff60)
	{
		strcpy(error_buffer, "身份证阅读器自检失败，不能接收命令.");
	}
	else if (ret == 0xffffff66)
	{
		strcpy(error_buffer, "身份证阅读器没经过授权无法使用.");
	}
	else if (ret == 0xffffff80)
	{
		strcpy(error_buffer, "找卡不成功.");
	}
	else if (ret == 0xffffff81)
	{
		strcpy(error_buffer, "选卡不成功.");
	}
	else if (ret == 0xffffff91)
	{
		strcpy(error_buffer, "该项无内容.");
	}

}

void Read_IDCardDNYS::Init_Dll()
{
	HINSTANCE my_DLL = LoadLibraryA("termbYS.dll");
	if (!my_DLL)
		return;
	//初始化串口
	InitComm = (FL_InitComm)GetProcAddress(my_DLL, "InitComm");
	//初始化串口（扩展）
	InitCommExt = (FL_InitCommExt)GetProcAddress(my_DLL, "InitCommExt");
	//关闭串口
	CloseComm = (FL_CloseComm)GetProcAddress(my_DLL, "CloseComm");
	//卡认证
	Authenticate = (FL_Authenticate)GetProcAddress(my_DLL, "Authenticate");
	//读卡操作
	Read_Content = (FL_Read_Content)GetProcAddress(my_DLL, "Read_Content");
	//读卡操作，将信息文件存储在指定目录
	Read_Content_Path = (FL_Read_Content_Path)GetProcAddress(my_DLL, "Read_Content_Path");
	//获取设备编号
	GetDeviceID = (FL_GetDeviceID)GetProcAddress(my_DLL, "GetDeviceID");
	//获取SAM编号
	GetSAMID = (FL_GetSAMID)GetProcAddress(my_DLL, "GetSAMID");
	//获取bmp照片
	GetPhoto = (FL_GetPhoto)GetProcAddress(my_DLL, "GetPhoto");
	//获取厂商信息和产品信息
	MfrInfo = (FL_MfrInfo)GetProcAddress(my_DLL, "MfrInfo");

	Nation.insert(make_pair("01", "汉"));
	Nation.insert(make_pair("02", "蒙古"));
	Nation.insert(make_pair("03", "回"));
	Nation.insert(make_pair("04", "藏"));
	Nation.insert(make_pair("05", "维吾尔"));
	Nation.insert(make_pair("06", "苗"));
	Nation.insert(make_pair("07", "彝"));
	Nation.insert(make_pair("08", "壮"));
	Nation.insert(make_pair("09", "布依"));
	Nation.insert(make_pair("10", "朝鲜"));
	Nation.insert(make_pair("11", "满"));
	Nation.insert(make_pair("12", "侗"));
	Nation.insert(make_pair("13", "瑶"));
	Nation.insert(make_pair("14", "白"));
	Nation.insert(make_pair("15", "土家"));
	Nation.insert(make_pair("16", "哈尼"));
	Nation.insert(make_pair("17", "哈萨克"));
	Nation.insert(make_pair("18", "傣"));
	Nation.insert(make_pair("19", "黎"));
	Nation.insert(make_pair("20", "傈僳"));
	Nation.insert(make_pair("21", "佤"));
	Nation.insert(make_pair("22", "畲"));
	Nation.insert(make_pair("23", "高山"));
	Nation.insert(make_pair("24", "拉祜"));
	Nation.insert(make_pair("25", "水"));
	Nation.insert(make_pair("26", "东乡"));
	Nation.insert(make_pair("27", "纳西"));
	Nation.insert(make_pair("28", "景颇"));
	Nation.insert(make_pair("29", "柯尔克孜"));
	Nation.insert(make_pair("30", "土"));
	Nation.insert(make_pair("31", "达斡尔"));
	Nation.insert(make_pair("32", "仫佬"));
	Nation.insert(make_pair("33", "羌"));
	Nation.insert(make_pair("34", "布朗"));
	Nation.insert(make_pair("35", "撒拉"));
	Nation.insert(make_pair("36", "毛南"));
	Nation.insert(make_pair("37", "仡佬"));
	Nation.insert(make_pair("38", "锡伯"));
	Nation.insert(make_pair("39", "阿昌"));
	Nation.insert(make_pair("40", "普米"));
	Nation.insert(make_pair("41", "塔吉克"));
	Nation.insert(make_pair("42", "怒"));
	Nation.insert(make_pair("43", "乌孜别克"));
	Nation.insert(make_pair("44", "俄罗斯"));
	Nation.insert(make_pair("45", "鄂温克"));
	Nation.insert(make_pair("46", "德昂"));
	Nation.insert(make_pair("47", "保安"));
	Nation.insert(make_pair("48", "裕固"));
	Nation.insert(make_pair("49", "京"));
	Nation.insert(make_pair("50", "塔塔尔"));
	Nation.insert(make_pair("51", "独龙"));
	Nation.insert(make_pair("52", "鄂伦春"));
	Nation.insert(make_pair("53", "赫哲"));
	Nation.insert(make_pair("54", "门巴"));
	Nation.insert(make_pair("55", "珞巴"));
	Nation.insert(make_pair("56", "基诺"));

	Sex.insert(make_pair("0", "未知"));
	Sex.insert(make_pair("1", "男"));
	Sex.insert(make_pair("2", "女"));
	Sex.insert(make_pair("9", "未说明"));

}


char* WcharToChar2(wchar_t* wc)
{
	int len = WideCharToMultiByte(CP_ACP, 0, wc, wcslen(wc), NULL, 0, NULL, NULL);
	char* m_char = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, wc, wcslen(wc), m_char, len, NULL, NULL);
	m_char[len] = '\0';
	return m_char;
}


//发送打开设备命令
int Read_IDCardDNYS::Send_OpenDev(std::string port)
{
	if (index_open)
	{
		return OP_OK;
	}

	int ret = comDNYS.InitPort(atoi(port.c_str()), 115200);
	Port = atoi(port.c_str());
	if (!ret)
	{
		PopErrMsg(ret);
		return OP_ERR;
	}

	index_open = true;
	return OP_OK;
}

//发送关闭串口命令
void Read_IDCardDNYS::Send_CloseDev()
{
	if (!index_open)
		return;

	comDNYS.closePort();

	index_open = false;
	return;
}

//发送查找卡命令
int Read_IDCardDNYS::Send_FindCard()
{
	int ret;
	char send_data[1024] = { 0x00 };
	send_data[0] = 0xAA;
	send_data[1] = 0xAA;
	send_data[2] = 0xAA;
	send_data[3] = 0x96;
	send_data[4] = 0x69;

	send_data[5] = 0x00;
	send_data[6] = 0x03;

	send_data[7] = 0x20;
	send_data[8] = 0x01;

	send_data[9] = send_data[5] ^ send_data[6];
	for (int i = 7; i < 9; i++)
	{
		send_data[9] = send_data[9] ^ send_data[i];
	}

	comDNYS.WriteData(send_data, 10);

	char RecvData[2048] = { 0 };
	if (comDNYS.ReadData(RecvData, 7) == 0)
	{
		return OP_ERR;
	}

	int len = RecvData[5] << 8 | RecvData[6];
	if (comDNYS.ReadData(RecvData + 7, len) == 0)
	{
		return OP_ERR;
	}

	if (RecvData[9] != 0xffffff9f)
	{
		PopErrMsg(RecvData[9]);
		return OP_ERR;
	}

	return OP_OK;
}

//发送选卡命令
int Read_IDCardDNYS::Send_ChooseCard()
{
	char send_data[1024] = { 0x00 };
	send_data[0] = 0xAA;
	send_data[1] = 0xAA;
	send_data[2] = 0xAA;
	send_data[3] = 0x96;
	send_data[4] = 0x69;

	send_data[5] = 0x00;
	send_data[6] = 0x03;

	send_data[7] = 0x20;
	send_data[8] = 0x02;

	send_data[9] = 0x21;

	comDNYS.WriteData(send_data, 10);

	char RecvData[2048] = { 0 };
	if (comDNYS.ReadData(RecvData, 7) == 0)
	{
		return OP_ERR;
	}

	int len = RecvData[5] << 8 | RecvData[6];
	if (comDNYS.ReadData(RecvData + 7, len) == 0)
	{
		return OP_ERR;
	}

	if (RecvData[9] != 0xffffff90)
	{
		PopErrMsg(RecvData[9]);
		return OP_ERR;
	}
	return OP_OK;

}


//int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
//{
//	UINT num = 0;
//	UINT size = 0;
//
//	ImageCodecInfo* pImageCodecInfo = NULL;
//
//	GetImageEncodersSize(&num, &size);
//	if (size == 0)
//	{
//		return -1;
//	}
//	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
//	if (pImageCodecInfo == NULL)
//	{
//		return -1;
//	}
//
//	GetImageEncoders(num, size, pImageCodecInfo);
//
//	for (UINT j = 0; j< num; ++j)
//	{
//		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
//		{
//			*pClsid = pImageCodecInfo[j].Clsid;
//			free(pImageCodecInfo);
//			return j;
//		}
//	}
//
//	free(pImageCodecInfo);
//	return -1;
//}
//
//BOOL RepeateBmp(char* pchPath, char *chFront, char *chHead, char *chBack, CRTDef_IDInfo_YS &pInfo)
//{
//	char buf[MAX_PATH] = { 0 };
//	wchar_t wbuf[MAX_PATH] = { 0 };
//
//	MultiByteToWideChar(CP_ACP, 0, chFront, strlen(chFront), wbuf, MAX_PATH);
//
//	Image imfront(wbuf);
//	Gdiplus::Graphics graph_imfront(&imfront);
//
//	Bitmap bmfront(imfront.GetWidth(), imfront.GetHeight(), &graph_imfront);
//	Graphics graph_bmfront(&bmfront);
//	graph_bmfront.DrawImage(&imfront, 0, 0, imfront.GetWidth(), imfront.GetHeight());
//
//	SolidBrush blackBrush(Color(255, 0, 0, 0));
//	FontFamily fontfamily(L"幼圆");
//	Font myFont(&fontfamily, 4, FontStyleBold);
//	StringFormat strFormat;
//	int mStrFormat = StringAlignmentNear;
//	PointF ptF;
//	//strFormat.SetAlignment((StringAlignment)mStrFormat);
//
//	ptF.X = 150;
//	ptF.Y = 75;
//	graph_bmfront.DrawString(pInfo.szName, wcslen(pInfo.szName), &myFont, ptF, &strFormat, &blackBrush);
//
//	memset(wbuf, 0, sizeof(wbuf));
//	MultiByteToWideChar(CP_ACP, 0, pInfo.szSex, strlen(pInfo.szSex), wbuf, MAX_PATH);
//	ptF.X = 150;
//	ptF.Y = 143;
//	graph_bmfront.DrawString(wbuf, wcslen(wbuf), &myFont, ptF, &strFormat, &blackBrush);
//
//	memset(wbuf, 0, sizeof(wbuf));
//	MultiByteToWideChar(CP_ACP, 0, pInfo.szNation, strlen(pInfo.szNation), wbuf, MAX_PATH);
//	ptF.X = 320;
//	ptF.Y = 143;
//	graph_bmfront.DrawString(wbuf, wcslen(wbuf), &myFont, ptF, &strFormat, &blackBrush);
//
//	memset(wbuf, 0, sizeof(wbuf));
//	memset(buf, 0, sizeof(buf));
//	memcpy(buf, pInfo.szBornDay, 4);
//	MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf), wbuf, MAX_PATH);
//	ptF.X = 150;
//	ptF.Y = 205;
//	graph_bmfront.DrawString(wbuf, wcslen(wbuf), &myFont, ptF, &strFormat, &blackBrush);
//
//	memset(wbuf, 0, sizeof(wbuf));
//	memset(buf, 0, sizeof(buf));
//	memcpy(buf, pInfo.szBornDay + 4, 2);
//	MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf), wbuf, MAX_PATH);
//	ptF.X = 280;
//	ptF.Y = 205;
//	graph_bmfront.DrawString(wbuf, wcslen(wbuf), &myFont, ptF, &strFormat, &blackBrush);
//
//	memset(wbuf, 0, sizeof(wbuf));
//	memset(buf, 0, sizeof(buf));
//	memcpy(buf, pInfo.szBornDay + 6, 2);
//	MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf), wbuf, MAX_PATH);
//	ptF.X = 360;
//	ptF.Y = 205;
//	graph_bmfront.DrawString(wbuf, wcslen(wbuf), &myFont, ptF, &strFormat, &blackBrush);
//
//	RectF rcF;
//	rcF.X = 150;
//	rcF.Y = 280;
//	rcF.Width = 400;
//	rcF.Height = 120;
//	graph_bmfront.DrawString(pInfo.szAddress, wcslen(pInfo.szAddress), &myFont, rcF, &strFormat, &blackBrush);
//
//	ptF.X = 270;
//	ptF.Y = 448;
//	graph_bmfront.DrawString(pInfo.szIDNum, wcslen(pInfo.szIDNum), &myFont, ptF, &strFormat, &blackBrush);
//
//	memset(wbuf, 0, sizeof(wbuf));
//	MultiByteToWideChar(CP_ACP, 0, chHead, strlen(chHead), wbuf, MAX_PATH);
//	Image imhead(wbuf);
//
//	//
//	Gdiplus::Graphics graph_imhead(&imhead);
//
//	Bitmap bmhead(imhead.GetWidth(), imhead.GetHeight(), &graph_imhead);
//	Graphics graph_bmhead(&bmhead);
//	graph_bmhead.DrawImage(&imhead, 0, 0, imhead.GetWidth(), imhead.GetHeight());
//
//	for (int i = 0; i < 280; i++)
//	{
//		for (int j = 0; j < 320; j++)
//		{
//			int x1 = 550 + i;
//			int y1 = 80 + j;
//			int x2 = i*bmhead.GetWidth() / 280;
//			int y2 = j*bmhead.GetHeight() / 320;
//			if (x2 == bmhead.GetWidth())
//			{
//				x2 = bmhead.GetWidth() - 1;
//			}
//			if (y2 == bmhead.GetHeight())
//			{
//				y2 = bmhead.GetHeight() - 1;
//			}
//			Color clrhead;
//			bmhead.GetPixel(x2, y2, &clrhead);
//			Color clrfront;
//			bmfront.GetPixel(550 + i, 80 + j, &clrfront);
//			if (clrhead.GetR() > 250 && clrhead.GetB() > 250 && clrhead.GetG() > 250)
//			{
//				bmhead.SetPixel(x2, y2, clrfront);
//			}
//		}
//	}
//	//
//
//	graph_bmfront.DrawImage(&bmhead, 550, 80, 280, 320);
//
//	CLSID pngClsid;
//	GetEncoderClsid(L"image/bmp", &pngClsid);
//
//	char szBuf[MAX_PATH] = { 0 };
//	wchar_t szwBuf[MAX_PATH] = { 0 };
//	sprintf(szBuf, "%s\\%s", pchPath, "front.bmp");
//
//	MultiByteToWideChar(CP_ACP, 0, szBuf, strlen(szBuf), szwBuf, MAX_PATH);
//
//	bmfront.Save(szwBuf, &pngClsid, NULL);
//
//	//////////////////////////////////////////////////////////////////////////
//	//
//	memset(wbuf, 0, sizeof(wbuf));
//	MultiByteToWideChar(CP_ACP, 0, chBack, strlen(chBack), wbuf, MAX_PATH);
//
//	Image imback(wbuf);
//	Gdiplus::Graphics graph_imback(&imback);
//
//	Bitmap bmback(imback.GetWidth(), imback.GetHeight(), &graph_imback);
//	Graphics graph_bmback(&bmback);
//	graph_bmback.DrawImage(&imback, 0, 0, imback.GetWidth(), imback.GetHeight());
//
//	ptF.X = 330;
//	ptF.Y = 385;
//	graph_bmback.DrawString(pInfo.szIssued, wcslen(pInfo.szIssued), &myFont, ptF, &strFormat, &blackBrush);
//
//	memset(wbuf, 0, sizeof(wbuf));
//	memset(buf, 0, sizeof(buf));
//	sprintf(buf, "%s-%s", pInfo.szBeginValidity, pInfo.szEndValidity);
//	MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf), wbuf, MAX_PATH);
//	ptF.X = 330;
//	ptF.Y = 455;
//	graph_bmback.DrawString(wbuf, wcslen(wbuf), &myFont, ptF, &strFormat, &blackBrush);
//
//	memset(szBuf, 0, sizeof(szBuf));
//	memset(szwBuf, 0, sizeof(szwBuf));
//	sprintf(szBuf, "%s\\%s", pchPath, "back.bmp");
//
//	MultiByteToWideChar(CP_ACP, 0, szBuf, strlen(szBuf), szwBuf, MAX_PATH);
//
//	bmback.Save(szwBuf, &pngClsid, NULL);
//
//	return TRUE;
//}


//#define RAWIMAGE_X 102
//#define RAWIMAGE_Y 126
//typedef unsigned char byte;
//
////生成bmp图像文件
//int     WriteBMP(char* file, char* Input)
//{
//
//	unsigned char head[1078] = {
//		/***************************/
//		//file header
//		0x42, 0x4d,//file type 
//		//0x36,0x6c,0x01,0x00, //file size***
//		0x0, 0x0, 0x0, 0x00, //file size***
//		0x00, 0x00, //reserved
//		0x00, 0x00,//reserved
//		0x36, 0x4, 0x00, 0x00,//head byte***
//		/***************************/
//		//infoheader
//		0x28, 0x00, 0x00, 0x00,//struct size
//
//		//0x00,0x01,0x00,0x00,//map width*** 
//		0x00, 0x00, 0x0, 0x00,//map width*** 
//		//0x68,0x01,0x00,0x00,//map height***
//		0x00, 0x00, 0x00, 0x00,//map height***
//
//		0x01, 0x00,//must be 1
//		0x08, 0x00,//color count***
//		0x00, 0x00, 0x00, 0x00, //compression
//		//0x00,0x68,0x01,0x00,//data size***
//		0x00, 0x00, 0x00, 0x00,//data size***
//		0x00, 0x00, 0x00, 0x00, //dpix
//		0x00, 0x00, 0x00, 0x00, //dpiy
//		0x00, 0x00, 0x00, 0x00,//color used
//		0x00, 0x00, 0x00, 0x00,//color important
//
//	};
//
//	FILE *fh;
//
//	if ((fh = fopen(file, "wb")) == NULL)
//		return 0;
//
//	int i, j;
//
//
//
//	long num;
//	num = RAWIMAGE_X; head[18] = num & 0xFF;
//	num = num >> 8;  head[19] = num & 0xFF;
//	num = num >> 8;  head[20] = num & 0xFF;
//	num = num >> 8;  head[21] = num & 0xFF;
//
//
//	num = RAWIMAGE_Y; head[22] = num & 0xFF;
//	num = num >> 8;  head[23] = num & 0xFF;
//	num = num >> 8;  head[24] = num & 0xFF;
//	num = num >> 8;  head[25] = num & 0xFF;
//
//
//
//	j = 0;
//	for (i = 54; i<1078; i = i + 4)
//	{
//		head[i] = head[i + 1] = head[i + 2] = j;
//		head[i + 3] = 0;
//		j++;
//	}
//
//	fwrite(head, sizeof(char), 1078, fh);
//
//	for (i = 0; i <= RAWIMAGE_Y - 1; i++)
//	{
//
//		fseek(fh, 1078 * sizeof(char) + (RAWIMAGE_Y - 1 - i)*RAWIMAGE_X, SEEK_SET);
//
//		fwrite((Input + i*RAWIMAGE_X), sizeof(char), RAWIMAGE_X, fh);
//	}
//
//
//	fclose(fh);
//
//	return 1;
//}



//发送读卡命令
int Read_IDCardDNYS::Send_ReadIDData()
{
	memset(error_buffer, 0, 4096);
	char send_data[1024] = { 0x00 };
	send_data[0] = 0xAA;
	send_data[1] = 0xAA;
	send_data[2] = 0xAA;
	send_data[3] = 0x96;
	send_data[4] = 0x69;

	send_data[5] = 0x00;
	send_data[6] = 0x03;

	send_data[7] = 0x30;
	send_data[8] = 0x10;

	send_data[9] = 0x23;

	comDNYS.WriteData(send_data, 10);

	Sleep(500);

	char RecvData[4096] = { 0 };
	if (comDNYS.ReadData(RecvData, 7)==0)
	{
		return OP_ERR;
	}
	
	int len = RecvData[5] << 8 | RecvData[6];
	if (comDNYS.ReadData(RecvData + 7, len) == 0)
	{
		return OP_ERR;
	}

	if (RecvData[9] != 0xffffff90)
	{
		PopErrMsg(RecvData[9]);
		return OP_ERR;
	}

	memset(&IDInfo, 0, sizeof(CRTDef_IDInfo_YS));
	memset(&PRTDInfo, 0, sizeof(CRTDef_PRTDInfo_YS));
	memset(&GATInfo, 0, sizeof(CRTDef_GATResidencePermit));

	int WordLen = RecvData[10] << 8 | RecvData[11];
	int PicLen = RecvData[12] << 8 | RecvData[13];
	int FingerLen = RecvData[14] << 8 | RecvData[15];
	char WordData[2048] = { 0 };
	char PicData[2048] = { 0 };
	char FingerData[2048] = { 0 };

	memcpy(WordData, RecvData + 16, WordLen);

	char country[2] = { 0 };
	memcpy(country, WordData + 248, 2);
	if (country == "I")
	{
		memcpy(PRTDInfo.szEnName, WordData, 120);
		memcpy(PRTDInfo.szSex, WordData + 120, 2);
		memcpy(PRTDInfo.szIDNum, WordData + 122, 30);
		memcpy(PRTDInfo.szAddress, WordData + 152, 6);
		memcpy(PRTDInfo.szCnName, WordData + 158, 30);
		memcpy(PRTDInfo.szBeginValidity, WordData + 188, 16);
		memcpy(PRTDInfo.szEndValidity, WordData + 204, 16);
		memcpy(PRTDInfo.szBornDay, WordData + 220, 16);
		memcpy(PRTDInfo.szVersion, WordData + 236, 4);
		memcpy(PRTDInfo.szIssued, WordData + 240, 8);
		memcpy(PRTDInfo.szType, WordData + 248, 2);

		wchar_t buf[5] = { 0 };
		memcpy(buf, PRTDInfo.szBornDay, 4);
		PRTDInfo.Age = atoi(WcharToChar2(buf));
		time_t nowTime;
		int year;
		nowTime = time(NULL);
		struct tm *sysTime = localtime(&nowTime);
		year = sysTime->tm_year + 1900;
		PRTDInfo.Age = year - PRTDInfo.Age;

		return OP_OK;
	}
	if (country == "J")
	{
		if (FingerLen != 0)
		{
			memcpy(FingerData, RecvData + 16 + WordLen + PicLen, FingerLen);
			memcpy(GATInfo.IDFinger, FingerData, FingerLen);
			GATInfo.FingerLen = FingerLen;
		}

		memcpy(GATInfo.szName, WordData, 30);

		wchar_t szSex[3] = { 0 };
		memcpy(szSex, WordData + 30, 2);
		findData = Sex.find(WcharToChar2(szSex));
		if (findData != Sex.end())
		{
			memcpy(GATInfo.szSex, findData->second.data(), strlen(findData->second.data()));
		}

		memcpy(GATInfo.szBornDay, WordData + 36, 16);
		memcpy(GATInfo.szAddress, WordData + 52, 70);
		memcpy(GATInfo.szIDNum, WordData + 122, 36);
		memcpy(GATInfo.szIssued, WordData + 158, 30);
		memcpy(GATInfo.szBeginValidity, WordData + 188, 16);
		memcpy(GATInfo.szEndValidity, WordData + 204, 16);
		memcpy(GATInfo.szPassNo, WordData + 220, 18);
		memcpy(GATInfo.szIssueNum, WordData + 238, 4);
		memcpy(GATInfo.szIdType, WordData + 248, 2);

		return OP_OK;
	}

	memcpy(PicData, RecvData + 16 + WordLen, PicLen);

	if (FingerLen != 0)
	{
		memcpy(FingerData, RecvData + 16 + WordLen + PicLen, FingerLen);
		memcpy(IDInfo.IDFinger,FingerData,FingerLen);
		IDInfo.FingerLen = FingerLen;
	}

	memcpy(IDInfo.szName, WordData, 30);
	char test[30] = { 0 };
	memcpy(test, WordData, 30);


	wchar_t szSex[3] = { 0 };
	memcpy(szSex, WordData + 30, 2);
	findData = Sex.find(WcharToChar2(szSex));
	if (findData != Sex.end())
	{
		memcpy(IDInfo.szSex, findData->second.data(), strlen(findData->second.data()));
	}

	wchar_t szNation[20] = { 0 };
	memcpy(szNation, WordData + 32, 4);
	findData = Nation.find(WcharToChar2(szNation));
	if (findData != Nation.end())
	{
		memcpy(IDInfo.szNation, findData->second.data(), strlen(findData->second.data()));
	}

	memcpy(IDInfo.szBornDay, WordData + 36, 16);
	memcpy(IDInfo.szAddress, WordData + 52, 70);
	memcpy(IDInfo.szIDNum, WordData + 122, 36);
	memcpy(IDInfo.szIssued, WordData + 158, 30);
	memcpy(IDInfo.szBeginValidity, WordData + 188, 16);
	memcpy(IDInfo.szEndValidity, WordData + 204, 16);

	wchar_t buf[5] = { 0 };
	memcpy(buf, WordData + 36, 8);
	IDInfo.Age = atoi(WcharToChar2(buf));
	time_t nowTime;
	int year;
	nowTime = time(NULL);
	struct tm *sysTime = localtime(&nowTime);
	year = sysTime->tm_year + 1900;
	IDInfo.Age = year - IDInfo.Age;

	Send_CloseDev();

	int ret = InitComm(Port);
	ret = Read_Content_Path("c:\\",1);
	//ret = GetPhoto("c:\\a.bmp");
	CloseComm();

	//RepeateBmp("c:", "c:\\front_cp.bmp", "c:\\back_cp.bmp", "c:\\zp.bmp", IDInfo);

	return OP_OK;
}

std::string Read_IDCardDNYS::get_error()
{
	return error_buffer;
}

