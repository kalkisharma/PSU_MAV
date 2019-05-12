import time
import logging
import math

class Jarvis:
    def __init__(self, server_, image_):
        self.server = server_
        self.image = image_
        self.takeoff_flag = True
        self.hover_flag = False
        self.first_setpoint_flag = False
        self.second_setpoint_flag = False
        self.third_setpoint_flag = False
        self.search_flag = False
        self.land_flag = False

    def update_setpoint(self, setpoint):
        self.server.x_desired = setpoint[0]
        self.server.y_desired = setpoint[1]
        self.server.z_desired = setpoint[2]
        self.server.yaw_desired = 0

    def enable_offboard(self):
        self.server.mav_cmd = [92, 1, 0, 0, 0, 0, 0, 0]
        time.sleep(0.1)

    def arm_pixhawk(self):
        self.server.mav_cmd = [400, 1, 0, 0, 0, 0, 0, 0]
        time.sleep(0.1)

    def open_servo(self):
        self.server.mav_cmd = [-1, 1, 0, 0, 0, 0, 0, 0]

    def close_servo(self):
        self.server.mav_cmd = [-2, 1, 0, 0, 0, 0, 0, 0]

    def dist_to_setpoint(self, setpoint):
        x = setpoint[0] - self.server.current_x
        y = setpoint[1] - self.server.current_y
        z = setpoint[2] - self.server.current_z
        return math.sqrt(x**2 + y**2 + z**2)
def main(jarvis):
    jarvis.image.tag_name = "PICKUP"
    #jarvis.enable_offboard()
    #jarvis.arm_pixhawk()
    #jarvis.open_servo()
    #jarvis.close_servo()
    first_setpoint = [0, 0, -2]
    second_setpoint = [0, 0, -2]
    third_setpoint = [0, 0, -2]
    fourth_setpoint = [0, 0, -2]
    while True:
        if jarvis.takeoff_flag:
            setpoint = [0, 0, -2]
            if jarvis.dist_to_setpoint(setpoint) < 0.1:
                jarvis.takeoff_flag = False
                jarvis.hover_flag = True
                start_time = time.time()
                timeout = 5
        if jarvis.hover_flag:
            setpoint = [jarvis.server.current_x, jarvis.server.current_y, jarvis.server.current_z]
            if time.time() > start_time + timeout:
                jarvis.hover_flag = False
                jarvis.first_setpoint_flag = True
        if jarvis.first_setpoint_flag:
            setpoint = first_setpoint
            if jarvis.dist_to_setpoint(setpoint) < 0.1:
                jarvis.first_setpoint_flag = False
                jarvis.second_setpoint_flag = True
        if jarvis.second_setpoint_flag:
            setpoint = second_setpoint
            if jarvis.dist_to_setpoint(setpoint) < 0.1:
                jarvis.second_setpoint_flag = False
                jarvis.image.tag_name = "DROPOFF"
        if jarvis.image.tag_name == "DROPOFF":
            jarvis.search_flag = True
            if jarvis.image.x not None and jarvis.image.y not None:
                setpoint = [jarvis.image.x, jarvis.image.y, jarvis.server.current_z]
                jarvis.serach_flag = False
                jarvis.image.target_name = "DROPOFF"
                jarvis.image.tag_name = ""
                jarvis.reach_target = True
        if jarvis.reach_target:
            if jarvis.dist_to_setpoint(setpoint) < 0.1:
                jarvis.reach_target = False
                jarvis.image.x = None
                jarvis.image.y = None
        if jarvis.image.target_name == "DROPOFF" and jarvis.reach_target == False:
            if jarvis.image.x not None and jarvis.image.y not None:
                setpoint = [jarvis.image.x, jarvis.image.y, jarvis.server.current_z]
                jarvis.reach_target = True
                jarvis.image.target_name = ""
                jarvis.third_setpoint_flag = True
                jarvis.land_flag = True
        if jarvis.land_flag:
            if jarvis.dist_to_setpoint(setpoint) < 0.1:
                setpoint = [jarvis.server.current_x, jarvis.server.current_y, 0]

        jarvis.update_setpoint(setpoint)
        #print(jarvis.image.x,jarvis.image.y)
