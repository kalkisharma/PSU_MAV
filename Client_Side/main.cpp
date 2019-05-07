#include "main.h"

Buffers buffers(1024, 1024, 1024, 1024, 1024, 1024);

// ------------------------------------------------------------------------------
//   Quit Signal Handler
// ------------------------------------------------------------------------------
// this function is called when you press Ctrl-C
void quit_handler( int sig )
{
	printf("\n");
	printf("TERMINATING AT USER REQUEST\n");
	printf("\n");

	// serial port
	try {
		serial_port_quit->handle_quit(sig);
	}
	catch (int error){}

	// serial trasmission
	try {
		translater_quit->handle_quit(sig);
	}
	catch (int error){}

	// tcp client
	try {
		tcp_client_quit->handle_quit(sig);
	}
	catch (int error){}

	// end program here
	exit(0);

}

// ------------------------------------------------------------------------------
//   COMMANDS
// ------------------------------------------------------------------------------

void
commands(Translater &translater)
{

	// --------------------------------------------------------------------------
	//   START OFFBOARD MODE
	// --------------------------------------------------------------------------
	sleep(5);
	translater.enable_offboard_control();
	usleep(100); // give some time to let it sink in

	// now the autopilot is accepting setpoint commands


	// --------------------------------------------------------------------------
	//   SEND OFFBOARD COMMANDS
	// --------------------------------------------------------------------------
	printf("SEND OFFBOARD COMMANDS\n");

	// initialize command data strtuctures
	mavlink_set_position_target_local_ned_t sp;
	mavlink_set_position_target_local_ned_t ip = translater.initial_position;

	// autopilot_interface.h provides some helper functions to build the command


	// Example 1 - Set Velocity
//	set_velocity( -1.0       , // [m/s]
//				  -1.0       , // [m/s]
//				   0.0       , // [m/s]
//				   sp        );

	// Example 2 - Set Position
	 set_position( ip.x - 5.0 , // [m]
			 	   ip.y - 5.0 , // [m]
				   ip.z       , // [m]
				   sp         );


	// Example 1.2 - Append Yaw Command
	set_yaw( ip.yaw , // [rad]
			 sp     );

	// SEND THE COMMAND
	translater.update_setpoint(sp);
	// NOW pixhawk will try to move

	// Wait for 8 seconds, check position
	for (int i=0; i < 8; i++)
	{
		mavlink_local_position_ned_t pos = translater.current_messages.local_position_ned;
		printf("%i CURRENT POSITION XYZ = [ % .4f , % .4f , % .4f ] \n", i, pos.x, pos.y, pos.z);
		sleep(1);
	}

	printf("\n");


	// --------------------------------------------------------------------------
	//   STOP OFFBOARD MODE
	// --------------------------------------------------------------------------

	translater.disable_offboard_control();

	// now pixhawk isn't listening to setpoint commands


	// --------------------------------------------------------------------------
	//   GET A MESSAGE
	// --------------------------------------------------------------------------
	printf("READ SOME MESSAGES \n");

	// copy current messages
	Mavlink_Messages messages = translater.current_messages;

	// local position in ned frame
	mavlink_local_position_ned_t pos = messages.local_position_ned;
	printf("Got message LOCAL_POSITION_NED (spec: https://mavlink.io/en/messages/common.html#LOCAL_POSITION_NED)\n");
	printf("    pos  (NED):  %f %f %f (m)\n", pos.x, pos.y, pos.z );

	// hires imu
	mavlink_highres_imu_t imu = messages.highres_imu;
	printf("Got message HIGHRES_IMU (spec: https://mavlink.io/en/messages/common.html#HIGHRES_IMU)\n");
	printf("    ap time:     %lu \n", imu.time_usec);
	printf("    acc  (NED):  % f % f % f (m/s^2)\n", imu.xacc , imu.yacc , imu.zacc );
	printf("    gyro (NED):  % f % f % f (rad/s)\n", imu.xgyro, imu.ygyro, imu.zgyro);
	printf("    mag  (NED):  % f % f % f (Ga)\n"   , imu.xmag , imu.ymag , imu.zmag );
	printf("    baro:        %f (mBar) \n"  , imu.abs_pressure);
	printf("    altitude:    %f (m) \n"     , imu.pressure_alt);
	printf("    temperature: %f C \n"       , imu.temperature );

	printf("\n");


	// --------------------------------------------------------------------------
	//   END OF COMMANDS
	// --------------------------------------------------------------------------

	return;

}

int main()
{
  Serial_Port serial_port("/dev/ttyACM0", 57600);
	Translater translater(&serial_port);
	TCP_Client tcp_client("10.20.101.124", 9999, &translater);
	//TCP_Client tcp_client("10.20.101.53", 8888, &translater);
	//Autopilot_Interface autopilot_interface(&serial_port);

  serial_port_quit         = &serial_port;
	translater_quit					 = &translater;
	tcp_client_quit          = &tcp_client;
	//autopilot_interface_quit = &autopilot_interface;
	signal(SIGINT, quit_handler);

	serial_port.start();
	translater.start();
	tcp_client.start();

	//autopilot_interface.start();
  while(1)
  {

  }
  return 0;

	// --------------------------------------------------------------------------
	//   RUN COMMANDS
	// --------------------------------------------------------------------------

	/*
	 * Now we can implement the algorithm we want on top of the autopilot interface
	 */
	commands(translater);


	// --------------------------------------------------------------------------
	//   THREAD and PORT SHUTDOWN
	// --------------------------------------------------------------------------

	/*
	 * Now that we are done we can stop the threads and close the port
	 */
	tcp_client.stop();
	translater.stop();
	serial_port.stop();


	// --------------------------------------------------------------------------
	//   DONE
	// --------------------------------------------------------------------------

	// woot!
	return 0;

	/*
	//autopilot_interface.start();
  while(1)
  {

  }
  return 0;
	*/
}
