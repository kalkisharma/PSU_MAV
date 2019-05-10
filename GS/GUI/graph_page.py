try:
    from global_include import *
except ModuleNotFoundError:
    from GUI.global_include import *

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

class graph_page(tk.Frame):

    def __init__(self, desired_state, parent, controller):

        tk.Frame.__init__(self, parent)
        label = ttk.Label(self, text="Graph Page", font=LARGE_FONT)
        label.grid(row=0, column=0, columnspan=2, sticky=tk.W, padx=15)
        #self.grid_rowconfigure(1, weight=1)
        #self.grid_columnconfigure(1, weight=1)

        controller.init_buttons(self)
        """
        controller.master.bind('<Up>', self.move_up)
        controller.master.bind('<Down>', self.move_down)
        controller.master.bind('<Left>', self.move_left)
        controller.master.bind('<Right>', self.move_right)

        canvas = FigureCanvasTkAgg(f, self)
        canvas.draw()
        canvas.get_tk_widget().grid(row=3,column=1,columnspan=100,rowspan=100)

        self.xlabel = ttk.Label(self, text="X = " + str(XLIST[-1]))
        self.xlabel.grid(row = 3, column = 0)

        self.ylabel = ttk.Label(self, text="Y = " + str(YLIST[-1]))
        self.ylabel.grid(row = 4, column = 0)

        controller.roll.set("0.0")
        self.rlabel = ttk.Label(self, textvariable=controller.roll)
        self.rlabel.grid(row = 5, column = 0)

        controller.pitch.set("0.0")
        self.rlabel = ttk.Label(self, textvariable=controller.pitch)
        self.rlabel.grid(row = 6, column = 0)

        controller.yaw.set("0.0")
        self.rlabel = ttk.Label(self, textvariable=controller.yaw)
        self.rlabel.grid(row = 7, column = 0)
        """
        #canvas.grid
        #canvas._tkcanvas.grid(column=0, row=0, sticky="nesw")
        #canvas._tkcanvas.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

    def move_up(self, event = None):

        XLIST.append(XLIST[-1])
        YLIST.append(YLIST[-1] + 1)

        self.xlabel.config(text="X = " + str(XLIST[-1]))
        self.ylabel.config(text="Y = " + str(YLIST[-1]))

    def move_down(self, event = None):

        XLIST.append(XLIST[-1])
        YLIST.append(YLIST[-1] - 1)

        self.xlabel.config(text="X = " + str(XLIST[-1]))
        self.ylabel.config(text="Y = " + str(YLIST[-1]))

    def move_left(self, event = None):

        XLIST.append(XLIST[-1] - 1)
        YLIST.append(YLIST[-1])

        self.xlabel.config(text="X = " + str(XLIST[-1]))
        self.ylabel.config(text="Y = " + str(YLIST[-1]))

    def move_right(self, event = None):

        XLIST.append(XLIST[-1] + 1)
        YLIST.append(YLIST[-1])

        self.xlabel.config(text="X = " + str(XLIST[-1]))
        self.ylabel.config(text="Y = " + str(YLIST[-1]))
