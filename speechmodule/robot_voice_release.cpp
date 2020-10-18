#include "robot_voice_release.h"
#include "ifly/qtts.h"
#include "ifly/qisr.h"
#include "robot_voice_struct.h"
#ifdef CCB_OFFLINEIFLY
#include "ifly/ccb/mt_scylla.h"
#endif // CCB_OFFLINEIFLY
#include "xCommon.h"



robot_voice_release::robot_voice_release()
{
}


robot_voice_release * robot_voice_release::getptr()
{
	static robot_voice_release _o;
	return &_o;

}



void robot_voice_release::run()
{
	while (1)
	{
		Message xMSG = getq();

		switch (xMSG.msgType)
		{
		case _EXIT_THREAD_:

			return ;
			break;
		default:
			;
		}

		if (xMSG.msgType==90&&!xMSG.message.empty())
		{
			//LOGT("SessionEnd %s 1", xMSG.message.c_str());
			QISRSessionEnd(xMSG.message.c_str(), "normal");
			//LOGT("SessionEnd %s 2", xMSG.message.c_str());
		}
#ifdef CCB_OFFLINEIFLY
		typedef int (SCYAPI *Proc_SCYMTSessionEndEx)(const char* session_id);
		if (xMSG.msgType==91&&!xMSG.message.empty())
		{
			LOGD("session end %s", xMSG.message.c_str());
			SCYMTSessionEnd(xMSG.message.c_str());
		}
		if (xMSG.msgType == 92 && !xMSG.message.empty())
		{
			LOGD("session end %s", xMSG.message.c_str());
			((Proc_SCYMTSessionEndEx)xMSG.msgObject)(xMSG.message.c_str());
		}
#endif // CCB_OFFLINEIFLY
	}

}

robot_voice_release::~robot_voice_release()
{
	if (state())quit();
}
