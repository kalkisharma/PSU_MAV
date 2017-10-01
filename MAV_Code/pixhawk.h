//Defines the class that sends data to the pixhawk over USB

#ifndef PIXHAWK_H
#define PIXHAWK_H

#include <termios.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "struct.h"
#include "/home/pi/Documents/mavlink_v2/common/mavlink.h" //changed this :- Mollik

typedef unsigned short int usi;
using namespace std;


extern Buffers buffers;

class Pixhawk{

private:
	mutex mPort;
 
//USB connection
	
	int baudRate;
	std::vector<std::string> devName;

	int file; //file descriptor
	
//pixhawk parameters

	uint64_t getTimeUsec(){
		struct timespec t;
		clock_gettime(CLOCK_REALTIME,&t);
		return (t.tv_sec)*1e6+(t.tv_nsec)/1e3;
	}

	Mavlink_Messages receivedMsgs;

//threading
	std::thread recvThread;
	bool recvThreadActive;
	
	bool msgWaitingToSend;
	
	void getMessages();

	void startReceivingMsgs();
	void stopReceivingMsgs(); 
	void printMessages();

public:

	Pixhawk(int bR, std::vector<std::string> dN);

	//initialize the USB connection. Needs to be called after obj creations
	int init();

	int sendMessage(mavlink_message_t &msg);

	void deInit();

	~Pixhawk(){
		stopReceivingMsgs();
	}
};




#endif
