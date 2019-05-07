#include <iostream>

#include "serial_port.h"

using namespace std;

int main(){

  Serial_Port serial_port("/dev/ttyACM0", 57600);
  serial_port.start();

  serial_port.stop();

  return 0;
}
