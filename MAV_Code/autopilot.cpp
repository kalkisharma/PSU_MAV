#include "autopilot.h"

class MoveInZ:public Behaviour{

	float targetDist;
	float targetTol;

	void ruleBase(){
		int cnt=0;

		param.cPosTarget.z=param.cPos.z+targetDist;
		//param.cPosTarget.y=param.cPos.y;
		//param.cPosTarget.x=param.cPos.x;

		param.cPosTarget.type_mask=MAVLINK_MSG_SET_POS_TAR_L_NED_POSITION;

		while(threadRunning){	
			if(!taskCompleted){
		
				//printf("\ncz: %f, tz: %f, cx: %f, tx: %f",param.cPos.z,param.cPosTarget.z,param.cPos.x,param.cPosTarget.x);

				if(abs(param.cPos.z-param.cPosTarget.z) <= targetTol){
					param.cPosTarget.z=param.cPos.z;
					taskCompleted=true;
				}
			}
			
			usleep(100000);
		}
	}

public:
	MoveInZ(QuadParam &q, float z, float t):Behaviour(q){
		cout<<"\nStarted move in z for "<<z<<" mts."<<std::flush;
		name="moveInZ";
		targetDist=z;
		targetTol=t;
	}

	void start(){
		if(!threadRunning){
			threadRunning=true;
			thread=std::thread(&MoveInZ::ruleBase,this);
		}
	}
	void stop(){
		if(threadRunning){
			threadRunning=false;
			if(thread.joinable())
				thread.join();
		}
	}

	~MoveInZ(){
		cout<<"\nMove in z ended."<<std::flush;
		stop();
	}

};

class Yaw:public Behaviour{

	float targetYaw;
	float targetTol;
	

	void ruleBase(){

		int cnt=1;

		param.cPosTarget.type_mask=MAVLINK_MSG_SET_POS_TAR_L_NED_YAW;

		param.cPosTarget.yaw=param.cYaw+(targetYaw/10);

		while(threadRunning){	
			if(!taskCompleted){
				//printf("\ncy: %f, tz: %f, cx: %f, tx: %f",param.cPos.z,param.cPosTarget.z,param.cPos.x,param.cPosTarget.x);

				if(abs(param.cYaw-param.cPosTarget.yaw) <= targetTol){
					param.cPosTarget.yaw=param.cYaw+(targetYaw/10);
					MoveInZ moveZ(param,param.cPosTarget.z,0.1);
					moveZ.start();
					while(!moveZ.completed());
					moveZ.stop();
					cnt++;
				}
				if(cnt>10){
					taskCompleted=true;
				}
			}
			
			usleep(100000);
		}
	}

public:
	Yaw(QuadParam &q, float y, float t):Behaviour(q){
		cout<<"\nStarted yaw for "<<y<<" mts."<<std::flush;
		name="yaw";
		targetYaw=y;
		targetTol=t;
	}

	void start(){
		if(!threadRunning){
			threadRunning=true;
			thread=std::thread(&Yaw::ruleBase,this);
		}
	}
	void stop(){
		if(threadRunning){
			threadRunning=false;
			if(thread.joinable())
				thread.join();
		}
	}

	~Yaw(){
		cout<<"\nYaw ended."<<std::flush;
		stop();
	}

};


class Takeoff:public Behaviour{
	
	float targetZ;
	float targetY;
	float targetTol;

	void ruleBase(){
		int task=1;
		int cnt=0;

		param.cPosTarget.z=param.iPosTarget.z-targetZ;
		param.cPosTarget.yaw=targetY;
		param.iPosTarget.yaw=targetY; //set takeoff yaw to initial yaw;

		while(threadRunning){	
			if(!taskCompleted){

				//takeoff to height specified	
				if(task==1){
					param.cPosTarget.type_mask=MAVLINK_MSG_SET_POS_TAR_L_NED_POSITION;
					
					param.cPosTarget.x=param.cPos.x;
					param.cPosTarget.y=param.cPos.y;
	
					//printf("\niP: %f, cP: %f, err: %f",param.iPosTarget.z,param.cPos.z,abs(param.cPos.z-param.iPosTarget.z));
					//cout<<"\nCurpos error: "<<abs(param.cPos.z );
					
					if(abs(param.cPos.z-param.cPosTarget.z) <= targetTol){
						task=2;
						param.cPosTarget.z=param.cPos.z;
						param.cPosTarget.x=param.cPos.x;
						param.cPosTarget.y=param.cPos.y;
						param.cPosTarget.type_mask=MAVLINK_MSG_SET_POS_TAR_L_NED_POSITION;
						
					}
					
				}

				//Stabilize and get intial position whenever optical flow starts
				if(task==2){
					//Get intial x and y position whenever optical flow starts
					if(param.cPos.x!=0 && param.cPos.y!=0 && param.iPosTarget.x==0){
						param.iPosTarget.x=param.cPos.x;
						param.iPosTarget.y=param.cPos.y;
						task=3;
						cnt=0;
					}

					param.cPosTarget.x=param.cPos.x;
					param.cPosTarget.y=param.cPos.y;

					param.cPosTarget.type_mask=MAVLINK_MSG_SET_POS_TAR_L_NED_POSITION;
				}
				
				//yaw to desired angle
				if(task==3){
					Yaw yaw(param,param.iPosTarget.yaw,0.035);
					yaw.start();
					while(!yaw.completed());
					yaw.stop();
					task=4;
				}

				//hover after takeoff
				if(task==4){
					cnt++;

					param.cPosTarget.type_mask=MAVLINK_MSG_SET_POS_TAR_L_NED_POSITION;
					param.cPosTarget.x=param.iPosTarget.x;
					param.cPosTarget.y=param.iPosTarget.y;
					param.cPosTarget.z=param.iPosTarget.z-targetZ;

					if(cnt>=5){
						cnt=0;
						task=5;	
					}
					
					usleep(100000);
				}

				else if(task==5){
					taskCompleted=true;
				}

			}

		}
	}

public:
	Takeoff(QuadParam &q, float h, float y, float t):Behaviour(q){
		cout<<"\nTakeoff started to "<<h<<" mts and a yaw of "<<y<<" radians."<<std::flush;
		name="takeoff";
		targetZ=h;
		targetY=y;
		targetTol=t;

	}

	void start(){
		if(!threadRunning){
			threadRunning=true;
			thread=std::thread(&Takeoff::ruleBase,this);
		}
	}
	void stop(){
		if(threadRunning){
			threadRunning=false;
			if(thread.joinable())
				thread.join();
		}
	}

	~Takeoff(){
		cout<<"\nTakeoff ended and current height is "<<param.cPos.z<<std::flush;
		stop();
	}

};



class Land:public Behaviour{
	

	void ruleBase(){
		param.cPosTarget.z=param.iPosTarget.z+0.3;
		while(threadRunning){	
			if(!taskCompleted){
				
				param.cPosTarget.type_mask=MAVLINK_MSG_SET_POS_TAR_L_NED_POSITION;
				param.cPosTarget.x=param.cPos.x;
				param.cPosTarget.y=param.cPos.y;
			
			}

			usleep(100000);
		}
	}

public:
	Land(QuadParam &q):Behaviour(q){
		cout<<"\nLanding started."<<std::flush;
		name="landing";
	}

	void start(){
		if(!threadRunning){
			threadRunning=true;
			thread=std::thread(&Land::ruleBase,this);
		}
	}
	void stop(){
		if(threadRunning){
			threadRunning=false;
			if(thread.joinable())
				thread.join();
		}
	}

	~Land(){
		cout<<"\nLanding ended."<<std::flush;
		stop();
	}

};


class MoveInX:public Behaviour{

	float targetDist;
	float targetTol;

	void ruleBase(){
		int cnt=0;

		//param.cPosTarget.z=param.cPos.z;
		param.cPosTarget.y=param.cPos.y;
		param.cPosTarget.x=param.cPos.x+targetDist;

		param.cPosTarget.type_mask=MAVLINK_MSG_SET_POS_TAR_L_NED_POSITION;

		while(threadRunning){	
			if(!taskCompleted){
		
				//printf("\ncz: %f, tz: %f, cx: %f, tx: %f",param.cPos.z,param.cPosTarget.z,param.cPos.x,param.cPosTarget.x);

				if(abs(param.cPos.x-param.cPosTarget.x) <= targetTol){
					param.cPosTarget.x=param.cPos.x;
					taskCompleted=true;
				}
			}
			
			usleep(100000);
		}
	}

public:
	MoveInX(QuadParam &q, float x, float t):Behaviour(q){
		cout<<"\nStarted move in x for "<<x<<" mts."<<std::flush;
		name="moveInX";
		targetDist=x;
		targetTol=t;
	}

	void start(){
		if(!threadRunning){
			threadRunning=true;
			thread=std::thread(&MoveInX::ruleBase,this);
		}
	}
	void stop(){
		if(threadRunning){
			threadRunning=false;
			if(thread.joinable())
				thread.join();
		}
	}

	~MoveInX(){
		cout<<"\nMove in x ended."<<std::flush;
		stop();
	}

};





class MoveInY:public Behaviour{

	float targetDist;
	float targetTol;

	void ruleBase(){
		int cnt=0;

		//param.cPosTarget.z=param.cPos.z;
		param.cPosTarget.x=param.cPos.x;
		param.cPosTarget.y=param.cPos.y+targetDist;

		param.cPosTarget.type_mask=MAVLINK_MSG_SET_POS_TAR_L_NED_POSITION;

		while(threadRunning){	
			if(!taskCompleted){
		
				//printf("\ncz: %f, tz: %f, cy: %f, ty: %f",param.cPos.z,param.cPosTarget.z,param.cPos.y,param.cPosTarget.y);

				if(abs(param.cPos.y-param.cPosTarget.y) <= targetTol){
					param.cPosTarget.y=param.cPos.y;
					taskCompleted=true;
				}
			}
			
			usleep(100000);
		}
	}

public:
	MoveInY(QuadParam &q, float y, float t):Behaviour(q){
		cout<<"\nStarted move in y for "<<y<<" mts."<<std::flush;
		name="moveInY";
		targetDist=y;
		targetTol=t;
	}

	void start(){
		if(!threadRunning){
			threadRunning=true;
			thread=std::thread(&MoveInY::ruleBase,this);
		}
	}
	void stop(){
		if(threadRunning){
			threadRunning=false;
			if(thread.joinable())
				thread.join();
		}
	}

	~MoveInY(){
		cout<<"\nMove in y ended."<<std::flush;
		stop();
	}

};






class AvoidBound:public Behaviour{

	float targetDist;
	float targetTol;

	Behaviour *obj;

	void ruleBase(){
		
		if(param.imgData.boundary.dir==1)
			obj=new MoveInX(param,-1*targetDist,targetTol);
		if(param.imgData.boundary.dir==2)
			obj=new MoveInY(param,-1*targetDist,targetTol);
		if(param.imgData.boundary.dir==3)
			obj=new MoveInX(param,targetDist,targetTol);
		if(param.imgData.boundary.dir==4)
			obj=new MoveInY(param,targetDist,targetTol);

		while(threadRunning){	
			if(!taskCompleted){
				obj->start();
				while(!obj->completed());
				obj->stop();
				taskCompleted=true;
			}
			
			usleep(100000);
		}
	}

public:
	AvoidBound(QuadParam &q, float d, float t):Behaviour(q){
		cout<<"\nStarted avoid boundary "<<std::flush;
		name="avoidBound";
		targetDist=d;
		targetTol=t;
	}

	void start(){
		if(!threadRunning){
			threadRunning=true;
			thread=std::thread(&AvoidBound::ruleBase,this);
		}
	}
	void stop(){
		if(threadRunning){
			threadRunning=false;
			if(thread.joinable())
				thread.join();
		}
	}

	~AvoidBound(){
		cout<<"\nAvoid bound."<<std::flush;
		stop();
	}

};





class AvoidObject:public Behaviour{

	float targetDist;
	float targetTol;


	void ruleBase(){
		while(!taskCompleted){
			
			MoveInY move1(param,targetDist,targetTol);
			move1.start();
			while(!move1.completed());
			move1.stop();
			
			taskCompleted=true;
		}
	}
public:
	AvoidObject(QuadParam &q, float d, float t):Behaviour(q){
		cout<<"\nStarted avoid object "<<std::flush;
		name="avoidObj";
		targetDist=d;
		targetTol=t;
	}

	void start(){
		if(!threadRunning){
			threadRunning=true;
			thread=std::thread(&AvoidObject::ruleBase,this);
		}
	}
	void stop(){
		if(threadRunning){
			threadRunning=false;
			if(thread.joinable())
				thread.join();
		}
	}

	~AvoidObject(){
		cout<<"\nAvoid object done."<<std::flush;
		stop();
	}

};





class MoveInSquare:public Behaviour{

	float targetLen;
	float targetTol;

	void ruleBase(){

		while(threadRunning){	
			if(!taskCompleted){
				
				MoveInX first(param,targetLen,0.1);
				first.start();
				while(!first.completed());
				first.stop();
				MoveInY second(param,-1*targetLen,0.1);
				second.start();
				while(!second.completed());
				second.stop();
				MoveInX third(param,-1*targetLen,0.1);
				third.start();
				while(!third.completed());
				third.stop();
				MoveInY fourth(param,targetLen,0.1);
				fourth.start();
				while(!fourth.completed());
				fourth.stop();
				
				taskCompleted=true;
			}
			
			usleep(100000);
		}
	}

public:
	MoveInSquare(QuadParam &q, float len, float t):Behaviour(q){
		cout<<"\nStarted move in square of "<<len<<" side."<<std::flush;
		name="moveInSquare";
		targetLen=len;
		targetTol=t;
	}

	void start(){
		if(!threadRunning){
			threadRunning=true;
			thread=std::thread(&MoveInSquare::ruleBase,this);
		}
	}
	void stop(){
		if(threadRunning){
			threadRunning=false;
			if(thread.joinable())
				thread.join();
		}
	}

	~MoveInSquare(){
		cout<<"\nMove in Square ended."<<std::flush;
		stop();
	}

};


class Sleep:public Behaviour{

	void ruleBase(){

		param.cPosTarget.z=param.cPos.z;
		param.cPosTarget.x=param.cPos.x;
		param.cPosTarget.y=param.cPos.y;
		param.cPosTarget.yaw=param.cYaw;
		param.cPosTarget.type_mask=MAVLINK_MSG_SET_POS_TAR_L_NED_POSITION;
		param.cPosTarget.coordinate_frame=MAV_FRAME_LOCAL_OFFSET_NED;

		while(threadRunning){

			//cout<<"\nSleeping"<<std::flush;

		}
	}

public:
	Sleep(QuadParam &q):Behaviour(q){
		cout<<"\nSleep started."<<std::flush;
			name="sleep";
	}

	void start(){
		if(!threadRunning){
			threadRunning=true;
			thread=std::thread(&Sleep::ruleBase,this);
		}
	}

	void stop(){
		if(threadRunning){
			threadRunning=false;
			if(thread.joinable())
				thread.join();
		}
	}

	~Sleep(){
		cout<<"\nSleep stopped."<<std::flush;
		stop();
	}
};







//------------------------------------------
//-----ACTION SELECTION MECHANISM-----------
//------------------------------------------


//Rule base that governs which behaviour is set
void Action::setBehaviour(){
	
	cout<<"\nCurrent behaviours: ";
	for(unsigned int i=0;i<behaviour.size();i++){
		cout<<behaviour[i]->getName()<<" "<<std::flush;
	}

	//If current behaviour is completed then remove it
	if(behaviour.size()){
		if( behaviour[behaviour.size()-1]->completed() ){

			//If the behaviour is done, then change state to appropriate one
			
			//If take off is complete then start searching
			if (behaviour[behaviour.size()-1]->getName()== "takeoff"){
				state=SEARCH;
			}

			//if searching is done then start landing
			if (behaviour[behaviour.size()-1]->getName()== "moveInSquare"){
				state=LAND;
			}
		
			//If done avoiding object then restore previous state
			//and restart the previous behaviour
			if (behaviour[behaviour.size()-1]->getName()== "avoidObj"){
				state=pState;
				behaviour[behaviour.size()-2]->start();
			}


			//Stop the completed behaviour and remove it from the stack
			behaviour[behaviour.size()-1]->stop();
			delete behaviour[behaviour.size()-1];
			behaviour.pop_back();
		}
		
	}

	//If no behaviour then set to sleep
	if(behaviour.size()==0){
		behaviour.push_back(new Sleep(param));
		behaviour[0]->start();
	}

	//For convenience in typing set goal as last behaviour - LIFO
	Behaviour * goal = behaviour[behaviour.size()-1];

	//If current goal is to sleep then clear out other behaviours
	if(goal->getName()== "sleep")
	{
		//Free the pointer and then remove the other behaviours
		//until only the behaviour 'sleep' is left

		vector<Behaviour*>::iterator it;

		for(it=behaviour.begin();it<behaviour.end()-1;it++){
			(*it)->stop();
			delete *it;
			behaviour.erase(it);
		}
	}

	
	if(state==SLEEP && abs(param.cPos.z-param.iPosTarget.z)<0.5){
		//Debug Test to see if there are memory leaks
		if(behaviour.size()!=1){
			cout<<"\nMemory leak somewhere"<<std::flush;
		}
	
		//If only behaviour is sleep, then add behaviour
		behaviour.push_back(new Takeoff(param,1.2, 3.14,0.1)); //takeoff to 1.5m and yaw to 3.14 with tol of 0.1m

		goal->stop(); //Can comment this to cause confusion???
		goal=behaviour[behaviour.size()-1];
		goal->start();

		state=TAKEOFF;

	}

	if(goal->getName()!= "moveInSquare" && state==SEARCH){
		
		//If and state is search then takeoff has completed
		//start square mission
		behaviour.push_back(new MoveInSquare(param,.1,0.1));

		goal->stop(); //Can comment this to cause confusion???
		goal=behaviour[behaviour.size()-1];
		goal->start();

	}

	if(goal->getName()!= "landing" && state==LAND){
		
		behaviour.push_back(new Land(param));

		goal->stop(); //Can comment this to cause confusion???
		goal=behaviour[behaviour.size()-1];
		goal->start();
	}

	/*
	//Avoid obstacle if found if not taking off or landing
	if(param.fSonarDist < 0.5 && state!=TAKEOFF && state!=LAND){
		pState=state;
		
		behaviour.push_back(new AvoidObject(param,0.2,0.1));

		goal->stop(); //Can comment this to cause confusion???
		goal=behaviour[behaviour.size()-1];
		goal->start();
	}
	*/

}


//Event handler. Main function goes here and it decides what
//action to take.
void Action::eventHandler(){

	uint64_t ct=getTimeUsec();

	image.setType(1,0,1,0);

	float comm[8];

	while(threadRunning){

		if(buffers.GSComm.size()){
			if(buffers.GSComm.pop(comm,8)){
				switch((int)comm[0]){
					case 1:
						cout<<"\nArm command: "<<comm[1]<<std::flush;
						if(comm[1]==1){
							rebootPixhawk();
							param.iPosTarget.yaw=7;
							param.iPosTarget.z=0;
						}

						mavlink_msg_command_long_pack(sysID,compID,&msg,1,0,400,1,comm[1],0,0,0,0,0,0);
						
						usleep(1000000);
					break;
				
					case 2:
						cout<<"\nOffboard command: "<<comm[1]<<std::flush;
						mavlink_msg_command_long_pack(sysID,compID,&msg,1,0,92,1,comm[1],0,0,0,0,0,0);
						pixhawk.sendMessage(msg);
						if(comm[1]==1)
							obFlag=true;
						if(comm[1]==0)
							obFlag=false;
					
						usleep(1000000);
					break;
				}
			}
		}
		
		//cout<<"\nBoundary: "<<param.imgData.boundary.dir<<"\ttarget: "<<param.imgData.target.dir;

		//cout<<"\nTime: "<<(float)(getTimeUsec()-ct)/1000.;			
		
		//Sending setpoints at atleast 4 Hz
		//cout<<"\nSending setpoint."<<std::flush;
		
		mavlink_msg_set_position_target_local_ned_encode(sysID,compID,&msg,&param.cPosTarget);
		pixhawk.sendMessage(msg);

		//WORKS CAUSE OUR CONTROLLER IS ALIVE :) :') 
		
		//If offboard then set behaviour
		//if in offboard mode then start going mental. Each hit = 100 pts, partial injury=50 pts
		//cout<<"\nCurrent mode: "<<buffers.messages.heartbeat.custom_mode<<endl;
		
		//Check if offboard mode is selected
		if(buffers.messages.heartbeat.custom_mode==MODE_OFFBOARD && obFlag==false){

			//rebootPixhawk();
			//mavlink_msg_command_long_pack(sysID,compID,&msg,1,0,400,1,1.,0,0,0,0,0,0);
			
			//mavlink_msg_command_long_pack(sysID,compID,&msg,1,0,92,1,1.,0,0,0,0,0,0);
						
			cout<<"\nStarted offboard mode."<<std::flush;
			obFlag=true;
			
		}

		//Check if stabilized mode is selected
		if(buffers.messages.heartbeat.custom_mode==MODE_STABILIZED && obFlag==true){
		//	rebootPixhawk();
			cout<<"\nStarted stabilized mode."<<std::flush;
			//obFlag=false;
			
		}
		

		//If offboard start		
		if(obFlag){

			//update parameters - make sure this is only called when you want to start the mission
			getParameters();
			
			setBehaviour();
		}

		usleep(250000);
		
		
	}
}


void Action::start(){
	if(!threadRunning){
		threadRunning=true;
		thread=std::thread(&Action::eventHandler,this);
		cout<<"\nStarted event handler"<<std::flush;
	}
}

void Action::stop(){
	if(threadRunning){
		threadRunning=false;
		if(thread.joinable()){
			thread.join();
			cout<<"\nStopping event handler"<<std::flush;
		}
	}
}


int Action::rebootPixhawk(){

	uint64_t t=buffers.messages.timeStamps.heartbeat;
	
	pixhawk.deInit();

	cout<<"\nRebooting"<<std::flush;

	firmata_digitalWrite(firmata, 4, HIGH);
	usleep(200000);
	firmata_digitalWrite(firmata, 4, LOW);
	usleep(7000000);

	pixhawk.init();

	while(t == buffers.messages.timeStamps.heartbeat);
	
	cout<<"\nDone rebooting"<<std::flush;
	
	usleep(2000000);

	return 1;
}

