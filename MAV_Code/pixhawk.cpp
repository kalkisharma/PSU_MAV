#include "pixhawk.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>

#include "struct.h"

#define DISPLAY_MESSAGES

//Initializes object with USB path and baud rate
Pixhawk::Pixhawk(int bR, std::vector<std::string> dN){
	
	baudRate=bR;
	devName=dN;

	recvThreadActive=false;
	msgWaitingToSend=false;
}

//Initializes the serial connection
int Pixhawk::init(){
	
	speed_t baud;
	switch(baudRate){
		case 57600:baud=B57600;
		break;
		case 115200:baud=B115200;
		break;
		case 921600:baud=B921600;
	}


	file=-1;
	
	int i;

	while(file<0){
		for(i=0;i<devName.size();i++){
	    	//opens USB port
			file=open(devName[i].c_str(),O_RDWR | O_NOCTTY | O_NONBLOCK);
		
			if(file<0){
				perror("ERROR: Could not open serial port");
				//return -1; //failed to open USB
			}
			else{
				cout<<"\nConnected to "<<devName[i];
				break;
			}
		}
		usleep(2000000);
	}

	usleep(1000000);

	#ifdef DISPLAY_MESSAGES	
	cout<<"\nCommunication port to pixhawk opened successfully."<<std::flush;
	#endif

	fcntl(file,F_SETFL,0); //???


	struct termios toptions; //set linux serial options

	//gets current attributes
	if(tcgetattr(file,&toptions)<0){
		perror("ERROR: Could not get serial port term attributes");
		return -2;
	}

	#ifdef DISPLAY_MESSAGES	
	cout<<"\nSerial port term attributes read."<<std::flush;
	#endif

    	
	//px4 options
	toptions.c_iflag &= ~(IGNBRK|BRKINT|ICRNL|INLCR|PARMRK|INPCK|ISTRIP|IXON);
	toptions.c_oflag &= ~(OCRNL|ONLCR|ONLRET|ONOCR|OFILL|OPOST);
	//set parity, stop bit, and data bits
	toptions.c_cflag &= ~(PARENB|CSIZE);
	toptions.c_cflag |= CS8;
	//toptions.c_cflag |= CREAD|CLOCAL;
	toptions.c_lflag &= ~(ICANON|ECHO|ECHONL|ISIG|IEXTEN);
	//return immediately
	toptions.c_cc[VMIN]=1;
	toptions.c_cc[VTIME]=10; //was 0

	#ifdef OLCUC
		toptions.c_oflag &= ~OLCUC;
	#endif

	#ifdef ONOEOT
		toptions.c_oflag &= ~ONOEOT;
	#endif


	//set baud rate
	cfsetspeed(&toptions, baud);


    	//set attributes
	if(tcsetattr(file,TCSANOW|TCSAFLUSH,&toptions)<0){
		perror("ERROR: Could not set serial port term attributes");
		return -3;
	}

	#ifdef DISPLAY_MESSAGES	
	cout<<"\nSerial port term attributes set successfully."<<std::flush;
	#endif

	
	//NEED TO DO ERROR CHECKING HERE to check if they are set properly
	int status;
	ioctl(file,TIOCMGET,&status);
	status|=TIOCM_DTR;
	status|=TIOCM_RTS;
	ioctl(file,TIOCMSET,&status);
	

	startReceivingMsgs();
}



void Pixhawk::deInit()
{
	if(file<0){
		perror("\nNo serial connection exists.");
		return;
	}

	stopReceivingMsgs();

	close(file);

}


//Send data in buffer to the pixhawk
int Pixhawk::sendMessage(mavlink_message_t &msg){
	uint16_t len;
	uint8_t buffer[MAVLINK_MAX_PACKET_LEN]; //mavlink msg to serialized buffer

	int n;
	len=mavlink_msg_to_send_buffer(buffer,&msg);

	msgWaitingToSend=true;

	mPort.lock();
	n=write(file,buffer,len);
	tcdrain(file);
	mPort.unlock();

	msgWaitingToSend=false;

	if(n==len){
		#ifdef DISPLAY_MESSAGES	
		//cout<<"\nMessage transmitted successfully over serial port."<<std::flush;
		#endif
	}

	return n;
}

void Pixhawk::startReceivingMsgs(){
	if(recvThreadActive==false){
		recvThreadActive=true;
		//Spawn read thread for messages
		recvThread=std::thread(&Pixhawk::getMessages,this);
	
		#ifdef DISPLAY_MESSAGES
		cout<<"\nSpawning thread to read serial port messages."<<std::flush;
		#endif
	}
}


void Pixhawk::stopReceivingMsgs(){
	if(recvThreadActive){
		recvThreadActive=false;
		if(recvThread.joinable()){
			recvThread.join();
			#ifdef DISPLAY_MESSAGES
			cout<<"\nJoined thread that read serial port messages."<<std::flush;
			#endif
		}
	}	
}



void Pixhawk::getMessages(){
	uint8_t cp,msgReceived;
	mavlink_status_t status;
	uint16_t len;
	int n;
	mavlink_message_t msg;
	bool receivedAllMsgs=false;
	Mavlink_Messages receivedMsgs;

	/*
	//Counting transfer rate
	int cnt=0;
	uint64_t pCntTime=getTimeUsec();
	*/	

	while(recvThreadActive){

		//cout<<"\nreading from "<<file<<std::flush;

		if(msgWaitingToSend)
			usleep(1); //if this is too small shit hits the fan
		
		//read a byte from serial port
		mPort.lock();
		n=read(file,&cp,1);
		mPort.unlock();

		if(n>0){
			
			/*		
			//counting transfer rate
			cnt++;
			if(getTimeUsec()-pCntTime >1000000){
				cout<<"\nTransfer rate (b/s) = "<<cnt;
				pCntTime=getTimeUsec();
				cnt=0;
			}
			*/

			msgReceived=mavlink_parse_char(MAVLINK_COMM_0,cp,&msg,&status);
			if(msgReceived){
				//telemetry
				if(!buffers.telemetry.push(msg)){
					#ifdef DISPLAY_MESSAGES
					//cout<<"\nTelemetry buffer full. Lost data."<<std::flush;
					#endif
				}

				//implement check for correct sys id at some point
				//or you know just ignore this as usual
				receivedMsgs.sysID=msg.sysid;
				receivedMsgs.compID=msg.compid;
				
				switch(msg.msgid){
					case MAVLINK_MSG_ID_HEARTBEAT:
						mavlink_msg_heartbeat_decode(&msg,&(receivedMsgs.heartbeat));
						receivedMsgs.timeStamps.heartbeat=getTimeUsec();
					break;
					case MAVLINK_MSG_ID_SYS_STATUS:	
						mavlink_msg_sys_status_decode(&msg,&(receivedMsgs.sysStatus));
						receivedMsgs.timeStamps.sysStatus=getTimeUsec();
					break;
					case MAVLINK_MSG_ID_BATTERY_STATUS:	
						mavlink_msg_battery_status_decode(&msg,&(receivedMsgs.batteryStatus));
						receivedMsgs.timeStamps.batteryStatus=getTimeUsec();
					break;						
					case MAVLINK_MSG_ID_RADIO_STATUS:	
						mavlink_msg_radio_status_decode(&msg,&(receivedMsgs.radioStatus));
						receivedMsgs.timeStamps.radioStatus=getTimeUsec();
					break;						
					case MAVLINK_MSG_ID_LOCAL_POSITION_NED:	
						mavlink_msg_local_position_ned_decode(&msg,&(receivedMsgs.localPos));
						receivedMsgs.timeStamps.localPos=getTimeUsec();
					break;						
					case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:	
						mavlink_msg_global_position_int_decode(&msg,&(receivedMsgs.globalPos));
						receivedMsgs.timeStamps.globalPos=getTimeUsec();
					break;						
					case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:	
						mavlink_msg_position_target_local_ned_decode(&msg,&(receivedMsgs.localPosTarget));
						receivedMsgs.timeStamps.localPosTarget=getTimeUsec();
					break;
					case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:	
						mavlink_msg_position_target_global_int_decode(&msg,&(receivedMsgs.globalPosTarget));
						receivedMsgs.timeStamps.globalPosTarget=getTimeUsec();
					break;
					case MAVLINK_MSG_ID_HIGHRES_IMU:	
						mavlink_msg_highres_imu_decode(&msg,&(receivedMsgs.imuData));
						receivedMsgs.timeStamps.imuData=getTimeUsec();
						//std::cout<<"\n\nGot imu data at "<<receivedMsgs.timeStamps.imuData;
	
					break;
					case MAVLINK_MSG_ID_ATTITUDE:	
						mavlink_msg_attitude_decode(&msg,&(receivedMsgs.attitude));
						receivedMsgs.timeStamps.attitude=getTimeUsec();
					break;
					
					case MAVLINK_MSG_ID_ATTITUDE_TARGET:
						mavlink_msg_attitude_target_decode(&msg,&(receivedMsgs.attitudeTarget));
						receivedMsgs.timeStamps.attitudeTarget=getTimeUsec();
					break;

					case MAVLINK_MSG_ID_OPTICAL_FLOW_RAD:
						mavlink_msg_optical_flow_rad_decode(&msg,&(receivedMsgs.opticalFlow));
						receivedMsgs.timeStamps.opticalFlow=getTimeUsec();		
					break;
						
				}
	//no mutex as getting an older msgs doesn't seem like an issue atm
	//might not be the fastest solution
				buffers.messages=receivedMsgs;
				printMessages();
			}//msg received end
		}//n end	
	}//while loop end				
}



void Pixhawk::printMessages(){
	cout<<"\n\n--------------------------------------------------";
	std::cout<<"\nReceived buffers.messages: ";	
	std::cout<<"\n\nGot heartbeat at "<<buffers.messages.timeStamps.heartbeat;
	std::cout<<"\nautopilot: "<<buffers.messages.heartbeat.autopilot;
	std::cout<<"\n\tb: "<<buffers.messages.heartbeat.base_mode;
	std::cout<<"\tc: "<<buffers.messages.heartbeat.custom_mode;
	std::cout<<"\tstatus: "<<buffers.messages.heartbeat.system_status;
				
	std::cout<<"\nGot system status at "<<buffers.messages.timeStamps.sysStatus;
	//This doesn't exist? cout<<"\n\tMode: "<<buffers.messages.sysStatus.mode<<"\t"<<buffers.messages.sysStatus.nav_mode;
	//This doesn't exist? cout<<"\n\tStatus: "<<buffers.messages.sysStatus.status;
	cout<<"\n\tBat vol: "<<buffers.messages.sysStatus.voltage_battery;	
	std::cout<<"\nGot battery status at "<<buffers.messages.timeStamps.batteryStatus;
	std::cout<<"\nGot radio status at "<<buffers.messages.timeStamps.radioStatus;
	std::cout<<"\nGot local position at "<<buffers.messages.timeStamps.localPos;
	std::cout<<"\n\tPos: "<<buffers.messages.localPos.x<<","<<buffers.messages.localPos.y<<","<<buffers.messages.localPos.z;
	std::cout<<"\n\tVel: "<<buffers.messages.localPos.vx<<","<<buffers.messages.localPos.vy<<","<<buffers.messages.localPos.vz;
	std::cout<<"\nGot global position at "<<buffers.messages.timeStamps.globalPos;
	//std::cout<<"\nGot local position target at "<<buffers.messages.timeStamps.localPosTarget;
	//std::cout<<"\n\tPos Tar: "<<buffers.messages.localPosTarget.x<<","<<buffers.messages.localPosTarget.y<<","<<buffers.messages.localPosTarget.z;
	//std::cout<<"\n\tVel Tar: "<<buffers.messages.localPosTarget.vx<<","<<buffers.messages.localPosTarget.vy<<","<<buffers.messages.localPosTarget.vz;
	//std::cout<<"\n\nGot global position target at "<<buffers.messages.timeStamps.globalPosTarget;
	//std::cout<<"\n\nGot imu data at "<<buffers.messages.timeStamps.imuData;
	std::cout<<"\n\nGot distance data from optical flow at "<<buffers.messages.timeStamps.opticalFlow;
	std::cout<<"\n\tDistance: "<<buffers.messages.opticalFlow.distance;
	std::cout<<"\n\nGot attitude at "<<buffers.messages.timeStamps.attitude;
	std::cout<<"\n\tRoll: "<<buffers.messages.attitude.roll<<"\tPitch: "<<buffers.messages.attitude.pitch<<"\tYaw: "<<buffers.messages.attitude.yaw;
	cout<<"\n--------------------------------------------------"<<endl;
}

