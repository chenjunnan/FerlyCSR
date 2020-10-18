
#include "robot_voice_tts_interface.h"


robot_voice_tts_interface::robot_voice_tts_interface() 
{
	
}

robot_voice_tts_interface::~robot_voice_tts_interface()
{
	if (state())quit();
}
