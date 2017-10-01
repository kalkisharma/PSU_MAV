#ifndef IMAGE_H
#define IMAGE_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <stdio.h>
#include <iostream>
    
#include <vector> 
#include <numeric>

#include <raspicam/raspicam_cv.h>

#include <stdint.h>

#include <sys/time.h>

#include <math.h>
#include <thread>

#include <unistd.h>

#include "struct.h"


using namespace cv;
using namespace std;

extern Buffers buffers;

//------------------------------------------------------------------------------
 

class Image{
	
	raspicam::RaspiCam_Cv cap;
	
	float theta;        // Camera half-angle in x-direction (640 pixels)
	float phi ;          // Camera half-angle in y-direction (480 pixels)
	int width;      // Maximum x-axis grid location
	int height;      // Maximum y-axis grid location
	
	ImgData imgData;

	std::thread capThread,boundThread,mainThread;
	bool capThreadActive,boundThreadActive,mainThreadActive;
	
	bool getBoundary,getHomeBase,getTarget,getPickup;

	uint64_t getTimeUsec(){
		struct timespec t;
		clock_gettime(CLOCK_REALTIME,&t);
		return (t.tv_sec)*1e6+(t.tv_nsec)/1e3;
	}

public:

	Image(int width, int height);

	void startProcessing();
	void stopProcessing();
	void startCapture();
	void stopCapture();
	void capture();
	void process();
	cv::Mat detectBoundary(cv::Mat, float, int &, float &);
	void calibrate(Scalar &u, Scalar &l);
	cv::Mat detectHomeBase(cv::Mat imgOriginal, float altitude, int &direction, float &x, float &y);
	cv::Mat detectTarget(cv::Mat imgOriginal, float altitude, int &direction, float &x, float &y);
	void setType(bool a, bool b, bool c, bool d);

	ImgData getImgData(){
		return imgData;
	}

	~Image(){
		stopProcessing();
		stopCapture();
	}
};

#endif
