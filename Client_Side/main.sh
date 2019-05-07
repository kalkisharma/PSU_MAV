#!/bin/bash
#g++ main.cpp Serial_Connection/serial_port.cpp -o main.exe -lpthread

#g++ main.cpp Serial_Connection/serial_port.cpp TCP_Server/tcp_server.cpp -o main.exe -lpthread

#g++ main.cpp Serial_Connection/serial_port.cpp Autopilot_Interface/autopilot_interface.cpp Buffer/buffer.cpp -o main.exe -lpthread

#g++ main.cpp Serial_Connection/serial_port.cpp Translater/translater.cpp Struct/struct.cpp -o main.exe -lpthread

#g++ main.cpp Serial_Connection/serial_port.cpp Struct/struct.cpp -o main.exe -lpthread

#g++ main.cpp Serial_Connection/serial_port.cpp TCP_Server/tcp_server.cpp Autopilot_Interface/autopilot_interface.cpp -o main.exe -lpthread

g++ main.cpp Serial_Connection/serial_port.cpp Translater/translater.cpp Struct/struct.cpp TCP_Client/tcp_client.cpp -o main.exe -lpthread
