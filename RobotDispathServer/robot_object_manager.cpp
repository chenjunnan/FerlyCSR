#include "robot_object_manager.h"
#include "common\xLogger.h"
#include "voice\robot_voice_module.h"
#include "face\robot_face_module.h"
#include "expression\robot_expression_module.h"
#include "controller\robot_controller_moduel.h"
#include "semantics\robot_semantics_module.h"
#include "ICreader\robot_ICreader_module.h"
#include "printer\robot_print_module.h"
#include "idcard\robot_idcard_module.h"
#include "keypad\robot_keypad_module.h"
#include "finger\robot_finger_module.h"



robot_object_manager::robot_object_manager()
{
	for (int i = 0; i < 20;i++)
		m_object[i] = NULL;
}


robot_object_manager * robot_object_manager::get_ptr()
{
	static robot_object_manager _rom;
	return &_rom;
}

int robot_object_manager::init()
{
	//read sql
	int v = check_one_service(VOICE_SERVICE, IFLY);	if (v){ LOGE("voice service offline error"); }
	int f = check_one_service(FACE_SERVICE, LOCAL);	if (f){ LOGE("face service offline error"); }
	int c = check_one_service(CONTROLLER_SERVICE, LOCAL);	if (c){ LOGE("controller service offline error"); }
	int e = check_one_service(EXPRESSION_SERVICE, LOCAL);	if (e){ LOGE("expression service offline error"); }
	int ic = check_one_service(ICCARD_SERVICE, LOCAL);	if (ic){ LOGE("iccard service offline error"); }
	int id = check_one_service(IDCARD_SERVICE, LOCAL);	if (id){ LOGE("idcard service offline error"); }
	int se = check_one_service(SEMANTICS_SERVICE, LOCAL);		if (se){ LOGE("semantics service offline error"); }
	int pr = check_one_service(PRINT_SERVER, LOCAL);		if (se){ LOGE("print service offline error"); }
	int ke = check_one_service(KEYPAD_SERVER, LOCAL);		if (se){ LOGE("keypad service offline error"); }
	int fi = check_one_service(FINGER_SERVER, LOCAL);		if (se){ LOGE("finger service offline error"); }
	if (v || f || c || ic || id || se || pr || ke || fi)
		return 1;
	return 0;
}

void robot_object_manager::release()
{
	for (int i = 0; i < 20; i++)
	{
		if (m_object[i] != NULL)
			m_object[i]->quit();
		SAFE_DELETE(m_object[i]);
	}		
}

bool robot_object_manager::set_device_type(DEVICE_TYPE type, SERVICE_PROVIDER provider)
{
	if (m_object[type] != NULL)
	{
		m_object[type]->quit();
		SAFE_DELETE(m_object[type]);
	}
	m_type[type] = LOCAL;
	robot_device_interface * temp=NULL;
	switch (type)
	{
	case	VOICE_SERVICE:
	{
		if (provider == IFLY || provider == LOCAL)
			temp = new robot_voice_module;
		if (provider == HCI)
			;
	}
		break;
	case	FACE_SERVICE:
		if (provider == LOCAL)
			temp = new robot_face_module;
		break;
	case	EXPRESSION_SERVICE:
		temp = new robot_expression_module;
		break;
	case	CONTROLLER_SERVICE:
		temp = new robot_controller_moduel;
		break;
	case	ICCARD_SERVICE:
		temp = new robot_ICreader_module;;
		break;
	case	IDCARD_SERVICE:
		temp = new robot_idcard_module;
		break;
	case	SEMANTICS_SERVICE:
		temp = new robot_semantics_module;
		break;
	case	PRINT_SERVER:
		temp = new robot_print_module;
		break;
	case	KEYPAD_SERVER:
		temp = new robot_keypad_module;
		break;
	case	FINGER_SERVER:
		temp = new robot_finger_module;
		break;
	}
	if (temp == NULL)
		return false;
	m_type[type] = provider;
	m_object[type] = temp;
	m_object[type]->start();
	return m_object[type]->check_service() ? false : true;

}

bool robot_object_manager::get(DEVICE_TYPE t, robot_device_interface *&rdi)
{
	if (!check_one_service(t, m_type[t]))
	{
		rdi = m_object[t];
		return true;
	}
	LOGF("%d cant create",t);
	return false;
}

robot_object_manager::~robot_object_manager()
{
}


int robot_object_manager::check_one_service(DEVICE_TYPE t, SERVICE_PROVIDER def)
{
	if (m_object[t] == NULL)
	{
		if (set_device_type(t, def))
			return 0;
		return 1;
	}
	else
	{
		if (m_object[t]->check_service() != 0)
			return 1;
		return 0;
	}
}