#include "connect.h"
#include <linux/types.h>
#include <iostream>
#include "struct.h"

#define DISPLAY_MESSAGES

using namespace std;

//Constructor
Connect::Connect(std::string a, int b){    
    serverIP=a;
	serverPortNo=b;
	socketAlive=true;
	sendThreadActive[0]=false;
	sendThreadActive[1]=false;
	receiveThreadActive=false;
}

//Destructor
Connect::~Connect(){
	stopReceiveThread();
	stopSendThread();
}

int Connect::init(){
	//set TCP socket
    //Initialize two socket file descriptors
	for(int i=0;i<2;i++){
		sockfd[i]=socket(AF_INET, SOCK_STREAM,0);
		if(sockfd[i]<0){
			perror("\nERROR: Cannot open socket");
			return -1;
		}
	}
    //Send server IP and get host name
    //Function returns  pointer to hostent struct
    //Function fills variable values of struct server
	server=gethostbyname(serverIP.c_str());

    //If gethostbyname returns null then print error
	if(server==NULL){
		perror("\nERROR: No such host\n");
		return -2;
	}

	//Initialize connection for telem
    //Function intializes struct to NULL
 	bzero((char *) &serv_addr, sizeof(serv_addr));
    //Set value of struct variable sin_family to AF_INET
    //sin_family stands for socket in family
    //AF_INET is an address family and designates that the socket can comminicate with IPv4 addresses
    serv_addr.sin_family = AF_INET;
    //Copy h_addr in struct server to sin_addr.s_addr in struct serv_addr
    //Copy host address to socket address
    bcopy((char *)server->h_addr,
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    //Convert host to network byte address and store in socker port
    serv_addr.sin_port = htons(serverPortNo);
    
    //Connect socket to address of the server using first socket file descriptor
	if (connect(sockfd[0],(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        perror("\nERROR: Cannot connect to server 0");
		return -3;
	}
	
    //Connect socket to address of the server using second socket file descriptor
	if (connect(sockfd[1],(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        perror("\nERROR: Cannot connect to server 1");
		return -3;
	}

	//Set read timeout to .1 sec
	struct timeval tv;
	tv.tv_sec=0;
	tv.tv_usec=100000;
	setsockopt(sockfd[0],SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
	setsockopt(sockfd[1],SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
	
	tPrevHb=getTimeUsec();
	
	startSendThread();
	startReceiveThread();

	return 1;
}


void Connect::startSendThread(){
	
	if(sendThreadActive[0]==false){
		sendThreadActive[0]=true;
		//Spawn thread for sending data
		sendThread[0]=std::thread(&Connect::sendData,this);
	
		#ifdef DISPLAY_MESSAGES
		cout<<"\nSpawning thread to send data.";
		#endif
	}
	
	if(sendThreadActive[1]==false){
		sendThreadActive[1]=true;
		//Spawn thread for sending data
		sendThread[1]=std::thread(&Connect::sendDataImg,this);
	
		#ifdef DISPLAY_MESSAGES
		cout<<"\nSpawning thread to send images.";
		#endif
	}
}


void Connect::sendData(){

	mavlink_message_t msg;
	uint16_t len;
	uint16_t numBytes=0;
	uint8_t buffer[buffers.telemLen*MAVLINK_MAX_PACKET_LEN];
	int n;
	uint8_t req;
	while(sendThreadActive[0]){

		int numMsgs=buffers.telemetry.size();
	
		if(numMsgs){
		
			numBytes=0;
	
			while(buffers.telemetry.pop(msg) && numMsgs>0){				
				len=mavlink_msg_to_send_buffer(buffer+numBytes,&msg); 				
				numBytes+=len;
			}
	
			uint16_t *ptr2=&numBytes;
	
			n=write(sockfd[0],ptr2,sizeof(uint16_t));
			if(n<0)
				perror("ERROR: Could not send telemetry size");
			else{
				n=write(sockfd[0],buffer,numBytes);
				if(n<0)
					perror("ERROR: Could not send telemetry bytes");
			}
		}
	}
}


void Connect::sendDataImg(){

	uint16_t numFrames;
	uint32_t numBytes=0;
	int n;
	cv::Mat frame;

	while(sendThreadActive[1]){

		int numFrames=buffers.sendFrame.size();

		if(numFrames){
		
			buffers.sendFrame.pop(frame);
			
			frame.reshape(0,1);
			numBytes=frame.total()*frame.elemSize(); 			
	
			n=write(sockfd[1],frame.data,numBytes);
			if(n<0)
				perror("ERROR: Could not send image bytes");
			
			
		}
	}
}


void Connect::checkStatus(){
	uint64_t t=getTimeUsec()-tPrevHb;

	
	if(t>2000000){
		perror("ERROR: Connection dropped");

		stopSendThread();
		stopReceiveThread();
		close(sockfd[0]);
		close(sockfd[1]);
		
		usleep(500000);

		#ifdef DISPLAY_MESSAGES
		cout<<"\nRestarting connection to server.";
		#endif

		int cnt=0;
		while(cnt < 5){
			int n=init();
			if(n<0){
				perror("\nERROR: Failed to restart connection. Retrying.");
				cnt++;
			}
			else{
				break;
			}
		}
		if(cnt==5){
			perror("\nERROR: Could not reconnect. Shuting down program.");
			//IMPLEMENT SHUT DOWN SEQUENCE 

		}
	}
}

void Connect::stopSendThread(){
	for(int i=0;i<2;i++){
		if(sendThreadActive[i]){
			sendThreadActive[i]=false;
			if(sendThread[i].joinable())
				sendThread[i].join();
		}
	}	
}


void Connect::startReceiveThread(){
	if(receiveThreadActive==false){
		receiveThreadActive=true;
		//Spawn thread for telemetry
		receiveThread=std::thread(&Connect::receiveData,this);
	
		#ifdef DISPLAY_MESSAGES
		cout<<"\nSpawning thread to receive data";
		#endif
	}
}

void Connect::receiveData(){

	int n;
	char req;

	float *GSComm=new float[buffers.commLen];
	
	while(receiveThreadActive){
		n=read(sockfd[0],GSComm,sizeof(float)*buffers.commLen);
	
		if(n>0){
			buffers.GSComm.clear();
			buffers.GSComm.push(GSComm,buffers.commLen);
		}
		//checkStatus();
	}
	

}

void Connect::stopReceiveThread(){
	if(receiveThreadActive){
		receiveThreadActive=false;
		if(receiveThread.joinable())
			receiveThread.join();
	}	
}



