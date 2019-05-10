from pymavlink import mavutil
from enum import Enum
import threading
import logging
import time
import numpy as np
class MAVServer:

    def __init__(self, ip_='localhost', port_=9999):

        print(mavutil.mavlink)
        self.ip = ip_
        self.port = port_
        self.server = None
        self.mavlink_cmd = 0
        self.msg_id_recv = 0
        self.msg_payload_recv = [0]*20
        self.msg_per_second = 4

        self.mav_cmd_id = 0
        self.param1 = 0
        self.param2 = 0
        self.param3 = 0
        self.param4 = 0
        self.param5 = 0
        self.param6 = 0
        self.param7 = 0

        self.send_flag = False
        self.recv_flag = False
        self.close_threads = False
        self.lock = threading.Lock()

        self.recv_thread = None

        self.x_desired = 0 # X
        self.y_desired = 0 # Y
        self.z_desired = 0 # Z
        self.yaw_desired = 0 # YAW

        self.x_current = 0
        self.y_current = 0
        self.z_current = 0

        self.msg_type_dict = [
            'ACTUATOR_CONTROL_TARGET',
            'LOCAL_POSITION_NED',
            'PING',
            'ATTITUDE',
            'ATTITUDE_QUATERNION',
            'HIGHRES_IMU',
            'SERVO_OUTPUT_RAW',
            'VFR_HUD',
            'ESTIMATOR_STATUS',
            'VIBRATION',
            'ALTITUDE',
            'GLOBAL_POSITION_INT',
            'TIMESYNC',
            'GPS_RAW_INT',
            'ATTITUDE_TARGET',
            'EXTENDED_SYS_STATE',
            'SYS_STATUS',
            'BATTERY_STATUS',
            'SYSTEM_TIME',
            'HEARTBEAT'
         ]

    def conn_server(self):

        logging.info("SERVER IP -> {0}, SERVER PORT -> {1}".format(self.ip, self.port))
        server = mavutil.mavlink_connection('tcpin:{0}:{1}'.format(self.ip, self.port), planner_format=False,
                                            notimestamps=True, robust_parsing=True)
        logging.info("SERVER CREATED")
        self.server = server

        return

    def wait_for_heartbeat(self):

        # Wait for a heartbeat so we know the target system IDs
        logging.info("WAITING FOR APM HEARTBEAT")
        try:
            msg = self.server.recv_match(type='HEARTBEAT', blocking=True)
            logging.info(f"HEARTBEAT FROM APM ({self.server.target_system}, {self.server.target_system})")
        except KeyboardInterrupt:
            logging.info("USER EXIT WITHOUT FINDING HEARTBEAT")

        return

    def run_server(self, msg_payload_recv, msg_payload_send, desired_state):

        self.wait_for_heartbeat()
        self.start_recv_thread()
        self.send_thread(msg_payload_recv, msg_payload_send, desired_state)

    def start_recv_thread(self):

        logging.info("SPAWNING RECV THREAD")

        self.recv_flag = True
        self.recv_thread = threading.Thread(target=self.recv_msg)
        self.recv_thread.start()

        return


    def close_server(self):

        self.close_threads = True


    def close_recv_thread(self):

        self.recv_thread.join()

    def send_thread(self, msg_payload_recv, msg_payload_send, desired_state):

        while True:

            # SETPOINTS
            # update from gui
            self.x_desired = desired_state[0]
            self.y_desired = desired_state[1]
            self.z_desired = desired_state[2]
            self.yaw_desired = desired_state[3]
            # send setpoints

            self.server.mav.set_position_target_local_ned_send(
                int(time.time()),
                1, 1,
                1, #local
                0b000111111000, #take pos and yaw
                self.x_desired, self.y_desired, self.z_desired, # x, y, z positions (not used)
                0, 0, 0, # x, y, z velocity in m/s
                0, 0, 0, # x, y, z acceleration (not used)
                self.yaw_desired, 0.
            )

            # LONG COMMANDS
            # check if there are long commands to be sent
            for i in range(len(self.msg_payload_recv)):
                msg_payload_recv[i] = self.msg_payload_recv[i]

            if msg_payload_send[0] != 0:
                self.lock.acquire()
                self.mav_cmd_id = int(msg_payload_send[0])
                self.param1 = float(msg_payload_send[1])
                self.param2 = float(msg_payload_send[2])
                self.param3 = float(msg_payload_send[3])
                self.param4 = float(msg_payload_send[4])
                self.param5 = float(msg_payload_send[5])
                self.param6 = float(msg_payload_send[6])
                self.param7 = float(msg_payload_send[7])
                self.lock.release()

                if msg_payload_send[0] > 0:
                    #send the command
                    self.server.mav.command_long_send(
                        1, # autopilot system id
                        1, # autopilot component id
                        self.mav_cmd_id, # command id
                        1, # confirmation
                        self.param1,
                        self.param2,
                        self.param3,
                        self.param4,
                        self.param5,
                        self.param6,
                        self.param7 # unused parameters for this command
                    )
                elif msg_payload_send[0] == -1:
                    self.server.mav.scaled_pressure_send(
                        int(time.time()),
                        1,
                        1,
                        1
                    )
                elif msg_payload_send[0] == -2:
                    self.server.mav.raw_pressure_send(
                        int(time.time()),
                        1,
                        1,
                        1,
                        1
                    )
                msg_payload_send[0] = 0


            time.sleep(1/self.msg_per_second)

    def recv_msg(self):

        while not self.close_threads:
            try:
                msg = self.server.recv_match()
                if not msg:
                    continue
                else:
                    pass
                if msg.get_type() == "LOCAL_POSITION_NED":
                    self.msg_payload_recv[0] = msg.id
                    for i,fieldname in enumerate(msg._fieldnames):
                        self.msg_payload_recv[i+1] = getattr(msg, fieldname)

                if msg.get_type() == 'LOCAL_POSITION_NED':
                    #pass
                    logging.info(f"X: {msg.x} \t Y: {msg.y} Z: {msg.z}")

            except KeyboardInterrupt:
                self.close_recv_thread()
                return

        self.close_recv_thread()

        return


def main(msg_payload_recv, msg_payload_send, desired_state, initial_state):

    try:
        server = MAVServer('10.20.60.140', 9999)
        server.conn_server()
        server.run_server(msg_payload_recv, msg_payload_send, desired_state)
    except KeyboardInterrupt:
        logging.info("CLOSING SERVER")
        server.close_server()

if __name__ == "__main__":

    logging.basicConfig(
        level=logging.INFO,
        format='(%(threadName)-10s) %(levelname)-10s %(message)-10s',
    )
    msg_payload_recv = np.zeros(20)
    msg_payload_send = np.zeros(20)
    desired_state = np.zeros(4)
    logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.INFO)
    try:
        server = MAVServer('192.168.10.13', 7777)
        server.conn_server()
        server.run_server(msg_payload_recv, msg_payload_send, desired_state, initial_state)
    except KeyboardInterrupt:
        logging.info("CLOSING SERVER")
        server.close_server()
