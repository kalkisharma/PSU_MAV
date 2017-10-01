/*Defines class Connect that uses sockets to implement TCP communication
with the command center*/

#ifndef CONNECT
#define CONNECT
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>
#include <thread>
#include <mutex>
#include "struct.h"
#include "image.h"

extern Buffers buffers;



class Connect{
	
	std::string serverIP;
	int serverPortNo;

	int sockfd[2]; //socket descriptor, port no, status variable
	struct sockaddr_in serv_addr;
	struct hostent *server;

	uint64_t tPrevHb; //mSecs

	std::thread sendThread[2];
	bool sendThreadActive[2];

	std::thread receiveThread;
	bool receiveThreadActive;

	uint64_t getTimeUsec(){
		struct timespec t;
		clock_gettime(CLOCK_REALTIME,&t);
		return (t.tv_sec)*1e6+(t.tv_nsec)/1e3; 
	}

	bool socketAlive;

	void startSendThread();
	void stopSendThread();
	void startReceiveThread();
	void stopReceiveThread();

	void receiveData();
	void checkStatus();
	void sendData();
	void sendDataImg();
	
public:
    //initializes socket (ip address, port no)
    //needs to be called after declaring object
	Connect(std::string a, int b);
	~Connect();
	int init();

};

#endif
