#pragma once
#include <string>
#include <vector>


enum MESSAGE_TASK_STATE
{
	MESSAGE_INIT,
	MESSAGE_STEP1,
	MESSAGE_STEP2,
	MESSAGE_STEP3,
	MESSAGE_STEP4,
	MESSAGE_STEP5,
	MESSAGE_STEP6,
	MESSAGE_RESULT,
	MESSAGE_NOTIFY,
	MESSAGE_NORESULT,
	MESSAGE_REDISPATCH,
	MESSAGE_TIMEOUT,
};

class  ROBOTSOCKETMESSAGE
{
public:
	int appID;
	int sendNode;
	int recvNode;
	int serverID;
	int eventID;
	int messageType;

	std::string package_guid;
	std::string to_client_guid;
	std::string fromer_client_guid;

	std::string task_name;

	MESSAGE_TASK_STATE task_state;

	std::string json;
	std::string response;

	std::vector<char> senddata;
	std::vector<char> recvdata;

	int return_state;

	ROBOTSOCKETMESSAGE()
	{
		appID = -1;
		sendNode = 0;
		recvNode = 0;
		serverID = 0;
		eventID = 0;
		messageType = 0;
		task_name = "ROBOT_UNKNOW";
		task_state = MESSAGE_INIT;
		return_state = 0;
	}
	~ROBOTSOCKETMESSAGE()
	{

	}
}; 

enum MESSAGE_DISPATH_TYPE_MAP
{
	NET_DATA_IN=0,
	//////////////////module///////////////////
	MESSAGE_IN_TYPE  = 1001,
	MESSAGE_SLEF_TYPE,
	MESSAGE_MONITOR_TYPE,
	MESSAGE_BINARY_TYPE
};


#define ROBOT_MAIN_SERVER_ID 16
#define ROBOT_EXPRESSION_APP       1001

#define  SOCKET_SENDNODE_LOCAL 0x00000000 
#define  SOCKET_RECVNODE_LOCAL 0x00000000 


#define  SOCKET_FIRST_HEAD    0x03
#define  SOCKET_SECOND_HEAD   0x16


#define VERSION_MODULE	"02.00"//大版本.模块号
#define MODULE_CLASS    0     //模块子版本类型
#define SUBVERSION      8     //模块小版本号.编译时间
