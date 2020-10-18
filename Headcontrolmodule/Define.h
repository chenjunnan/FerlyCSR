#pragma once
#include <string>


struct RobotActionMessage
{
	int timeout=2;
	int opertype=0;
	int luminance;
	int rotateid;
	int rotatetype;
	int rotateangle;
	int anglelevel;
	int anglevertical;
	int speed;
	int R, G, B;
	std::string from;
	std::string response;
};