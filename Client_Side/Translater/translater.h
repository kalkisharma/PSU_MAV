#ifndef TRANSLATER_H_
#define TRANSLATER_H_

#include <pthread.h> // This uses POSIX Threads

#include "../Struct/struct.h"
#include "../Serial_Connection/serial_port.h"

// ------------------------------------------------------------------------------
//   Global Buffer
// ------------------------------------------------------------------------------

extern Buffers buffers;

// ------------------------------------------------------------------------------
//   Prototypes
// ------------------------------------------------------------------------------


// helper functions
void* start_translater_read_thread(void *args);
void* start_translater_write_thread(void *args);

// ----------------------------------------------------------------------------------
//   Serial Port Communcation Link Class
// ----------------------------------------------------------------------------------
class Translater
{

public:

	Translater();
	Translater(Serial_Port *serial_port_);
	~Translater();

	char reading_status;
	char writing_status;
	char control_status;
  uint64_t write_count;

  int system_id;
	int autopilot_id;
	int companion_id;

	Mavlink_Messages current_messages;
	mavlink_set_position_target_local_ned_t initial_position;

	void update_setpoint(mavlink_set_position_target_local_ned_t setpoint);
	void read_messages();
	int  write_message(mavlink_message_t message);

	void enable_offboard_control();
	void disable_offboard_control();

	void start();
	void stop();

	void start_read_thread();
	void start_write_thread(void);

	void handle_quit( int sig );


private:

	Serial_Port *serial_port;

	bool time_to_exit;

	pthread_t read_tid;
	pthread_t write_tid;

	mavlink_set_position_target_local_ned_t current_setpoint;

	void read_thread();
	void write_thread(void);

	int toggle_offboard_control( bool flag );
	void write_setpoint();

};



#endif // TRANSLATER_H_
