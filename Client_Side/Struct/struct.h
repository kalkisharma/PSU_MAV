#ifndef STRUCT_H
#define STRUCT_H

#include <iostream>
#include <vector>
#include <sys/time.h>
#include <pthread.h>

#include "/home/kalki/Documents/Python_Projects/MAV/PSU_MAV/VFS_2019/c_library_v2/ardupilotmega/mavlink.h"

// ------------------------------------------------------------------------------
//   Defines
// ------------------------------------------------------------------------------

/**
 * Defines for mavlink_set_position_target_local_ned_t.type_mask
 *
 * Bitmask to indicate which dimensions should be ignored by the vehicle
 *
 * a value of 0b0000000000000000 or 0b0000001000000000 indicates that none of
 * the setpoint dimensions should be ignored.
 *
 * If bit 10 is set the floats afx afy afz should be interpreted as force
 * instead of acceleration.
 *
 * Mapping:
 * bit 1: x,
 * bit 2: y,
 * bit 3: z,
 * bit 4: vx,
 * bit 5: vy,
 * bit 6: vz,
 * bit 7: ax,
 * bit 8: ay,
 * bit 9: az,
 * bit 10: is force setpoint,
 * bit 11: yaw,
 * bit 12: yaw rate
 * remaining bits unused
 *
 * Combine bitmasks with bitwise &
 *
 * Example for position and yaw angle:
 * uint16_t type_mask =
 *     MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_POSITION &
 *     MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_YAW_ANGLE;
 */

                                                // bit number  876543210987654321
#define MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_POSITION     0b0000110111111000
#define MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_VELOCITY     0b0000110111000111
#define MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_ACCELERATION 0b0000110000111111
#define MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_FORCE        0b0000111000111111
#define MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_YAW_ANGLE    0b0000100111111111
#define MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_YAW_RATE     0b0000010111111111

// ------------------------------------------------------------------------------
//   Helper Functions
// ------------------------------------------------------------------------------
uint64_t get_time_usec();
void set_position(float x, float y, float z, mavlink_set_position_target_local_ned_t &sp);
void set_velocity(float vx, float vy, float vz, mavlink_set_position_target_local_ned_t &sp);
void set_acceleration(float ax, float ay, float az, mavlink_set_position_target_local_ned_t &sp);
void set_yaw(float yaw, mavlink_set_position_target_local_ned_t &sp);
void set_yaw_rate(float yaw_rate, mavlink_set_position_target_local_ned_t &sp);

// ------------------------------------------------------------------------------
//   Data Structures
// ------------------------------------------------------------------------------

struct Time_Stamps
{
	Time_Stamps()
	{
		reset_timestamps();
	}

	uint64_t heartbeat;
	uint64_t sys_status;
	uint64_t battery_status;
	uint64_t radio_status;
	uint64_t local_position_ned;
	uint64_t global_position_int;
	uint64_t position_target_local_ned;
	uint64_t position_target_global_int;
	uint64_t highres_imu;
	uint64_t attitude;

	void
	reset_timestamps()
	{
		heartbeat = 0;
		sys_status = 0;
		battery_status = 0;
		radio_status = 0;
		local_position_ned = 0;
		global_position_int = 0;
		position_target_local_ned = 0;
		position_target_global_int = 0;
		highres_imu = 0;
		attitude = 0;
	}

};


// Struct containing information on the MAV we are currently connected to

struct Mavlink_Messages {

	int sysid;
	int compid;

	// Heartbeat
	mavlink_heartbeat_t heartbeat;

	// System Status
	mavlink_sys_status_t sys_status;

	// Battery Status
	mavlink_battery_status_t battery_status;

	// Radio Status
	mavlink_radio_status_t radio_status;

	// Local Position
	mavlink_local_position_ned_t local_position_ned;

	// Global Position
	mavlink_global_position_int_t global_position_int;

	// Local Position Target
	mavlink_position_target_local_ned_t position_target_local_ned;

	// Global Position Target
	mavlink_position_target_global_int_t position_target_global_int;

	// HiRes IMU
	mavlink_highres_imu_t highres_imu;

	// Attitude
	mavlink_attitude_t attitude;

	mavlink_command_long_t command_long;
	// System Parameters?


	// Time Stamps
	Time_Stamps time_stamps;

	void
	reset_timestamps()
	{
		time_stamps.reset_timestamps();
	}

};

// ------------------------------------------------------------------------------
//   Circular Buffer Class
// ------------------------------------------------------------------------------

template <class T>
class CircularBuffer
{
	std::vector <T> data;
	int max_len;
	unsigned int n;
	int f,b;
	pthread_mutex_t  lock;

	void cntInc(int &i)
	{
		i++;
		if(i == max_len)
			i = 0;
	}

	void cntDec(int &i)
	{
		i--;
		if(i < 0)
			i = max_len;
	}
public:
	CircularBuffer(int l)
	{
		if(l > 1)
			max_len = l;
		else
			max_len = 0;
		data.resize(max_len);
		n=0; f=0; b=0;
	}

	CircularBuffer()
	{
		max_len = 1;
		data.resize(max_len);
		n=0; f=0; b=0;
	}

	void display()
	{
		std::cout << "\nSize: "<< size() << ", " << b << "," << f << "\n";
		for(int i = b, cnt = 0; cnt < n; cntInc(i), cnt++)
		{
			std::cout << data[i] << " ";
		}
	}

	bool push(T ele)
	{
		// Lock
		pthread_mutex_lock(&lock);

		if(n==0)
		{
			data[f]=ele;
			cntInc(f);
			n++;
			// Unlock
			pthread_mutex_unlock(&lock);
			return 1;
		}
		else if(f!=b)
		{
			data[f]=ele;
			cntInc(f);
			n++;
			// Unlock
			pthread_mutex_unlock(&lock);
			return 1;
		}
		else
		{
			cntInc(b);
			data[f]=ele;
			cntInc(f);
			// Unlock
			pthread_mutex_unlock(&lock);
			return 0;
		}
	}

	bool push(T *ele, int l)
	{
		bool safe=true;
		for(int i = 0; i < l; i++)
		{
			if(!push(ele[i]))
			{
				safe=false;
			}
		}
		return safe;
	}

	bool pop(T &ele)
	{
		if(n)
		{
			// Lock
			pthread_mutex_lock(&lock);
			ele=data[b];
			if(n>1)
			{
				cntInc(b);
			}
			n--;
			if(n==0)
			{
				f=b;
			}
			// Unlock
			pthread_mutex_unlock(&lock);
			return 1;
		}
		return 0;
	}

	bool pop(T *ele,int l)
	{
		bool safe=true;
		for(int i = 0; i < l; i++)
		{
			if(!pop(ele[i]))
			{
				safe=false;
				break;
			}
		}
		return safe;
	}

	bool copy(T &ele)
	{
		if(n){
			// Lock
			pthread_mutex_lock(&lock);
			ele=data[b];
			// Unlock
			pthread_mutex_unlock(&lock);
			return 1;
		}
		return 0;
	}

	size_t size()
	{
		return n;
	}

	void clear()
	{
		data.clear();
		n=0;f=0;b=0;
	}
};

struct Buffers{
	CircularBuffer<mavlink_message_t> serial_send_buffer;
	CircularBuffer<mavlink_message_t> serial_recv_buffer;
	CircularBuffer<mavlink_message_t> client_send_buffer;
	CircularBuffer<mavlink_message_t> client_recv_buffer;
	CircularBuffer<mavlink_message_t> serial_msg_buffer;
	CircularBuffer<mavlink_message_t> client_msg_buffer;

	int serial_msg_len;
	int client_msg_len;
	int serial_send_len;
	int serial_recv_len;
	int client_send_len;
	int client_recv_len;

	Buffers(int s, int c, int ss, int sr, int cs, int cr):serial_msg_buffer(s), client_msg_buffer(c), serial_send_buffer(ss), serial_recv_buffer(sr), client_send_buffer(cs), client_recv_buffer(cr)
	{
		serial_msg_len = s;
		client_msg_len = c;
		serial_send_len = ss;
		serial_recv_len = sr;
		client_send_len = cs;
		client_recv_len = cr;
	}
};

#endif
