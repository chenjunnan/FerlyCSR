
#include <time.h>
#include "FL_Read_IDCard.h"
#include <string>
#include "xCommon.h"

void Read_IDCD::PopErrMsg(int ret)
{
	switch (ret)
	{
	case -1:
		strcpy(error_buffer, "���δ��Ȩ");
		break;
	case -2:
		strcpy(error_buffer, "��Ȩ�ļ����Ȳ���ȷ");
		break;
	case -3:
		strcpy(error_buffer, "���� BUFFER ʧ��");
		break;
	case -4:
		strcpy(error_buffer, "����ʧ��");
		break;
	case -5:
		strcpy(error_buffer, "����ʧ��");
		break;
	case -6:
		strcpy(error_buffer, "����ʧ��");
		break;
	case -7:
		strcpy(error_buffer, "��ȡ�ͻ� IP ��ַ");
		break;
	case -8:
		strcpy(error_buffer, "δ�ҵ���");
		break;
	case -9:
		strcpy(error_buffer, "�������");
		break;
	case -10:
		strcpy(error_buffer, "δ�ҵ�оƬ��Ϣ");
		break;
	case -11:
		strcpy(error_buffer, "ϵͳ����");
		break;
	case -12:
		strcpy(error_buffer, "���ڲ��ܴ�");
		break;
	case -13:
		strcpy(error_buffer, "���ڲ���д");
		break;
	case -14:
		strcpy(error_buffer, "���ڲ��ܶ�");
		break;
	case -15:
		strcpy(error_buffer, "���ڳ�ʱ");
		break;
	case -16:
		strcpy(error_buffer, "δ�������");
		break;
	case -27:
		strcpy(error_buffer, "��������");
		break;
	case -28:
		strcpy(error_buffer, "û��Ĭ�ϴ�ӡ��");
		break;
	case -29:
		strcpy(error_buffer, "û������סַ��Ϣ");
		break;
	case -30:
		strcpy(error_buffer, "��Ƭ�������");
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

			str1 = GetFPR_ENName();	//Ӣ������
			if (str1 != NULL)
			{
				memcpy(info.name, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_SEX();			//�Ա�
			if (str1 != NULL)
			{
				memcpy(info.sex, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_IDNo();		//���þ���֤����
			if (str1 != NULL)
			{
				memcpy(info.IDno, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_NationCode();//���������ڵ�������
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
			str1 = GetFPR_ValidStartDate();//֤��ǩ������
			if (str1 != NULL)
			{
				memcpy(info.sta_data, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_ValidEndDate();//֤����ֹ����
			if (str1 != NULL)
			{
				memcpy(info.end_data, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_Birthday();//��������
			if (str1 != NULL)
			{
				memcpy(info.B_data, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_IDVersion();//֤���汾��
			if (str1 != NULL)
			{
				memcpy(info.IDversion, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_IssuingAuthorityCode();//��������������ش���
			if (str1 != NULL)
			{
				memcpy(info.Iss_aut, str1, strlen(str1));
				DeleteMBCSString(str1);
			}
			str1 = GetFPR_IDType();//֤�����ͱ�ʶ,��д��ĸI
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
			strcat(m_prth, ("ZP.BMP"));//ͷ��
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

