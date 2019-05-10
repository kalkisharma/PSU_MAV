from multiprocessing import Process, Value, Array

import GUI.gui as gui
import TCP_Server.tcp_server as server
import logging

def main():
    msg_payload_recv = Array('d', 20)   # Data recieved on-board
    msg_payload_send = Array('d', 20)   # Data sent to on-board
    initial_state = Array('d', 4)        # [x, y, z, yaw] in local ned frame
    desired_state = Array('d', 4)       # [x, y, z, yaw] in local ned frame

    # Run MavLink software
    logging.info("RUNNING SERVER")
    server_process = Process(target=server.main, args=(msg_payload_recv, msg_payload_send, desired_state, initial_state))
    server_process.start()

    # Run GUI software
    logging.info("RUNNING GUI")
    gui_process = Process(target=gui.main, args=(msg_payload_recv, msg_payload_send, desired_state, initial_state))
    gui_process.start()

    server_process.join()
    gui_process.join()

if __name__ == "__main__":
    #logging.basicConfig(filename="log.log", filemode="w", format='%(levelname)s:%(message)s', level=logging.INFO)
    logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.INFO)
    main()
