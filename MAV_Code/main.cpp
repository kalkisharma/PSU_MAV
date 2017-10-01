#include <sys/time.h>
#include "connect.h"
#include "pixhawk.h"
#include <iostream>
#include "struct.h"
#include "autopilot.h"
#include "image.h"
#include <string>
#include <fstream>

Buffers buffers(10,8,2);

int main(){
	
	//std::ofstream out("out.txt");
	//std::streambuf *coutbuf=std::cout.rdbuf();
	//std::cout.rdbuf(out.rdbuf());
	
    /*
     * Initialize object of class Connect.
     * Arguments are the IP address and the port number of the server.
	 * Object calls member function init of class Connect.
     */
    Connect phoenix("10.20.106.66",27013);
	phoenix.init();
    
	
	std::vector<std::string> usb;
	usb.push_back("/dev/ttyACM0");
	usb.push_back("/dev/ttyACM1");

	Pixhawk px4Mini(57600,usb);
	px4Mini.init();

	Image image(320,240);
	image.startProcessing();

	Action action(px4Mini, image);
	action.start();

	while(1){
	}
}
