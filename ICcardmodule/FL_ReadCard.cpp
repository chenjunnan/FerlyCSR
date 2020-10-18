#include "FL_ReadCard.h"
#include <string>
#include <vector>
#include "xCommon.h"

Read_BankCD::Read_BankCD()
{
	memset(Err_Code,0x00,2048);
	hCom = NULL;

}

Read_BankCD::~Read_BankCD()
{
}

std::string Read_BankCD::get_error()
{
	return Err_Code;
}




	//获取读卡器命令执行失败(返回-1 时)的错误原因;
	//打开设备（波特率默认为 9600bps）
	//按指定的波特率打开设备;
	//关闭指定设备；
	//CRT310复位读卡器；
	//读取设备状态；
	//CTR310走卡位置设置；
	//读磁轨数据；
		
	 

void Read_BankCD::PopErrMsg(int ErrorCode)   //根据返回的错误码转为英文错误表
{

	switch (ErrorCode)
	{
	case -1:
		strcpy(Err_Code, "A given command code is unidentified");
		break;
	case -2:
		strcpy(Err_Code, "Parameter is not correct");
		break;
	case -200:
		strcpy(Err_Code, "A given command code is unidentified");
		break;
	case -201:
		strcpy(Err_Code, "Parameter is not correct");
		break;
	case -202:
		strcpy(Err_Code, "Command execution is impossible");
		break;
	case -204:
		strcpy(Err_Code, "Command data error");
		break;
	case -205:
		strcpy(Err_Code, "Input supply voltage is not within the normal scope of work");
		break;
	case -206:
		strcpy(Err_Code, "Abnormal card(long or short) in the reader");
		break;
	case -207:
		strcpy(Err_Code, "The main power supply power-down");
		break;
	default:
		strcpy(Err_Code, "Communication Error");
		break;
	}
}

bool Read_BankCD::Init(char *port)
{
	char version_buffer[1024] = { 0 };
	if (hCom == NULL){
		hCom = CommOpen(port);
		if (hCom == NULL){
			PopErrMsg(-101);
			return false;
		}
		int ret= GetSysVerion(hCom, version_buffer);
		if (ret != 0)
		{
			Close_Device();
			PopErrMsg(ret);
			LOGE("iccard reader port error:%s.", Err_Code);
			return false;
		}
		LOGI("iccard reader version:%s.", version_buffer);
		ret=CRT310_Reset(hCom, 1);
		if (ret != 0)
		{
			PopErrMsg(ret);
			LOGE("iccard reader reset error:%s.", Err_Code);
		}
		ret = CRT310_CardSetting(hCom, 0x03, 0x01);
		if (ret != 0)
		{
			PopErrMsg(ret);
			LOGE("iccard reader set error:%s.", Err_Code);
		}
		ret = CRT310_CardPosition(hCom, 0x04);
		if (ret != 0)
		{
			PopErrMsg(ret);
			LOGE("iccard reader set Position error:%s.", Err_Code);
		}
		CRT310_LEDSet(hCom, 0x00);
		strcpy(Err_Code, "Success");
		return true;
	}
	strcpy(Err_Code, "Already open device");
	return true;
}

void Read_BankCD::Close_Device()
{
	if (hCom)
		CommClose(hCom);
}
bool Read_BankCD::getstate()
{
	if (!hCom)
		return false;
	unsigned char aatposition = 0, forsetting = 0, rearsetting = 0;
	int errmsg = 0;
	int ret = CRT310_GetStatus(hCom, &aatposition, &forsetting, &rearsetting);
	unsigned char pssstate[6] = { 0 };
	unsigned char ctsw = 0, ksw = 0;
	if (ret)
	{
		GetErrCode(&errmsg);
		PopErrMsg(errmsg);
		return false;
	}
	if (aatposition == 0x4E)
	{
		return false;
	}
	if (aatposition == 0x4A ||aatposition == 0x4B)
		return true;
	return false;
}

int Read_BankCD::ASCII_Read_Track(std::vector<std::string> &out)
{
	return _read_card_imp(0x30,out);

}

int Read_BankCD::BIN_Read_Track(std::vector<std::string> &out)
{
	return _read_card_imp(0x31, out);
}

int Read_BankCD::_read_card_imp(char arga, std::vector<std::string> &out)
{
	if (!hCom)
		return OP_ERR;
	out.clear();
	int errmsg = 0;

	int ret, dat_len, Track1, Track2, Track3;
	std::string Track1_dat, Track2_dat, Track3_dat;
	BYTE Data[4096];
	memset(Data, 0x00, sizeof(Data));
	unsigned char aatposition=0, forsetting=0, rearsetting=0;
	ret = CRT310_GetStatus(hCom, &aatposition, &forsetting, &rearsetting);
	unsigned char pssstate[6] = { 0 };
	unsigned char ctsw = 0,ksw=0;
	if (ret)
	{
		GetErrCode(&errmsg);
		PopErrMsg(errmsg);
		return OP_ERR;
	}
	ret = CRT310_SensorStatus(hCom, &pssstate[0], &pssstate[1], &pssstate[2], &pssstate[3], &pssstate[4], &pssstate[5], &ctsw, &ksw);
	if (ret)
	{
		GetErrCode(&errmsg);
		PopErrMsg(errmsg);
		return OP_ERR;
	}
	if (aatposition == 0x4E )
	{
		return OP_NO_CARD;
	}
	if (aatposition == 0x46 || aatposition == 0x47)
	{
		return OP_UNKNOWN_CARD;
	}
	if (aatposition != 0x4A &&aatposition != 0x4B)
	{
		return OP_NO_CARD;
	}

	ret = MC_ReadTrack(hCom, arga, 0x37, &dat_len, Data);


	if (ret < 0){
		GetErrCode(&errmsg);
		PopErrMsg(errmsg);
		return OP_ERR;
	}


	if (0x45 == ret){
		strcpy(Err_Code, "No Find Card!");
		return OP_NO_CARD;
	}
	else if (ret != 0){
		GetErrCode(&errmsg);
		PopErrMsg(errmsg);
		return OP_ERR;
	}
	else{
		int i;
		Track1 = 0;

		for (i = 1; i < dat_len; i++){
			if (0 == Data[i])
				break;
			if (31 == Data[i]){
				Track2 = i;
				break;
			}
		}
		for (i = Track2 + 1; i < dat_len; i++){
			if (31 == Data[i]){
				Track3 = i;
				break;
			}
		}
		out.push_back(Track1_dat);
		out.push_back(Track2_dat);
		out.push_back(Track3_dat);

		switch (Data[1])
		{
		case 89:
			for (i = 2; i < Track2; i++){
				char c[5] = { 0 };
				sprintf_s(c, 5, "%c", Data[i]);
				Track1_dat.append(c);
			}
			out[0] = Track1_dat;
			break;
		case 78:	
			switch (Data[2])
			{
			case 225:
				Track1_dat = "No start bits (STX)";
				break;
			case 226:
				Track1_dat = "No stop bits (ETX)";
			case 227:
				Track1_dat = "Byte Parity Error(Parity))";
			case 228:
				Track1_dat = "Parity Bit Error(LRC)";
			case 229:
				out[0] = "has card error";
				Track1_dat = "Card Track Data is Blank";
				break;
			}
		default:
			break;
		}

		switch (Data[Track2 + 1])
		{
		case 89:
			for (i = Track2 + 2; i < Track3; i++){
				if (0 == Data[i])
					break;
				char c[5] = { 0 };
				sprintf_s(c, 5, "%c", Data[i]);
				Track2_dat.append(c);
			}
			out[1] = Track2_dat;
			break;
		case 78:

			switch (Data[Track2 + 2])
			{
			case 225:
				Track2_dat = "No start bits (STX)";
				break;
			case 226:
				Track2_dat = "No stop bits (ETX)";
			case 227:
				Track2_dat = "Byte Parity Error(Parity))";
			case 228:
				Track2_dat = "Parity Bit Error(LRC)";
			case 229:
				out[1] = "has card error";
				Track2_dat = "Card Track Data is Blank";
				break;
			}
		default:
			break;
		}

		switch (Data[Track3 + 1])
		{
		case 89:
			for (i = Track3 + 2; i < dat_len; i++){

				char c[5] = { 0 };
				sprintf_s(c, 5, "%c", Data[i]);
				Track3_dat.append(c);
			}
			out[2] = Track3_dat;
			break;
		case 78:

			switch (Data[Track3 + 2])
			{
			case 225:
				Track3_dat = "No start bits (STX)";
				break;
			case 226:
				Track3_dat = "No stop bits (ETX)";
			case 227:
				Track3_dat = "Byte Parity Error(Parity))";
			case 228:
				Track3_dat = "Parity Bit Error(LRC)";
			case 229:
				out[2] = "has card error";
				Track3_dat = "Card Track Data is Blank";
				break;
			}
		default:
			break;
		}

		return OP_OK;
	}
	return OP_ERR;
}


int Read_BankCD::Reset_Card()
{
	int errmsg = 0;
	if (hCom == NULL){
		return OP_ERR;
	}

	int ret;
	ret = CRT310_Reset(hCom, 0x0);
	if (0x45 == ret){
		strcpy(Err_Code, "No Find Card!");
		return OP_NO_CARD;
	}
	if (ret != 0){
		GetErrCode(&errmsg);
		PopErrMsg(errmsg);
		return OP_ERR;
	}

	return OP_OK;
}

int Read_BankCD::Cancel_Card()
{
	int errmsg = 0;
	if (hCom == NULL){
		return OP_ERR;
	}

	int ret;
	ret = CRT310_MovePosition(hCom, 0x1);
	if (ret < 0){
		GetErrCode(&errmsg);
		PopErrMsg(errmsg);
		return OP_ERR;
	}

	switch (ret)
	{
	case 0:
		strcpy(Err_Code, "Success");
		return OP_OK;
		break;
	case 0x4e:
		strcpy(Err_Code, "Error");
		return OP_ERR;
		break;
	case 0x45:
		strcpy(Err_Code, "No Card In The Reader");
		return OP_ERR;
		break;
	case 0x57:
		strcpy(Err_Code, "The card is not on the card operation position");
		return OP_ERR;
		break;
	default:
		strcpy(Err_Code, "Error");
		return OP_ERR;
		break;
	}
}

