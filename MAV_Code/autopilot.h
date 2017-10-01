#ifndef _AUTOPILOT_H_
#define _AUTOPILOT_H_

#include <termios.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <cmath>
#include <atomic>
#include <fstream>

#include "pixhawk.h"
#include "image.h"

#include "struct.h"
#include "/home/pi/Documents/mavlink_v2/common/mavlink.h" //changed this :- Mollik

extern "C"{
#include "/home/pi/Documents/FirmataC/includes/firmata.h"
}

typedef unsigned short int usi;
using namespace std;


#define MAVLINK_MSG_SET_POS_TAR_L_NED_POSITION 		0b0000110111111000
#define MAVLINK_MSG_SET_POS_TAR_L_NED_VELOCITY 		0b0000110111000111
#define MAVLINK_MSG_SET_POS_TAR_L_NED_YAW		0b0000000111111111
#define MAVLINK_MSG_SET_POS_TAR_L_NED_ALL		0b0000110111000000

#define MAVLINK_MSG_SET_POS_TAR_L_NED_TAKEOFF		0x1000

#define MAVLINK_MSG_SET_ATT_EULER_RATE			0b11111000
#define MAVLINK_MSG_SET_ATT_QUATERNION			0b01111111
#define MAVLINK_MSG_SET_ATT_THRUST			0b10111111

#define MODE_OFFBOARD		393216
#define MODE_OFFBOARD2		100925440
#define MODE_STABILIZED		458752
extern Buffers buffers;

struct QuadParam{
	mavlink_set_position_target_local_ned_t iPosTarget;
	mavlink_set_position_target_local_ned_t cPosTarget;
	mavlink_local_position_ned_t cPos;
	float cYaw;

	ImgData imgData;
	
	float fSonarDist;

	QuadParam(){
	}

	QuadParam(const QuadParam &obj){
		if(this!=&obj){
			iPosTarget=obj.iPosTarget;
			cPosTarget=obj.cPosTarget;
			cYaw=obj.cYaw;
			imgData=obj.imgData;
			fSonarDist=obj.fSonarDist;
		}
	}

	QuadParam & operator= (const QuadParam &obj){
		if(this!=&obj){
			iPosTarget=obj.iPosTarget;
			cPosTarget=obj.cPosTarget;
			cYaw=obj.cYaw;
			imgData=obj.imgData;
			fSonarDist=obj.fSonarDist;
		}
		return *this;
	}
};
	
class Behaviour{
protected:
	string name;
	std::thread thread;
	std::atomic<bool> threadRunning;
	std::atomic<bool> taskCompleted;

	QuadParam &param;

	virtual void ruleBase()=0; //rules that govern the behaviour

public:
	Behaviour(QuadParam &q):param(q){
		name="GenericBehaviour";
		threadRunning=false;
		taskCompleted=false;
	}

	virtual ~Behaviour(){};

	virtual void start()=0; //To start the behaviour
	virtual void stop()=0; //To stop the behaviour

	bool completed(){return taskCompleted;}
	string getName(){return name;}
};


class Action{
	
	int sysID;
	int compID;

	Pixhawk &pixhawk;
	Image &image;

	t_firmata *firmata;

	QuadParam param;

	vector<Behaviour*> behaviour;

	std::thread thread;
	bool threadRunning;

	bool obFlag;

	mavlink_message_t msg;

	enum {LAND,SLEEP,TAKEOFF,SEARCH, AVOIDB, AVOIDO}state,pState;

	void getParameters(){

		//Get initial yaw when pixhawk starts
		if(param.iPosTarget.yaw==7 && buffers.messages.timeStamps.attitude){
			param.iPosTarget.yaw=buffers.messages.attitude.yaw;
			param.cPosTarget.yaw=param.iPosTarget.yaw;
			printf("\nGot an initial yaw of: %f",param.iPosTarget.yaw);

		}
		
		//Get initial z value when pixhawk starts
		if(param.iPosTarget.z==0
			&& buffers.messages.timeStamps.localPos!=0 ){
			param.iPosTarget.z=buffers.messages.localPos.z;
			param.cPosTarget.z=param.iPosTarget.z;
			printf("\nGot an initial height of: %f",param.iPosTarget.z);
		}

		
		param.cPos=buffers.messages.localPos;
		param.cYaw=buffers.messages.attitude.yaw;
		param.imgData=image.getImgData();
		
		firmata_pull(firmata);
		param.fSonarDist=(firmata->pins[14].value*5)/1000.; //in mts

	}

	uint64_t getTimeUsec(){
		struct timespec t;
		clock_gettime(CLOCK_REALTIME,&t);
		return (t.tv_sec)*1e3+(t.tv_nsec)/1e3; 
	}

public:
	void setBehaviour();

	Action(Pixhawk &px, Image &img):pixhawk(px),image(img){
		threadRunning=false;
		sysID=0;compID=0;
		state=SLEEP;
		pState=SLEEP;

		param.iPosTarget.target_system=sysID;
		param.iPosTarget.target_component=compID;
		param.iPosTarget.x=0;param.iPosTarget.y=0;param.iPosTarget.z=0;
		param.iPosTarget.vx=0;param.iPosTarget.vy=0;param.iPosTarget.vz=0;
		param.iPosTarget.afx=0;param.iPosTarget.afy=0;param.iPosTarget.afz=0;
		param.iPosTarget.yaw=-7;param.iPosTarget.yaw_rate=0; //yaw cannot be greater than 2*pi
		param.iPosTarget.coordinate_frame=MAV_FRAME_LOCAL_OFFSET_NED;

		param.cPosTarget=param.iPosTarget;
		param.cPosTarget.vx=0;param.cPosTarget.vy=0;param.cPosTarget.vz=0;
		param.cPosTarget.type_mask=MAVLINK_MSG_SET_POS_TAR_L_NED_VELOCITY;
		param.cPosTarget.target_system=0;
		param.cPosTarget.target_component=0;
		
		
		param.cYaw=0;

		param.fSonarDist=-1;

		cout<<"\nStarting connection to UNO."<<std::flush;
		firmata = firmata_new("/dev/ttyS0");

		while(!firmata->isReady) //Wait until device is up
			firmata_pull(firmata);

		cout<<"\nConnected to UNO."<<std::flush;

		firmata_pinMode(firmata, 4, MODE_OUTPUT);
		firmata_pinMode(firmata, 14, MODE_INPUT);

		obFlag=false;
	}

	void eventHandler();
	void start();
	void stop();
	int rebootPixhawk();
};

#endif