#include "tcp_client.h"

TCP_Client::TCP_Client(std::string target_ip, int target_port, Translater *translater_)
{
    send_tid = 0;
    recv_tid = 0;

    send_status = 0;      // whether the read thread is running
    recv_status  = 0;      // whether the recv thread is running
    time_to_exit   = false;  // flag to signal thread exit

    translater = translater_;

    // Start mutex
  	int result = pthread_mutex_init(&lock, NULL);
  	if ( result != 0 )
  	{
  		printf("\n mutex init failed\n");
  		throw 1;
  	}

    _target_ip = target_ip;
    _target_port = target_port;

    std::cout << "SERVER IP: " << _target_ip << std::endl;
    std::cout << "SERVER PORT: " << _target_port << std::endl;

    if(_target_port < 0 || _target_port > 65535)
        throw std::logic_error(std::string("Port outside range."));

    if(!connect_client())
    {
        std::cerr << strerror(errno) << std::endl;
        exit(-1);
    }

    std::cout << "CLIENT CONNECTED TO SERVER" << std::endl;

    connected = true;
    //reading_thread = new std::thread(&MavlinkTCP::read_loop, this);
}
//----------------------------------------------------------------------------//
TCP_Client::~TCP_Client()
{
    if(is_server)
    {
        close(csock);
    }
    close(sock);
}
//----------------------------------------------------------------------------//
bool TCP_Client::connect_client()
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&gcAddr, 0, sizeof(struct sockaddr_in));
    gcAddr.sin_family = AF_INET;
    gcAddr.sin_port = htons(_target_port);
    gcAddr.sin_addr.s_addr = inet_addr(_target_ip.c_str());

    return connect(sock, (struct sockaddr*) &gcAddr, sizeof(struct sockaddr)) == 0;
}
//----------------------------------------------------------------------------//
void TCP_Client::start()
{
  int result;

  printf("START CLIENT SEND THREAD \n");

  result = pthread_create( &send_tid, NULL, &start_tcp_client_send_thread, this );
  if ( result ) throw result;

  printf("\n");

  // --------------------------------------------------------------------------
	//   RECV THREAD
	// --------------------------------------------------------------------------
	printf("START RECV CLIENT THREAD \n");

  result = pthread_create( &recv_tid, NULL, &start_tcp_client_recv_thread, this );

  // now we're streaming setpoint commands
	printf("\n");


	// Done!
	return;
}
// ------------------------------------------------------------------------------
//   SHUTDOWN
// ------------------------------------------------------------------------------
void TCP_Client::stop()
{
	// --------------------------------------------------------------------------
	//   CLOSE THREADS
	// --------------------------------------------------------------------------
	printf("CLOSE THREADS\n");

	// signal exit
	time_to_exit = true;

	// wait for exit
	pthread_join(recv_tid ,NULL);
  pthread_join(send_tid ,NULL);

	// now the recv and send threads are closed
	printf("\n");

	// still need to close the serial_port separately
}

// ------------------------------------------------------------------------------
//   Quit Handler
// ------------------------------------------------------------------------------
void TCP_Client::handle_quit( int sig )
{

	try {
		stop();

	}
	catch (int error) {
		fprintf(stderr,"Warning, could not stop autopilot interface\n");
	}

}

//----------------------------------------------------------------------------//
void TCP_Client::send_server()
{
    size_t length = 256;
    uint8_t buffer[buffers.serial_recv_len*MAVLINK_MAX_PACKET_LEN];
    mavlink_status_t status;
    mavlink_message_t msg;
    uint16_t len;
    int n;
    ssize_t bytes_sent = 0;

    while( !time_to_exit )
    {
        int num_msg = buffers.serial_recv_buffer.size();

        if(!connected)
        {
            std::cout << "LOST CONNECTION" << std::endl;
            close(sock);
            reconnect();
        }

        if(recv_status > false)
        {
          usleep(100);
        }

        if(num_msg){

    			while(buffers.serial_recv_buffer.pop(msg) && num_msg > 0)
          {
    				pthread_mutex_lock(&lock);
            if(connected)
            {
              len = mavlink_msg_to_send_buffer(buffer, &msg);
              bytes_sent = sendto(sock, buffer, len, MSG_NOSIGNAL, (struct sockaddr*)&gcAddr, sizeof(struct sockaddr_in));
              connected = (bytes_sent != -1);
            }
            pthread_mutex_unlock(&lock);
    			}

    		}

    }
    return;

}
//----------------------------------------------------------------------------//
void TCP_Client::reconnect()
{
    if(is_server)
    {
        std::cout << "[TCP] Disconnected.\n[TCP] server listening for connection..."<< std::endl;
        //connect_server();
    }
    else
    {
        std::cout << "[TCP] Disconnected."<< std::endl;
        while(!connect_client())
        {
            std::cout << "[TCP] Connection retry..."<< std::endl;
            usleep(1000000);
        }
    }
    connected = true;
    std::cout << "[TCP] Connected."<< std::endl;
}

// ------------------------------------------------------------------------------
//   Read Thread
// ------------------------------------------------------------------------------
void TCP_Client::send_thread()
{
	send_status = true;

	while ( ! time_to_exit )
	{
		send_server();
		usleep(100000); // Read batches at 10Hz
	}

	send_status = false;

	return;
}

// ------------------------------------------------------------------------------
//   Read Thread
// ------------------------------------------------------------------------------
void TCP_Client::start_send_thread()
{

	if ( send_status != 0 )
	{
		fprintf(stderr,"read thread already running\n");
		return;
	}
	else
	{
		send_thread();
		return;
	}

}

// ------------------------------------------------------------------------------
//   Receive Client Messages
// ------------------------------------------------------------------------------
void TCP_Client::recv_server()
{
    size_t length = 256;
    uint8_t buffer[length];
    mavlink_status_t status;
    mavlink_message_t msg;
    Mavlink_Messages current_messages;
    std::string input_curr, input_prev;

    input_prev = "";
    int x = 0;

    while( !time_to_exit )
    {
        socklen_t fromlen = sizeof(struct sockaddr);

        // Lock
        //pthread_mutex_lock(&lock);
        ssize_t nb_read = recvfrom(sock, (void *)buffer, length, 0, (struct sockaddr *)&gcAddr, &fromlen);

        // Unlock
      	//pthread_mutex_unlock(&lock);
        /*
        if(nb_read == -1)
            throw std::logic_error("Unable to read from TCP socket.");
        */
        // give the write thread time to use the port
        /*
        if ( send_status > false ) {
    			usleep(100); // look for components of batches at 10kHz
    		}
        */

        for(int i=0;i<nb_read;i++)
        {
          if(mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &msg, &status))
          {
            //buffers.serial_send_buffer.push(msg);
            //translater->write_message(msg);
            switch(msg.msgid)
            {
              /*
              // Auto Pilot Modes enumeration
              enum control_mode_t {
                STABILIZE =     0,  // manual airframe angle with manual throttle
                ACRO =          1,  // manual body-frame angular rate with manual throttle
                ALT_HOLD =      2,  // manual airframe angle with automatic throttle
                AUTO =          3,  // fully automatic waypoint control using mission commands
                GUIDED =        4,  // fully automatic fly to coordinate or fly at velocity/direction using GCS immediate commands
                LOITER =        5,  // automatic horizontal acceleration with automatic throttle
                RTL =           6,  // automatic return to launching point
                CIRCLE =        7,  // automatic circular flight with automatic throttle
                LAND =          9,  // automatic landing with horizontal position control
                DRIFT =        11,  // semi-automous position, yaw and throttle control
                SPORT =        13,  // manual earth-frame angular rate control with manual throttle
                FLIP =         14,  // automatically flip the vehicle on the roll axis
                AUTOTUNE =     15,  // automatically tune the vehicle's roll and pitch gains
                POSHOLD =      16,  // automatic position hold with manual override, with automatic throttle
                BRAKE =        17,  // full-brake using inertial/GPS system, no pilot input
                THROW =        18,  // throw to launch mode using inertial/GPS system, no pilot input
                AVOID_ADSB =   19,  // automatic avoidance of obstacles in the macro scale - e.g. full-sized aircraft
                GUIDED_NOGPS = 20,  // guided mode but only accepts attitude and altitude
                SMART_RTL =    21,  // SMART_RTL returns to home by retracing its steps
                FLOWHOLD  =    22,  // FLOWHOLD holds position with optical flow without rangefinder
                FOLLOW    =    23,  // follow attempts to follow another vehicle or ground station
                ZIGZAG    =    24,  // ZIGZAG mode is able to fly in a zigzag manner with predefined point A and point B
              };
              */
                // case MAVLINK_MSG_ID_HEARTBEAT:
                // {
                //     mavlink_heartbeat_t heartbeat;
                //     mavlink_msg_heartbeat_decode(&msg, &heartbeat);
                //     std::cout << "heartbeat" << std::endl;
                //
                //     // access message specific fields
                //     // std::cout << "    type:            "            << (uint)heartbeat.type << std::endl;
                //     // std::cout << "    autopilot:       "       << (uint)heartbeat.autopilot << std::endl;
                //     // std::cout << "    base_mode:       "       << (uint)heartbeat.base_mode << std::endl;
                //     // std::cout << "    custom_mode:     "     << (uint)heartbeat.custom_mode << std::endl;
                //     // std::cout << "    system_status:   "   << (uint)heartbeat.system_status << std::endl;
                //     // std::cout << "    mavlink_version: " << (uint)heartbeat.mavlink_version << std::endl;
                //     break;
                // }
                // case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
                // {
                //     mavlink_param_request_list_t param_request_list;
                //     mavlink_msg_param_request_list_decode(&msg, &param_request_list);
                //     std::cout << "param request list" << std::endl;
                //     break;
                // }

                case MAVLINK_MSG_ID_COMMAND_LONG:
                {
                  std::cout << MAVLINK_MSG_ID_COMMAND_LONG << std::endl;
                  mavlink_command_long_t command_long;
                  mavlink_msg_command_long_decode(&msg, &command_long);
                  if (command_long.command == 115)
                  {
                    std::cout << "YAW DESIRED: " << command_long.param1 << std::endl;
                  }
                  else if (command_long.command == 92)
                  {
                    std::cout << "ENABLE OFFBOARD: " << command_long.param1 << std::endl;
                  }
                  else if (command_long.command == 400)
                  {
                    std::cout << "ARM PIXHAWK: " << command_long.param1 << std::endl;
                  }
                  else if (command_long.command == 214)
                  {
                    std::cout << "DESIRED STATE: " << command_long.param1 << std::endl;
                  }
                  break;
                }

                case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
                {
                   mavlink_local_position_ned_t local_position_ned;
                   mavlink_msg_local_position_ned_decode(&msg, &local_position_ned);
                   std::cout << "X DESIRED: " << local_position_ned.x << std::endl;
                   std::cout << "Y DESIRED: " << local_position_ned.y << std::endl;
                   std::cout << "Z DESIRED: " << local_position_ned.z << std::endl;
                   break;
                }

                case MAVLINK_MSG_ID_SCALED_PRESSURE:
                {
                  mavlink_scaled_pressure_t scaled_pressure;
                  std::cout << scaled_pressure.press_abs << " " << scaled_pressure.press_diff << " " << scaled_pressure.temperature << std::endl;
                  break;
                }

                case MAVLINK_MSG_ID_RAW_PRESSURE:
                {
                  mavlink_raw_pressure_t raw_pressure;
                  std::cout << raw_pressure.press_abs << " " << raw_pressure.press_diff1 << " " << raw_pressure.temperature << std::endl;
                  break;
                }
                /*
                case MAVLINK_MSG_ID_COMMAND_LONG: //MAV_CMD_NAV_GUIDED_ENABLE:
                {
                  mavlink_command_long_t command_long;
                  mavlink_msg_command_long_decode(&msg, &command_long);
                  std::cout << "OFFBOARD: " << command_long.command << std::endl;
                  break;
                }
                */
                // case MAVLINK_MSG_ID_MISSION_SET_CURRENT:
                // {
                //     mavlink_mission_set_current_t mission_set_current;
                //     mavlink_msg_mission_set_current_decode(&msg, &mission_set_current);
                //     std::cout << "mission set current" << std::endl;
                //     break;
                // }
                // case MAVLINK_MSG_ID_REQUEST_DATA_STREAM:
                // {
                //     mavlink_request_data_stream_t request_data_stream;
                //     mavlink_msg_request_data_stream_decode(&msg, &request_data_stream);
                //     std::cout << "request data stream" << std::endl;
                //     break;
                // }
                // case MAVLINK_MSG_ID_NAMED_VALUE_FLOAT:
                // {
                //     mavlink_named_value_float_t named_value_float;
                //     mavlink_msg_named_value_float_decode(&msg, &named_value_float);
                //     std::cout << "named value float" << std::endl;
                //     break;
                // }
                // case MAVLINK_MSG_ID_STATUSTEXT:
                // {
                //     mavlink_statustext_t statustext;
                //     mavlink_msg_statustext_decode(&msg, &statustext);
                //     std::cout << "status text" << std::endl;
                //     std::cout << statustext.text << std::endl;
                //     break;
                // }
                // Send the message
              	//int len = serial_port->write_message(msg);
                default:
                {
                    // std::cout << "Unsupported packet -> ";
                    /*
                    std::cout << "SYS: "     << (int)msg.sysid;
                    std::cout << ", COMP: "   << (int)msg.compid;
                    std::cout << ", SEQ: "    << (int)msg.seq;
                    std::cout << ", LEN: "    << (int)msg.len;
                    std::cout << ", MSG ID: " << (int)msg.msgid << std::endl;
                    */
                    break;
                }
            }
            std::cout << "----------------------" << std::endl;
          }
        }

        /*
        input_curr = (char *)buffer;
        std::stringstream input_int(input_curr.substr(0,2));
        input_int >> x;

        if (input_curr != input_prev)
        {
          std::cout << "Input: " << input_curr.substr(0,2) << std::endl;
        }

        input_prev = input_curr;
        */
    }
}

// ------------------------------------------------------------------------------
//   Socket Thread
// ------------------------------------------------------------------------------
void TCP_Client::recv_thread(void)
{

	recv_status = true;

	// Pixhawk needs to see off-board commands at minimum 2Hz,
	// otherwise it will go into fail safe
	while ( !time_to_exit )
	{
		usleep(250000);   // Stream at 4Hz
		recv_server();
	}

	// signal end
	recv_status = false;

	return;

}

// ------------------------------------------------------------------------------
//   Recv Thread
// ------------------------------------------------------------------------------
void TCP_Client::start_recv_thread(void)
{
	if ( not recv_status == false )
	{
		fprintf(stderr,"recv thread already running\n");
		return;
	}

	else
	{
		recv_thread();
		return;
	}

}

// ------------------------------------------------------------------------------
//  Pthread Starter Helper Functions
// ------------------------------------------------------------------------------

void* start_tcp_client_send_thread(void *args)
{
	// takes an autopilot object argument
	TCP_Client *tcp_client = (TCP_Client *)args;

	// run the object's read thread
	tcp_client->start_send_thread();

	// done!
	return NULL;
}

void* start_tcp_client_recv_thread(void *args)
{
	// takes an autopilot object argument
	TCP_Client *tcp_client = (TCP_Client *)args;

	// run the object's read thread
	tcp_client->start_recv_thread();

	// done!
	return NULL;
}
