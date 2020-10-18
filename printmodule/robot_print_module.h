#pragma once
#include <Windows.h>
#include "ThLPrinterDLL.h"
#include <string>
#include "robot_json_rsp.h"
struct print_message
{
	int appid;
	int messagetype;
	std::string fromguid;
	std::string content;
	int return_state;
	int				   font_size;
	std::string            text;
	bool            under_line;
	int                align;
	bool              recoil;
	bool              bold;
	bool		      nopaper;
	bool		      outofpaper;
	print_message()
	{
		appid = 0;
		messagetype = 0;
		return_state = 0;
		font_size = 0; //0 ascii 1	bin
		under_line = false;
		align = 0;//0 left 1 center 2 right
		recoil = false;
		bold = false;
		outofpaper = false;
		nopaper = false;
	}
};

class robot_print_module 
{
public:
	robot_print_module();
	~robot_print_module();
	int  init();
	void stop();
	bool message_handler(print_message& rcs);
private:
	void close_device();
	void open_device();
	robot_json_rsp robot_json;
	int m_run_print_usb;
};

