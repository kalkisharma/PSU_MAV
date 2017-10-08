
#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>
#include <queue>
#include <mutex>
#include <iostream>

#include "/home/pi/Documents/mavlink_v2/common/mavlink.h"

#include "opencv2/core/core.hpp"

struct AAA{
	float dist;
	int dir;
	float x;
	float y;
		
	AAA(){
		dist=-1;
		dir=-1;
		x=-1;
		y=-1;
	}
		
	void reset(){
		dist=-1;
		dir=-1;
		x=-1;
		y=-1;
	}
};

struct ImgData{
	AAA boundary;
	AAA homeBase;
	AAA target;
	AAA pickup;
};

//Messages we care about
struct TimeStamps{
	uint64_t heartbeat;
	uint64_t sysStatus;
	uint64_t batteryStatus;
	uint64_t radioStatus;
	uint64_t localPos;
	uint64_t globalPos;
	uint64_t localPosTarget;
	uint64_t globalPosTarget;
	uint64_t imuData;
	uint64_t attitude;
	uint64_t attitudeTarget;
	uint64_t opticalFlow;

	void reset();
	bool isAllNonZero();
	TimeStamps();

};

struct Mavlink_Messages{
	int sysID;
	int compID;

	mavlink_heartbeat_t heartbeat;
	mavlink_sys_status_t sysStatus;
	mavlink_battery_status_t batteryStatus;
	mavlink_radio_status_t radioStatus;
	mavlink_local_position_ned_t localPos;
	mavlink_global_position_int_t globalPos;
	mavlink_position_target_local_ned_t localPosTarget;
	mavlink_position_target_global_int_t globalPosTarget;//remove doesn't work
	mavlink_highres_imu_t imuData;
	mavlink_attitude_t attitude;
	mavlink_attitude_target_t attitudeTarget;
	mavlink_optical_flow_rad_t opticalFlow;

	TimeStamps timeStamps;

	Mavlink_Messages(){sysID=-1;compID=-1;}
};


template <class T>
class CircularBuffer{
	std::vector <T> data;
	int maxLen;
	unsigned int n;
	int f,b;
	std::mutex mu;

	void cntInc(int &i){
		i++;
		if(i==maxLen)
			i=0;
	}

	void cntDec(int &i){
		i--;
		if(i<0)
			i=maxLen;
	}

public:
	CircularBuffer(int l){
		if(l>1)
			maxLen=l;
		else
			maxLen=0;
		data.resize(maxLen);
		n=0;f=0;b=0;
	}

	CircularBuffer(){
		maxLen=1;
		data.resize(maxLen);
		n=0;f=0;b=0;
	}

	void display(){
		std::cout<<"\nSize: "<<size()<<", "<<b<<","<<f<<"\n";
		for(int i=b,cnt=0;cnt<n;cntInc(i),cnt++){
			std::cout<<data[i]<<" ";
		}
	}
	bool push(T ele){
		mu.lock();

		if(n==0){
			data[f]=ele;
			cntInc(f);
			n++;
			mu.unlock();
			return 1;
		}

		else if(f!=b){
			data[f]=ele;
			cntInc(f);
			n++;
			mu.unlock();
			return 1;
		}
		else{
			cntInc(b);
			data[f]=ele;
			cntInc(f);
			mu.unlock();
			return 0;
		}

	}

	bool push(T *ele,int l){
		bool safe=true;
		for(int i=0;i<l;i++){
			if(!push(ele[i])){
				safe=false;
			}
		}
		return safe;
	}

	bool pop(T &ele){
		if(n){
			mu.lock();
			ele=data[b];
			if(n>1){
				cntInc(b);
			}
			n--;
			if(n==0){
				f=b;
			}
			mu.unlock();
			return 1;
		}
		return 0;
	}

	bool copy(T &ele){
		if(n){
			mu.lock();
			ele=data[b];
			mu.unlock();
			return 1;
		}
		return 0;
	}

	bool pop(T *ele,int l){
		bool safe=true;
		for(int i=0;i<l;i++){
			if(!pop(ele[i])){
				safe=false;
				break;
			}
		}
		return safe;
	}

	size_t size(){
		return n;
	}

	void clear(){
		data.clear();
		n=0;f=0;b=0;
	}
};


struct Buffers{
	CircularBuffer<mavlink_message_t> telemetry;
	CircularBuffer<float> GSComm;
 	Mavlink_Messages messages;
	CircularBuffer <cv::Mat> frames; //buffer to store camera pics
	CircularBuffer <cv::Mat> sendFrame; //picture to be sent to GS

	int telemLen;
	int commLen;
	int frameLen;
	
	Buffers(int m, int n, int o):telemetry(m),GSComm(n),frames(o), sendFrame(2){
		telemLen=m;
		commLen=n;
		frameLen=o;
	}
};


#endif