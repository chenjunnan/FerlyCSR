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
	char name[128];//������
	char enname[128];//������
	char sex[5];//�Ա�
	char nation[64];//���壻
	char B_data[128];//�������ڣ�
	char address[2048];//��ϸ��ַ��
	char IDno[256];//֤�����룻
	char Iss_aut[1024];//ǩ�����أ�
	char sta_data[128];//��ʼ��Ч���ڣ�
	char end_data[128];//������Ч���ڣ�
	char head_pic[38862];//֤��ͷ��
	char fig_tmp[4096];//ָ�����ݣ�
	char new_add[2048];//���µ�ַ��
	char IDversion[1024];
	char IDtype[1024];
	int old;//���䣻age
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