#pragma once

enum DEVICE_TYPE
{
	VOICE_SERVICE,
	FACE_SERVICE,
	EXPRESSION_SERVICE,
	CONTROLLER_SERVICE,
	ICCARD_SERVICE,
	IDCARD_SERVICE,
	SEMANTICS_SERVICE,
	PRINT_SERVER,
	KEYPAD_SERVER,
	FINGER_SERVER,
};


enum SERVICE_PROVIDER
{
	IFLY,
	HCI,
	LOCAL,
};


#define POST_MESSAGE(x,msg) \
do{ robot_device_interface *temp_robot_message;\
	if (robot_object_manager::get_ptr()->get((x), temp_robot_message))  \
	temp_robot_message->message_handler((msg));} while (0);    

class robot_object_manager
{
public:
	static robot_object_manager *get_ptr();
	int init();
	void release();
	bool set_device_type(DEVICE_TYPE, SERVICE_PROVIDER);
	bool get(DEVICE_TYPE t, robot_device_interface *&rdi);
	~robot_object_manager();
private:
	int check_one_service(DEVICE_TYPE, SERVICE_PROVIDER def);
	robot_object_manager();
	robot_device_interface   *m_object[20];
	SERVICE_PROVIDER          m_type[20];
};

