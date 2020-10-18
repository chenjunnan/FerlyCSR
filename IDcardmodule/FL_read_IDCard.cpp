
#include <time.h>
#include "FL_Read_IDCard.h"
#include <string>
#include "xCommon.h"

void Read_IDCD::PopErrMsg(int ret)
{
	switch (ret)
	{
	case -1:
		strcpy(error_buffer, "软件未授权");
		break;
	case -2:
		strcpy(error_buffer, "授权文件长度不正确");
		break;
	case -3:
		strcpy(error_buffer, "分配 BUFFER 失败");
		break;
	case -4:
		strcpy(error_buffer, "发送失败");
		break;
	case -5:
		strcpy(error_buffer, "接收失败");
		break;
	case -6:
		strcpy(error_buffer, "连接失败");
		break;
	case -7:
		strcpy(error_buffer, "读取客户 IP 地址");
		break;
	case -8:
		strcpy(error_buffer, "未找到卡");
		break;
	case -9:
		strcpy(error_buffer, "命令不可用");
		break;
	case -10:
		strcpy(error_buffer, "未找到芯片信息");
		break;
	case -11:
		strcpy(error_buffer, "系统错误");
		break;
	case -12:
		strcpy(error_buffer, "串口不能打开");
		break;
	case -13:
		strcpy(error_buffer, "串口不能写");
		break;
	case -14:
		strcpy(error_buffer, "串口不能读");
		break;
	case -15:
		strcpy(error_buffer, "串口超时");
		break;
	case -16:
		strcpy(error_buffer, "未定义错误");
		break;
	case -27:
		strcpy(error_buffer, "参数错误");
		break;
	case -28:
		strcpy(error_buffer, "没有默认打印机");
		break;
	case -29:
		strcpy(error_buffer, "没有最新住址信息");
		break;
	case -30:
		strcpy(error_buffer, "相片解码错误");
		break;
	default:
		break;
	}
}

int	Read_IDCD::Open_dev(int port)
{
	if (openstatus)
		return OP_OK;

	int ret;

	ret = InitComm(port);
	if (ret != 0){
		PopErrMsg(ret);
		return OP_ERR;
	}

	openstatus = true;
	return OP_OK;
}

int Read_IDCD::Close_dev()
{
	if (!openstatus)
		return OP_OK;

	int ret;
	ret = CloseComm();
	if (ret != 0){
		PopErrMsg(ret);
		return OP_ERR;
	}

	openstatus = false;
	return OP_OK;
}

bool Read_IDCD::Read_IDCard(std::string path)
{
	if (!openstatus)
		return false;

	LPSTR str1;

	int ret;
	//ret = IsVerifyKey();
 	LOGI("Authenticate idcard");
	char path_c[1024] = { 0 };
	strcpy(path_c, path.c_str());
	ret = Authenticate();

	if (0 == ret){
		ret = Read_Content_Path(path_c, 6);
		if (ret != 0){
			PopErrMsg(ret);
			return false;
		}				

		memset(&info, 0x00, sizeof(ID_Info));
	
		if (IsFPRIDCard() == 1)
		{
			info.isf = true;

			str1 = GetFPR_ENName();	//英文姓名
			if (str1 != NULL)
			{
				memcpy(info.name, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_SEX();			//性别
			if (str1 != NULL)
			{
				memcpy(info.sex, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_IDNo();		//永久居留证号码
			if (str1 != NULL)
			{
				memcpy(info.IDno, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_NationCode();//国籍或所在地区代码
			if (str1 != NULL)
			{
				memcpy(info.address, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_CHNName();
			if (str1 != NULL)
			{
				memcpy(info.enname, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_ValidStartDate();//证件签发日期
			if (str1 != NULL)
			{
				memcpy(info.sta_data, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_ValidEndDate();//证件终止日期
			if (str1 != NULL)
			{
				memcpy(info.end_data, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_Birthday();//出生日期
			if (str1 != NULL)
			{
				memcpy(info.B_data, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_IDVersion();//证件版本号
			if (str1 != NULL)
			{
				memcpy(info.IDversion, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_IssuingAuthorityCode();//当次申请受理机关代码
			if (str1 != NULL)
			{
				memcpy(info.Iss_aut, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_IDType();//证件类型标识,大写字母I
			if (str1 != NULL)
			{
				memcpy(info.IDtype, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
		}
		else
		{

			str1 = GetName();
			if (str1 != NULL)
			{

				memcpy(info.name, str1, strlen(str1));
		
				DeleteMBCSString(str1);
			}


			str1 = GetNation();
			if (str1 != NULL)
			{
				memcpy(info.nation, str1, strlen(str1));

				DeleteMBCSString(str1);
			}

			str1 = GetSex();
			if (str1 != NULL)
			{
				memcpy(info.sex, str1, strlen(str1));
				DeleteMBCSString(str1);
			}

			str1 = GetBornDate();
			if (str1 != NULL)
			{
				memcpy(info.B_data, str1, strlen(str1));
				char buf[5] = { 0 };
				memcpy(buf, info.B_data, 4);
				info.old = atoi(buf);

				time_t nowTime;
				int year;
				nowTime = time(NULL);
				struct tm *sysTime = localtime(&nowTime);
				year = sysTime->tm_year + 1900;
				info.old = year - info.old;

				DeleteMBCSString(str1);
			}


			str1 = GetAddress();
			if (str1 != NULL)
			{
				memcpy(info.address, str1, strlen(str1));
				DeleteMBCSString(str1);
			}


			str1 = GetIDNo();
			if (str1 != NULL)
			{
				memcpy(info.IDno, str1, strlen(str1));
				DeleteMBCSString(str1);
			}


			str1 = GetSignGov();
			if (str1 != NULL)
			{
				memcpy(info.Iss_aut, str1, strlen(str1));
				DeleteMBCSString(str1);
			}

			str1 = GetStartDate();
			if (str1 != NULL)
			{
				memcpy(info.sta_data, str1, strlen(str1));
				DeleteMBCSString(str1);
			}

			str1 = GetEndDate();

			if (str1 != NULL)
			{
				memcpy(info.end_data, str1, strlen(str1));
				DeleteMBCSString(str1);
			}

			str1 = GetNewAddress();

			if (str1 != NULL)
			{
				memcpy(info.new_add, str1, strlen(str1));
				DeleteMBCSString(str1);
			}


			char m_prth[1024] = {};
			strcpy(m_prth, path.c_str());
			strcat(m_prth, ("ZP.BMP"));//头想
			ret = MakeIDCardImages(m_prth, path_c);

			unsigned char sImg[38862] = { 0 };
			int img_len = 0;
			GetPhoto2(sImg, &img_len);
			memcpy(info.head_pic, sImg, 38862);


			unsigned char sTmp[4096] = { 0 };
			int tmp_len = 0;
			GetFingerPrint2(sTmp, &tmp_len);
			memcpy(info.fig_tmp, sTmp, 4096);
		}

		return true;
	}
	else{
		PopErrMsg(ret);
		return false;
	}
}

std::string Read_IDCD::get_error()
{
	return error_buffer;
}

