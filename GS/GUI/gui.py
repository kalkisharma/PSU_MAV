import webbrowser as wb
import math
import numpy as np
import logging
import time
#import matplotlib.animation as animation
from matplotlib.figure import Figure
import PIL.Image, PIL.ImageTk
import cv2
from mpl_toolkits.mplot3d import Axes3D

if __name__=='__main__':
    from global_include import *
    from main_page import main_page
    from graph_page import graph_page
    from command_page import command_page
else:
    from GUI.global_include import *
    from GUI.main_page import main_page
    from GUI.graph_page import graph_page
    from GUI.command_page import command_page

WINDOW_WIDTH = 600
WINDOW_HEIGHT = 600

DUAL_MONITOR = False

class Window(tk.Tk):

    def __init__(self, desired_state, master = None):

        container = tk.Frame.__init__(self, master)
        self.master = master
        self.first_read = True
        self.time_start = time.time()
        self.init_window(container, desired_state)

    def init_window(self, container, desired_state):

        self.window_config()
        self.window_menu()
        self.cycle_frames(container, desired_state)

    def window_config(self):

        self.master.title("Quadcopter GUI")

        #self.master.configure(background = '#000000')

        if (DUAL_MONITOR):
            screen_width = self.winfo_screenwidth()/2
        else:
            screen_width = self.winfo_screenwidth()

        screen_height = self.winfo_screenheight()
        x_coord = screen_width/2 - WINDOW_WIDTH/2
        y_coord = screen_height/2 - WINDOW_HEIGHT/2

        self.master.geometry("%dx%d+%d+%d" % (WINDOW_WIDTH, WINDOW_HEIGHT, x_coord, y_coord))

        #self.master.resizable(width = False, height = False)

    def window_menu(self):

        menu = tk.Menu(self.master)
        self.master.config(menu=menu)

        file = tk.Menu(menu)
        file.add_command(label="Save")
        file.add_command(label="Exit", command=self.close_window, accelerator = "Esc")
        self.master.bind("<Escape>", self.close_window)
        self.master.bind("<Control-c>", self.close_window)
        menu.add_cascade(label="File", menu=file)

        help = tk.Menu(menu)
        help.add_command(label="Visit Webpage", command=self.open_webpage)
        menu.add_cascade(label="Help", menu=help)

        edit = tk.Menu(menu)
        edit.add_command(label="Undo")
        menu.add_cascade(label="Edit", menu=edit)

    def close_window(self, event = None):

        exit()

    def open_webpage(self):

        wb.open("http://pythonprogramming.net")

    def cycle_frames(self, container, desired_state):

        self.frames = {}

        for _frame in (main_page, graph_page, command_page):

            page_name = _frame.__name__
            frame = _frame(desired_state, parent = container, controller = self)
            self.frames[page_name] = frame
            frame.grid(row=0, column=0, sticky="nsew")

        self.show_frame("main_page")

    def show_frame(self, page_name):

        frame = self.frames[page_name]
        frame.tkraise()

    def init_buttons(self, controller):

        button1 = ttk.Button(controller, text="Home",
                            command=lambda: self.show_frame("main_page"))
        button1.grid(row=1, column=0)

        button2 = ttk.Button(controller, text="Graphs",
                            command=lambda: self.show_frame("graph_page"))
        button2.grid(row=1, column=1)

        button3 = ttk.Button(controller, text="Commands",
                            command=lambda: self.show_frame("command_page"))
        button3.grid(row=1, column=2)

def update_video(command_page):
    ret, frame = command_page.vid.get_frame()

    if ret and command_page.panel is not None:
        image = PIL.ImageTk.PhotoImage(PIL.Image.fromarray(frame))
        command_page.panel.configure(image=image)
        command_page.panel.image = image
        # Display the resulting frame
    """
        if not cv2.waitKey(1) & 0xFF == ord('q'):
            cv2.imshow('frame',frame)
        photo = PIL.ImageTk.PhotoImage(image = PIL.Image.fromarray(frame))
        command_page.canvas.create_image(0, 0, anchor=tk.NW, image=photo)
    """

def update_current_setpoint(app, command_page, desired_state, msg_payload_send, msg_payload_recv, initial_state):
    msg_payload = msg_payload_recv[:]

    if int(msg_payload[0]) == 32:
        command_page.x_current.set("%.2f" % msg_payload[2])
        command_page.y_current.set("%.2f" % msg_payload[3])
        command_page.z_current.set("%.2f" % msg_payload[4])
        if app.first_read:
            command_page.x_desired.set("%.2f" % msg_payload[2])
            command_page.y_desired.set("%.2f" % msg_payload[3])
            command_page.z_desired.set("%.2f" % msg_payload[4])
            desired_state[0] = float(msg_payload[2])
            desired_state[1] = float(msg_payload[3])
            desired_state[2] = float(msg_payload[4])
            initial_state[0] = float(msg_payload[2])
            initial_state[1] = float(msg_payload[3])
            initial_state[2] = float(msg_payload[4])
            app.first_read = False
    elif int(msg_payload[0]) == 30:
        command_page.roll_current.set("%.2f" % msg_payload[2])
        command_page.pitch_current.set("%.2f" % msg_payload[3])
        command_page.yaw_current.set("%.2f" % msg_payload[4])
    if int(command_page.mav_cmd[0]) != 0:
        print(command_page.mav_cmd)
        for i,item in  enumerate(command_page.mav_cmd):
            msg_payload_send[i] = item
        #time.sleep(0.5)
        #msg_payload_send[0] = 0
        command_page.mav_cmd = [0, 0, 0, 0, 0, 0, 0, 0]

def update_plot(command_page, desired_state, time_start, dt=1):
    if dt < (time.time() - time_start):
        if float(command_page.x_current.get()) != command_page.current_x_values[-1] or \
            float(command_page.y_current.get()) != command_page.current_y_values[-1] or \
            float(command_page.z_current.get()) != command_page.current_z_values[-1]:
            command_page.current_x_values.append(float(command_page.x_current.get()))
            command_page.current_y_values.append(float(command_page.y_current.get()))
            command_page.current_z_values.append(float(command_page.z_current.get()))

        if float(desired_state[0]) != command_page.desired_x_values[-1] or \
            float(desired_state[1]) != command_page.desired_y_values[-1] or \
            float(desired_state[2]) != command_page.desired_z_values[-1]:
            command_page.desired_x_values.append(float(desired_state[0]))
            command_page.desired_y_values.append(float(desired_state[1]))
            command_page.desired_z_values.append(float(desired_state[2]))


        """
        command_page.a.plot3D(
            command_page.current_x_values,
            command_page.current_y_values,
            command_page.current_z_values,
            'gray'
        )

        command_page.a.plot3D(
            command_page.current_x_values[-2:],
            command_page.current_y_values[-2:],
            command_page.current_z_values[-2:]
        )
        """

        command_page.a.plot(
            command_page.current_x_values,
            command_page.current_y_values,
            color='blue',
            linestyle='--',
            #linewidth=2,
            marker='o'
        )
        command_page.a.plot(
            command_page.desired_x_values,
            command_page.desired_y_values,
            color='green',
            linestyle='--',
            #linewidth=2,
            marker='o'
        )

        command_page.canvas.draw()

        return (time.time())
    else:
        return time_start

def update_data(root, app, msg_payload_recv, msg_payload_send, desired_state, initial_state):

    command_page = app.frames['command_page']

    update_current_setpoint(app, command_page, desired_state, msg_payload_send, msg_payload_recv, initial_state)
    update_video(command_page)
    if not app.first_read:
        app.time_start = update_plot(command_page, desired_state, app.time_start, dt=1)

    root.after(
        1, # Update time in milliseconds
        update_data, root, app, msg_payload_recv, msg_payload_send, desired_state, initial_state
    )

def main(msg_payload_recv, msg_payload_send, desired_state, initial_state):
    root = tk.Tk()
    app = Window(desired_state, root)
    update_data(root, app, msg_payload_recv, msg_payload_send, desired_state, initial_state)
    root.mainloop()

if __name__ == "__main__":
    main(np.zeros(20), np.zeros(20), np.zeros(4))
