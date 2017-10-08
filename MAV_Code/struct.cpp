#include "struct.h"


void TimeStamps::reset(){
	heartbeat=0;
	sysStatus=0;
	batteryStatus=0;
	radioStatus=0; //not recv
	localPos=0;
	globalPos=0;//not recb
	localPosTarget=0;//not recv
	globalPosTarget=0; //not recv
	imuData=0;
	attitude=0;
	attitudeTarget=0;
	opticalFlow=0;
}

TimeStamps::TimeStamps(){
	reset();
}



bool TimeStamps::isAllNonZero(){
	return (heartbeat && sysStatus && batteryStatus && radioStatus && localPos
	&& globalPos && localPosTarget && globalPosTarget && imuData && attitude && attitudeTarget && opticalFlow);
}


