import threading
import logging
import TCP_Server
import Image_Recognition
import Jarvis
import cv2
import time

def main():
    # Run MavLink software
    logging.info("RUNNING SERVER")

    server = TCP_Server.MAVServer("localhost", 8777)
    image = Image_Recognition.MAVImageRecognition(server)
    jarvis = Jarvis.Jarvis(server, image)

    server_thread = threading.Thread(target=TCP_Server.main, args=(server,))
    image_thread = threading.Thread(target=Image_Recognition.main, args=(image,))
    jarvis_thread = threading.Thread(target=Jarvis.main, args=(jarvis,))

    server_thread.start()

    while not server.server_started:
        time.sleep(0.1)
    image_thread.start()
    jarvis_thread.start()
    """
    time_start = time.time()
    timeout = 10
    while time.time() < time_start + timeout:
        pass
    """
    jarvis_thread.join()
    image_thread.join()
    server_thread.join()



if __name__ == "__main__":
    # logging.basicConfig(filename="log.log", filemode="w", format='%(levelname)s:%(message)s', level=logging.INFO)
    logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.INFO)
    main()
