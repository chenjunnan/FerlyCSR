#ifndef __FL_READ_IDCARD_H__
#define __FL_READ_IDCARD_H__

#include <windows.h>
#include "termb.h"
#include <string>
#define OP_OK				 0
#define OP_ERR				-1
#define OP_NO_CARD			-2
#define OP_UNKNOWN_ERR		-3



typedef struct ID_Info{
	bool isf = false;
	char name[128];//姓名；
	char enname[128];//姓名；
	char sex[5];//性别；
	char nation[64];//民族；
	char B_data[128];//出生日期；
	char address[2048];//详细地址；
	char IDno[256];//证件号码；
	char Iss_aut[1024];//签发机关；
	char sta_data[128];//起始有效日期；
	char end_data[128];//截至有效日期；
	char head_pic[38862];//证件头像
	char fig_tmp[4096];//指纹数据；
	char new_add[2048];//最新地址；
	char IDversion[1024];
	char IDtype[1024];
	int old;//年龄；age
}ID_Info;


class Read_IDCD
{
public:
	Read_IDCD(){ memset(error_buffer, 0x00, 4096); openstatus = false; }
	~Read_IDCD(){}
public:
	int	Open_dev(int port);
	int Close_dev();
	bool Read_IDCard(std::string path);
	std::string get_error();
	ID_Info info;
private:
	bool index_open = false;
	char error_buffer[4096];
	void PopErrMsg(int ret);
	bool openstatus;
};


#endif