// SlamWareRobotControlModule.cpp : 定义 DLL 应用程序的导出函数。
//

#include "bitmap_image.hpp"
#include "SlamWareRobotControlModule.h"
#include <stdio.h>
#include <iostream>
#include <regex>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <rpos/robot_platforms/slamware_core_platform.h>
#include <rpos/features/location_provider/map.h>
#include <rpos/core/pose.h>

#include <rpos/features/location_provider/map.h>
#include <rpos/robot_platforms/slamware_core_platform.h>
#include <rpos/robot_platforms/objects/composite_map.h>
#include <rpos/robot_platforms/objects/composite_map_reader.h>
#include <rpos/robot_platforms/objects/composite_map_writer.h>
#include <rpos/robot_platforms/objects/grid_map_layer.h>
#include <rpos/robot_platforms/objects/line_map_layer.h>
#include <rpos/robot_platforms/objects/pose_map_layer.h>
#include <rpos/robot_platforms/objects/points_map_layer.h>
#include <rpos/features/system_resource/power_management.h>
#include <rpos/system/exception.h>
//#include "bitmap_image.hpp"

using namespace rpos::robot_platforms;
using namespace rpos::robot_platforms::objects;
using namespace rpos::features::location_provider;
using namespace rpos::features::artifact_provider;
using namespace rpos::features::system_resource; 
using namespace rpos::core;
using namespace rpos::actions;
using namespace rpos::features::motion_planner;


const double pi=3.1415926;

std::string ipaddress = "";
const char *ipReg = "\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}";

SlamwareCorePlatform sdp=NULL;
bool				 isconnect=false;
//rpos::actions::MoveAction action;
std::string error_message;
std::vector<tLASERPOINT>	laserpointcache;
std::vector<tSLPOINT>		pathcache;

//"192.168.11.1"
SLAMWAREROBOTCONTROLMODULE_API int InitSlamWare(const char*ip,LOGSINK fun)
{
	try {
		sdp = SlamwareCorePlatform::connect(ip, 1445);
		std::cout <<"SDK Version: " << sdp.getSDKVersion() << std::endl;
		std::cout <<"SDP Version: " << sdp.getSDPVersion() << std::endl;
	} catch(ConnectionTimeOutException& e) {
		std::cout <<e.what() << std::endl;
		return 1;
	} catch(ConnectionFailException& e) {
		std::cout <<e.what() << std::endl;
		return 1;
	}catch(...)
	{
		std::cout <<"unknow error" << std::endl;
		return 1;
	}
	std::cout <<"Connection Successfully" << std::endl;
	isconnect=true;
	return 0;
}

SLAMWAREROBOTCONTROLMODULE_API void Bye()
{
		try{
			sdp.disconnect();
			isconnect=false;
		}
	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
}

SLAMWAREROBOTCONTROLMODULE_API int GetCurrentPosition(double *x ,double *y,double *yaw)
{
	try{
		if(!isconnect)
			return -1;
		rpos::core::Pose pose = sdp.getPose();
		*x= pose.x();
		*y= pose.y();
		*yaw= pose.yaw();
		*yaw=pose.yaw()*180/pi;
		std::cout << "Robot Pose: " << std::endl;
		std::cout << "x: " << pose.x() << ", ";
		std::cout << "y: " << pose.y() << ", ";
		std::cout << "yaw: " << pose.yaw() <<" angle :"<<*yaw<< std::endl;

		return 0;
	}

	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return -1;
}

SLAMWAREROBOTCONTROLMODULE_API int NavigationTo(double x ,double y,double yaw,int mode)
{
	if(!isconnect)
		return -1;
	try {
		rpos::actions::MoveAction action = sdp.getCurrentAction();
		if (action)
			action.cancel();	
		rpos::features::motion_planner::MoveOptions options;
		options.flag = MoveOptionFlag(MoveOptionFlagMilestone | MoveOptionFlagPrecise|MoveOptionFlagWithYaw);
		double tempyaw=(yaw*pi)/180;
		action = sdp.moveTo(rpos::core::Location(x,  y), options,tempyaw);

		return (action.getStatus()==ActionStatusRunning|| action.getStatus()==ActionStatusWaitingForStart)?0:8;	
	}	
	catch (const rpos::system::detail::ExceptionBase& e) {		
		std::cout << e.what() << std::endl;
		return 1;	
	}catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}

	return 0;
}

SLAMWAREROBOTCONTROLMODULE_API enROBOTSTATE WaitNavigation()
{
	try{
		if(!isconnect)
			return STATUS_Disconnect;
		rpos::actions::MoveAction action = sdp.getCurrentAction();
		if(!action)
			return STATUS_ActiveEmpty;
		if(action.isEmpty())
			return STATUS_ActiveEmpty;
		action.waitUntilDone();
		ActionStatus  status=action.getStatus() ;
		if (status== rpos::core::ActionStatusError)
		{
			error_message=action.getReason();
			std::cout << "Action Failed: " << action.getReason() << std::endl;
		}
		return (enROBOTSTATE)(int)status;
	}
	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return STATUS_Disconnect;
}


SLAMWAREROBOTCONTROLMODULE_API const char * GetWaitErrorResult()
{
	return error_message.c_str();
}

SLAMWAREROBOTCONTROLMODULE_API int CancelCurrentAction()
{
	try{
		rpos::actions::MoveAction action = sdp.getCurrentAction();
		if (action)
			action.cancel();	
		return  0;
	}
	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return -1;
}

SLAMWAREROBOTCONTROLMODULE_API int RotateAngleTo(int angle,double speed)
{
	try{
		if(!isconnect)
			return -1;
		rpos::actions::MoveAction action = sdp.getCurrentAction();
		if (action)
			action.cancel();	

		rpos::features::motion_planner::MoveOptions options;
		options.speed_ratio=speed;
		int ori=(angle*pi)/180;
		action=sdp.rotateTo(Rotation(ori), options);
		return action.isEmpty()?1:0;

	}
	
	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return -1;
}

SLAMWAREROBOTCONTROLMODULE_API int RotateAngle(int angle,double speed)
{
	try{
		if(!isconnect)
			return -1;
		rpos::actions::MoveAction action = sdp.getCurrentAction();
		if (action)
			action.cancel();	
		rpos::features::motion_planner::MoveOptions options;
		options.speed_ratio=speed;
		int ori=(angle*pi)/180;
		action=sdp.rotate(Rotation(ori), options);
		return action.isEmpty()?1:0;

	}
	
	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return -1;
}

SLAMWAREROBOTCONTROLMODULE_API int Move(int ori,double speed)
{
	try{
		rpos::features::motion_planner::MoveOptions options;
		options.speed_ratio=speed;
		MoveAction  nowaction;
		switch (ori)
		{
		case 0:
			nowaction=sdp.moveBy(Direction(TURNRIGHT),options);
			//nowaction=sdp.moveBy(0, options);
			break;
		case 1:
			nowaction=sdp.moveBy((45*pi)/180, options);
			break;
		case 2:
			nowaction=sdp.moveBy(Direction(FORWARD),options);
			//nowaction=sdp.moveBy(90, options);
			break;
		case 3:
			nowaction=sdp.moveBy((135*pi)/180, options);
			break;
		case 4:
			nowaction=sdp.moveBy(Direction(TURNLEFT),options);
			//nowaction=sdp.moveBy(180, options);
			break;
		case 5:
			//nowaction=sdp.moveBy((-135*pi)/180, options);
			nowaction=sdp.moveBy((225*pi)/180, options);
			break;
		case 6:
			nowaction=sdp.moveBy(Direction(BACKWARD),options);
			//nowaction=sdp.moveBy(270, options);
			break;
		case 7:
			nowaction=sdp.moveBy((315*pi)/180, options);
			break;
		}
		return nowaction.isEmpty()?1:0;
	}
	
	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return -1;
}


bool saveToBmp(const char * filename, Map map)
{
	std::string finalFilename = filename;

	bitmap_image mapBitmap(map.getMapDimension().x(), map.getMapDimension().y());

	for (size_t posY = 0; posY < map.getMapDimension().y(); ++posY)
	{
		for (size_t posX = 0; posX < map.getMapDimension().x(); ++posX)
		{
			rpos::system::types::_u8 cellValue = ((rpos::system::types::_u8)128U) + map.getMapData()[posX + (map.getMapDimension().y()-posY-1) * map.getMapDimension().x()];
			mapBitmap.set_pixel(posX, posY, cellValue, cellValue, cellValue);
		}
	}
	mapBitmap.save_image(finalFilename);
	return true;
}


SLAMWAREROBOTCONTROLMODULE_API int GetMapBmp(const char* bmppath)
{
	if(!isconnect)
		return -1;
	try {
		rpos::core::RectangleF knownArea = sdp.getKnownArea(MapTypeBitmap8Bit, rpos::features::location_provider::EXPLORERMAP);
		Map map = sdp.getMap(MapTypeBitmap8Bit, knownArea, rpos::features::location_provider::EXPLORERMAP);
		saveToBmp(bmppath,map);
		return 0;
	} catch (const rpos::system::detail::ExceptionBase  & e) {
		fprintf(stderr, "Failed: $s\n", e.what());
		return -2;
	}	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return -1;
}


SLAMWAREROBOTCONTROLMODULE_API int ConvertMap2Bmp(const char* mappath,const char* bmppath)
{
	try {
		bool result = false;
		CompositeMapReader composite_map_reader;
		std::string error_message;
		boost::shared_ptr<CompositeMap> composite_map(composite_map_reader.loadFile(error_message, mappath));
		if (composite_map) {
			for (auto it = composite_map->maps().begin(); it != composite_map->maps().end(); ++it) {
				auto layer = *it;
				std::string usage = layer->getUsage();
				std::string type = layer->getType();
				std::cout << "Layer Usage : " << usage << std::endl;
				//get grid map layer
				if (type == GridMapLayer::Type) {
					auto grid_map = boost::dynamic_pointer_cast<GridMapLayer>(layer);


					std::string finalFilename = bmppath;

					bitmap_image mapBitmap(grid_map->getDimension().x(), grid_map->getDimension().y());

					for (size_t posY = 0; posY < grid_map->getDimension().y(); ++posY)
					{
						for (size_t posX = 0; posX < grid_map->getDimension().x(); ++posX)
						{
							rpos::system::types::_u8 cellValue = ((rpos::system::types::_u8)128U) + grid_map->mapData()[posX + (grid_map->getDimension().y()-posY-1) * grid_map->getDimension().x()];
							mapBitmap.set_pixel(posX, posY, cellValue, cellValue, cellValue);
						}
					}
					mapBitmap.save_image(finalFilename);
					/*std::cout << "Map Position : (" << grid_map->getOrigin().x() << " , " << 
					grid_map->getOrigin().y() << ")" <<std::endl;
					std::cout << "Map Resolution : (" << grid_map->getResolution().x() <<  
					" , " << grid_map->getResolution().y() << ")" <<std::endl;
					std::cout << "Map Dimension: (" << grid_map->getDimension().x() << 
					" , " << grid_map->getDimension().y() << ")" <<std::endl;
					std::cout << "Map Data:" << std::endl;
					for (auto it = grid_map->mapData().begin(); it != grid_map->mapData().end(); ++it) {
					std::cout << (int)*it << " " ;				
					}
					std::cout << std::endl << std::endl;		*/	
				}
				//get line map layer
				else if (type == LineMapLayer::Type) {
					auto line_map = boost::dynamic_pointer_cast<LineMapLayer>(layer);
					for (auto it = line_map->lines().begin(); it != line_map->lines().end(); ++it) {
						auto line = it->second;
						std::cout << "start: (" << line.start.x() << " , " << line.start.y() << ")" << std::endl;
						std::cout << "end: (" << line.end.x() << " , " << line.end.y() << ")" << std::endl;
					}
					std::cout << std::endl;
				}
				//get pose map layer
				else if (type == PoseMapLayer::Type) {
					auto pose_map = boost::dynamic_pointer_cast<PoseMapLayer>(layer);
					for (auto it = pose_map->poses().begin(); it != pose_map->poses().end(); ++it) {					
						auto pos = it->second;
						std::cout << "Position : (" << pos.pose.x() << " , " << pos.pose.y() << ")" << std::endl;
					}	
					std::cout << std::endl;
				}
				else if (type == PointsMapLayer::Type) {                
					//TODO: get Points map layer
					std::cout << std::endl;
				}
				else {
					//TODO: get unknown map layer
					std::cout << std::endl;
				}
			}
			return composite_map->maps().size()==0?2:0;
		}	
		else
			return 1;
	}
	catch (const rpos::system::detail::ExceptionBase &e) {
		std::cout << e.what() << std::endl;
		return 1;	
	}
	return 1;
}


SLAMWAREROBOTCONTROLMODULE_API int GetLaserData(tLASERPOINT **data)
{
	try{
		if(!isconnect)
			return -1;
		rpos::features::system_resource::LaserScan ls = sdp.getLaserScan();
		std::vector<rpos::core::LaserPoint> points = ls.getLaserPoints();
		laserpointcache.clear();
		for(std::vector<rpos::core::LaserPoint>::const_iterator it=points.begin(); it!=points.end(); it++)
		{
			tLASERPOINT temp;
			temp.angle=it->angle();
			temp.distance=it->distance() ;
			temp.value=it->valid();
			laserpointcache.push_back(temp);
			//std::cout << "distance: " << it->distance() << ", angle: " << it->angle() << ", valid: " << (it->valid()?"true":"false") << std::endl;
		}
		*data=laserpointcache.data();
		return laserpointcache.size();
	}

	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return -1;
}

SLAMWAREROBOTCONTROLMODULE_API int ScanMap()
{
	try{
		if(!isconnect)
			return -1;

		return sdp.setMapUpdate(true)?0:1;
	}
	
	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return -1;
}


SLAMWAREROBOTCONTROLMODULE_API int CancelScanMap()
{
	try{
		if(!isconnect)
			return -1;

		return sdp.setMapUpdate(false)?0:1;

	}
	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return -1;
}

SLAMWAREROBOTCONTROLMODULE_API int SaveMap(const char* path)
{
	try{
		if(!isconnect)
			return -1;

		sdp.setMapUpdate(false);
		CompositeMap composite_map = sdp.getCompositeMap();
		CompositeMapWriter composite_map_writer;
		std::string error_message;
		bool result = composite_map_writer.saveFile(error_message, path, composite_map);

		return result;
	}
	
	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return -1;
}

SLAMWAREROBOTCONTROLMODULE_API int SetMap(double x ,double y,double yaw,const char* path)
{
	if(!isconnect)
		return -1;
	CompositeMapReader composite_map_reader;
	std::string error_message;
	double tempyaw=(yaw*pi)/180;
	 rpos::core::Pose pose(Location(x,y),Rotation(tempyaw));
	 try
	 {
		 boost::shared_ptr<CompositeMap> composite_map(composite_map_reader.loadFile(error_message, path));
		 if (composite_map) {
			 sdp.clearMap();
			 sdp.setCompositeMap((*composite_map), pose);	
			 return 0;
		 }else
		 {
			 std::cout<<"set map error "<<error_message<<std::endl;
		 }
	 }
	 catch (...)
	 {
		 std::cout<<"set map error "<<error_message<<std::endl;
	 }
	
	return 1;
}

SLAMWAREROBOTCONTROLMODULE_API int ReLocalization(int count,tSLPOINT *list/*=0 */)
{
	try{
		if(!isconnect)
			return -1;
		rpos::actions::MoveAction action = sdp.getCurrentAction();
		if (action)
			action.cancel();

		sdp.setMapLocalization(true);
		//RectangleF temp=sdp.getKnownArea(MapTypeBitmap8Bit, EXPLORERMAP);

		action=sdp.recoverLocalization(RectangleF(0,0,0,0));

		return (action.getStatus()==ActionStatusRunning|| action.getStatus()==ActionStatusWaitingForStart)?0:1;
	}

	catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return -1;
}

SLAMWAREROBOTCONTROLMODULE_API enROBOTSTATE GetRobotStatus()
{
	try{
		if(!isconnect)
			return STATUS_ActionStatusError;
		MoveAction  tempaction = sdp.getCurrentAction();
		return (enROBOTSTATE)(int)tempaction.getStatus();
	}catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return STATUS_ActionStatusError;
}

SLAMWAREROBOTCONTROLMODULE_API int GoHome()
{
	try{
		if(!isconnect)
			return -1;
		rpos::actions::MoveAction action = sdp.getCurrentAction();
		if (action)
			action.cancel();
		action =sdp.goHome();
		return (action.getStatus()==ActionStatusRunning|| action.getStatus()==ActionStatusWaitingForStart)?0:1;
	}catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return -1;
}

SLAMWAREROBOTCONTROLMODULE_API bool SetHome(double x ,double y,double yaw)
{
	try{
		if(!isconnect)
			return false;
		double tempyaw=(yaw*pi)/180;
		return sdp.setHomePose(Pose(Location(x,y),Rotation(tempyaw)));
	}catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return false;
}

SLAMWAREROBOTCONTROLMODULE_API int GetBattery()
{
	try{
		if(!isconnect)
			return -1;
		return sdp.getBatteryPercentage();
	}catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return 0;
}


SLAMWAREROBOTCONTROLMODULE_API int GetChargeStatus(tSlamRobotChargeState *data)
{
	if(!isconnect)
		return -1;
	try{
			memset(data,0x00,sizeof(tSlamRobotChargeState));
			BaseHealthInfo  bh=sdp.getRobotHealth();
			if(bh.hasSdpDisconnected)
				data->sdpconnected=!(*bh.hasSdpDisconnected);

			if(data->sdpconnected==false)
				return 0;

			if(bh.hasSystemEmergencyStop)
				data->emergencystop=(*bh.hasSystemEmergencyStop);
			PowerStatus  fa=sdp.getPowerStatus();
			data->isCharging=fa.isCharging;
			data->isDCConnect=fa.isDCConnected;
			data->onDock=fa.dockingStatus;
			data->PowerStageState=fa.powerStage;
		}catch(...)
		{
			std::cout <<"unknow error" << std::endl;
		}
	return 0;
}

SLAMWAREROBOTCONTROLMODULE_API int ClearAllError()
{
	try
	{
		rpos::features::system_resource::BaseHealthInfo health=sdp.getRobotHealth();
		if(health.errors.size())
		{

			for(int i=0;i<health.errors.size();i++)
			{

				sdp.clearRobotHealth(health.errors[i].errorCode);
			}
		}
	}catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return 0;
}

SLAMWAREROBOTCONTROLMODULE_API bool SetSpeed(int speed)
{	
	try
	{
		if(!isconnect)
			return false;
		if(speed==0)
			return sdp.setSystemParameter(SYSPARAM_ROBOT_SPEED, SYSVAL_ROBOT_SPEED_HIGH);
		else if(speed==1)
			return sdp.setSystemParameter(SYSPARAM_ROBOT_SPEED, SYSVAL_ROBOT_SPEED_LOW);
		else
			return sdp.setSystemParameter(SYSPARAM_ROBOT_SPEED, SYSVAL_ROBOT_SPEED_MEDIUM);
	}catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return false;
}


SLAMWAREROBOTCONTROLMODULE_API int ResetRobot(bool softmode)
{
	try
	{
		bool ret=false;
		if(softmode)
			ret=sdp.restartModule(rpos::features::system_resource::RestartModeSoft);
		else
			ret=sdp.restartModule(rpos::features::system_resource::RestartModeHard);

		return ret?0:1;

	}catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return 2;
}

SLAMWAREROBOTCONTROLMODULE_API int ShutdownRobot(int shutdowntime,int starttime)
{
	try
	{
		bool ret=false;
		rpos::core::SlamcoreShutdownParam slamarags;
		slamarags.restartTimeIntervalMinute=starttime;
		slamarags.shutdownTimeIntervalMinute=shutdowntime;
		ret=sdp.shutdownSlamcore(slamarags);
		return ret?0:1;

	}catch(...)
	{
		std::cout <<"unknow error" << std::endl;
	}
	return 2;
}


SLAMWAREROBOTCONTROLMODULE_API int GetRemainPath(tSLPOINT **path)
{
	if(!isconnect)
		return -1;



	try
	{
		rpos::actions::MoveAction action = sdp.getCurrentAction();
		if(!action)
			return -2;
		if(action.isEmpty())
			return -3;

		pathcache.clear();
		rpos::features::motion_planner::Path milestones = action.getRemainingMilestones();
		std::vector<rpos::core::Location> points = milestones.getPoints();
		if(points.size()<=0)
		{
			return 0;
		}

		for(std::vector<rpos::core::Location>::const_iterator it=points.begin(); it!=points.end(); it++)
		{
			tSLPOINT temp;
			temp.x=it->x();
			temp.y=it->y();
			pathcache.push_back(temp);
		}
		*path=pathcache.data();
		return pathcache.size();
	}
	catch(RequestTimeOutException& e) {
		std::cout << e.what() << std::endl;
	}
	catch(...)
	{
		std::cout <<"unknow error" << std::endl;

	}
	return 0;
}

int main()
{
	std::cout <<"test slam robot"<< std::endl;

	if(InitSlamWare("192.168.11.1",NULL))
	{

		std::cout<<"connect robot failed !"<<std::endl;
		return 0;
	}

	rpos::core::Location location = sdp.getLocation();
	std::cout << "Robot Location: " << std::endl;
	std::cout << "x: " << location.x() << ", ";
	std::cout << "y: " << location.y() << std::endl;

	rpos::core::Pose pose = sdp.getPose();
	std::cout << "Robot Pose: " << std::endl;
	std::cout << "x: " << pose.x() << ", ";
	std::cout << "y: " << pose.y() << ", ";
	std::cout << "yaw: " << pose.yaw() << std::endl;

	int battPercentage = sdp.getBatteryPercentage();
	std::cout <<"Battery: " << battPercentage << "%" << std::endl;
	MoveAction  nowaction;
	int switchcommand=0;
	double x,y,yam;

	while (1)
	{
		switchcommand=getchar();
		switch(switchcommand)
		{
		case 'a':
			{
		
				if(GetCurrentPosition(&x,&y,&yam))
				{
					std::cout<<"get robot position failed !"<<std::endl;

				}else
				{
					std::cout<<"get robot position "<<x<< y << yam<<"  !"<<std::endl;

				}
			}
			break;
		case 'b':
			{
				if(NavigationTo(x+1,y,yam,1))
				{
					std::cout<<"get robot navigation failed !"<<std::endl;

				}else
				{
					std::cout<<"navigation start "<<std::endl;
				}
				
			}
			break;
		case 'd':
			std::cout<<"wait navigation "<<std::endl;
			 WaitNavigation();
			 std::cout<<"wait navigation done"<<std::endl;
			break;
		case 'r':
			CancelCurrentAction();
			 std::cout<<"cancel navigation done"<<std::endl;
			break;
		case 'h':
			RotateAngle(45,0.8);
			break;
		case 'c':
			ScanMap();
			break;
		case 's':
			SaveMap("test.stcm");
			ConvertMap2Bmp("test.stcm","test.bmp");
			break;

		case 'i':
			SetMap(0,0,0,"test.stcm");
			break;
		case 'u':
			std::cout<<"wait ReLocalization "<<std::endl;
			ReLocalization(0);
			WaitNavigation();
			std::cout<<"wait ReLocalization done"<<std::endl;
			break;
		case '1':
			Move(1,0.5);
			break;
		case '3':
			Move(3,0.5);
			break;
		case '5':
			Move(5,0.5);
			break;
		case '7':
			Move(7,0.5);
			break;
		case 't':
		{
			tLASERPOINT * ptr=nullptr;
			GetLaserData(&ptr);

		}break;
		case 'm':
			{
				GetMapBmp("ii.bmp");
			}break;
		case 'n':
			{
				tSlamRobotChargeState temp;
				GetChargeStatus(&temp);

			}break;
		default:
			;
		}
	}
	return 0;

}



// bool DisplayMapLayerInfo(const std::string file_path) {
// 	bool result = false;
// 	CompositeMapReader composite_map_reader;
// 	std::string error_message;
// 	boost::shared_ptr<CompositeMap> composite_map(composite_map_reader.loadFile(error_message, file_path));
// 	if (composite_map) {
// 		for (auto it = composite_map->maps().begin(); it != composite_map->maps().end(); ++it) {
// 			auto layer = *it;
// 			std::string usage = layer->getUsage();
// 			std::string type = layer->getType();
// 			std::cout << "Layer Usage : " << usage << std::endl;
// 			//get grid map layer
// 			if (type == GridMapLayer::Type) {
// 				auto grid_map = boost::dynamic_pointer_cast<GridMapLayer>(layer);
// 				std::cout << "Map Position : (" << grid_map->getOrigin().x() << " , " << 
// 					grid_map->getOrigin().y() << ")" <<std::endl;
// 				std::cout << "Map Resolution : (" << grid_map->getResolution().x() <<  
// 					" , " << grid_map->getResolution().y() << ")" <<std::endl;
// 				std::cout << "Map Dimension: (" << grid_map->getDimension().x() << 
// 					" , " << grid_map->getDimension().y() << ")" <<std::endl;
// 				std::cout << "Map Data:" << std::endl;
// 				// 				for (auto it = grid_map->mapData().begin(); it != grid_map->mapData().end(); ++it) {
// 				// 					std::cout << (int)*it << " " ;				
// 				// 				}
// 				std::cout << std::endl << std::endl;			
// 			}
// 			//get line map layer
// 			else if (type == LineMapLayer::Type) {
// 				auto line_map = boost::dynamic_pointer_cast<LineMapLayer>(layer);
// 				for (auto it = line_map->lines().begin(); it != line_map->lines().end(); ++it) {
// 					auto line = it->second;
// 					std::cout << "start: (" << line.start.x() << " , " << line.start.y() << ")" << std::endl;
// 					std::cout << "end: (" << line.end.x() << " , " << line.end.y() << ")" << std::endl;
// 				}
// 				std::cout << std::endl;
// 			}
// 			//get pose map layer
// 			else if (type == PoseMapLayer::Type) {
// 				auto pose_map = boost::dynamic_pointer_cast<PoseMapLayer>(layer);
// 				for (auto it = pose_map->poses().begin(); it != pose_map->poses().end(); ++it) {					
// 					auto pos = it->second;
// 					std::cout << "Position : (" << pos.pose.x() << " , " << pos.pose.y() << ")" << std::endl;
// 				}	
// 				std::cout << std::endl;
// 			}
// 			else if (type == PointsMapLayer::Type) {                
// 				//TODO: get Points map layer
// 				std::cout << std::endl;
// 			}
// 			else {
// 				//TODO: get unknown map layer
// 				std::cout << std::endl;
// 			}
// 		}
// 		return true;
// 	}	
// 	return true;
// }
 //int main()
 //{
 //	std::cout <<"test slam robot"<< std::endl;
 //
 //	
 //
 //	rpos::core::Location location = sdp.getLocation();
 //	std::cout << "Robot Location: " << std::endl;
 //	std::cout << "x: " << location.x() << ", ";
 //	std::cout << "y: " << location.y() << std::endl;
 //
 //	rpos::core::Pose pose = sdp.getPose();
 //	std::cout << "Robot Pose: " << std::endl;
 //	std::cout << "x: " << pose.x() << ", ";
 //	std::cout << "y: " << pose.y() << ", ";
 //	std::cout << "yaw: " << pose.yaw() << std::endl;
 //
 //	int battPercentage = sdp.getBatteryPercentage();
 //	std::cout <<"Battery: " << battPercentage << "%" << std::endl;
 //	MoveAction  nowaction;
 //	int switchcommand=0;
 //
 //	while (1)
 //	{
 //		switchcommand=getchar();
 //		switch(switchcommand)
 //		{
 //		case 'a':
 //			{
 //				rpos::features::motion_planner::MoveOptions options;
 //				options.speed_ratio=0.5;
 //				int count=5;
 //				//while(count>=0)
 //				{
 //
 //					nowaction=sdp.moveBy(Direction(FORWARD), options);
 //					count--;
 //					//	Sleep(200);
 //				}
 //
 //
 //			}break;
 //		case 'b':
 //			{
 //				if(sdp.getMapUpdate())
 //				{
 //
 //					std::cout<<"MapUpdate true"<<std::endl;
 //					sdp.setMapUpdate(false);
 //				}
 //				else
 //				{
 //
 //					std::cout<<"MapUpdate false"<<std::endl;
 //					sdp.setMapUpdate(true);
 //				}
 //
 //
 //
 //			}break;
 //		case 'd':
 //			{
 //				StcmMapWriter("A.stcm",sdp);
 //				DisplayMapLayerInfo("A.stcm");
 //
 //			}break;
 //		case 'r':
 //			{
 //				RectangleF temp=sdp.getKnownArea(MapTypeBitmap8Bit, EXPLORERMAP);
 //
 //				nowaction=sdp.recoverLocalization(temp);
 //			}break;
 //		case 'h':
 //			{
 //				rpos::features::system_resource::BaseHealthInfo health=sdp.getRobotHealth();
 //				if(health.errors.size())
 //				{
 //
 //					for(int i=0;i<health.errors.size();i++)
 //					{
 //
 //						sdp.clearRobotHealth(health.errors[i].errorCode);
 //					}
 //				}
 //
 //			}break;
 //		case 'c':
 //			{
 //				nowaction.cancel();
 //
 //			}break;
 //		default:
 //			;
 //		}
 //	}
 //	Sleep(-1);
 //	return 0;
 //
 //}


// bool parseCommandLine(int argc, const char * argv[])
// {
// 	bool opt_show_help = false;
// 
// 	for (int pos=1; pos<argc; ++pos )
// 	{
// 		const char * current = argv[pos];
// 		if(strcmp(current, "-h") == 0) {
// 			opt_show_help = true;
// 		} else {
// 			ipaddress = current;
// 		}
// 	}
// 
// 	std::regex reg(ipReg);
// 	if(! opt_show_help && ! std::regex_match(ipaddress, reg)) 
// 	{
// 		opt_show_help = true;
// 	}
// 
// 	if (opt_show_help)
// 	{
// 		showHelp("moveandpathdemo");
// 		return false;
// 	}
// 
// 	return true;
// }

// int main(int argc, const char * argv[])
// {
// 	if(! parseCommandLine(argc, argv) )
// 	{
// 		return 1;
// 	}
// 
// 	std::vector<rpos::core::Location> pointsToGo;
// 	pointsToGo.push_back(rpos::core::Location(2, 2));
// 	pointsToGo.push_back(rpos::core::Location(-2, 2));
// 	pointsToGo.push_back(rpos::core::Location(-2, -2));
// 	pointsToGo.push_back(rpos::core::Location(2, -2));
// 
// 	
// 	int i = 0;
// 	while(true)
// 	{
// 		try
// 		{
// 			rpos::actions::MoveAction moveAction = sdp.getCurrentAction();
// 
// 			if(moveAction)
// 			{
// 				std::cout << (moveAction.isEmpty()? "Empty" : "Non-Empty") << std::endl;
// 				std::cout << "Action ID: " << moveAction.getActionId() << std::endl;
// 				std::cout << "Action Name: " << moveAction.getActionName() << std::endl;
// 				boost::this_thread::sleep_for(boost::chrono::milliseconds(2000));
// 
// 				if(moveAction.getStatus()==rpos::core::ActionStatusFinished) 
// 				{
// 					moveAction = sdp.moveTo(pointsToGo, false, true);
// 				}
// 			}
// 			else
// 			{
// 				std::cout << "Empty" << std::endl;
// 				std::cout << "Action Name: " << "EmptyAction" << std::endl;
// 				boost::this_thread::sleep_for(boost::chrono::milliseconds(2000));
// 
// 				moveAction = sdp.moveTo(pointsToGo, false, true);
// 			}
// 
// 			while(moveAction)
// 			{
// 				std::cout << (moveAction.isEmpty()? "Empty" : "Non-Empty") << std::endl;
// 				std::cout << "Action ID: " << moveAction.getActionId() << std::endl;
// 				std::cout << "Action Name: " << moveAction.getActionName() << std::endl;
// 				boost::this_thread::sleep_for(boost::chrono::milliseconds(2000));
// 
// 				if(i%4==2) {
// 					moveAction.cancel();
// 				} else if(i%4==3) {
// 					std::cout << "Wait until done" << std::endl;
// 					moveAction.waitUntilDone();
// 					std::cout << "Done" << std::endl;
// 				}
// 
// 				boost::this_thread::sleep_for(boost::chrono::milliseconds(2000));
// 				rpos::features::motion_planner::Path milestones = moveAction.getRemainingMilestones();
// 				std::vector<rpos::core::Location> points = milestones.getPoints();
// 
// 				std::cout << "Remaining Milestones: " << std::endl;
// 				for(std::vector<rpos::core::Location>::const_iterator it=points.begin(); it!=points.end(); it++)
// 				{
// 					std::cout << "(" << it->x() << ", ";
// 					std::cout << it->y() << ")" << std::endl;
// 				}
// 				boost::this_thread::sleep_for(boost::chrono::milliseconds(2000));
// 
// 				rpos::features::motion_planner::Path path = moveAction.getRemainingPath();
// 				std::vector<rpos::core::Location> locations = path.getPoints();
// 
// 				if(locations.size()<=0)
// 				{
// 					break;
// 				}
// 
// 				std::cout << "Remaining Path: " << std::endl;
// 				for(std::vector<rpos::core::Location>::const_iterator it=locations.begin(); it!=locations.end(); it++)
// 				{
// 					std::cout << "(" << it->x() << ", ";
// 					std::cout << it->y() << ")" << std::endl;
// 				}
// 
// 				rpos::core::Location location = sdp.getLocation();
// 				std::cout << "Robot Location: " << std::endl;
// 				std::cout << "x: " << location.x() << ", ";
// 				std::cout << "y: " << location.y() << std::endl;
// 
// 				rpos::core::Pose pose = sdp.getPose();
// 				std::cout << "Robot Pose: " << std::endl;
// 				std::cout << "x: " << pose.x() << ", ";
// 				std::cout << "y: " << pose.y() << ", ";
// 				std::cout << "yaw: " << pose.yaw() << std::endl;
// 
// 				int battPercentage = sdp.getBatteryPercentage();
// 				std::cout <<"Battery: " << battPercentage << "%" << std::endl;
// 			}
// 		} catch(ConnectionFailException e) {
// 			std::cout << e.what() << std::endl;
// 			break;
// 		} catch(RequestTimeOutException& e) {
// 			std::cout << e.what() << std::endl;
// 		}
// 
// 		i++;
// 	}
// 
// 	return 0;
// }