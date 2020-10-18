
#ifndef __FL_READCARD_H__
#define __FL_READCARD_H__

#include <windows.h>
#include <vector>
#include "CRT_310.h"
#define OP_OK				 0
#define OP_ERR				-1
#define OP_NO_CARD			-2
#define OP_UNKNOWN_ERR		-3
#define OP_UNKNOWN_CARD		-4

class Read_BankCD 
{
public:
	Read_BankCD();
	~Read_BankCD();

public:

	std::string get_error();
	void PopErrMsg(int ErrorCode);//根据返回的错误码转为英文错误表;
	bool Init(char *port);
	int ASCII_Read_Track(std::vector<std::string> &out);
	int BIN_Read_Track(std::vector<std::string> &out);
	int Reset_Card();
	int Cancel_Card();
	void Close_Device();
	bool getstate();
private:
	char Err_Code[2048];
	HANDLE hCom;
	int _read_card_imp(char arga, std::vector<std::string> &out);
};

#endif