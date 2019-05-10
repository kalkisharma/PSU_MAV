try:
    from global_include import *
except ModuleNotFoundError:
    from GUI.global_include import *

class main_page(tk.Frame):

    def __init__(self, desired_state, parent, controller):

        tk.Frame.__init__(self, parent)
        label = ttk.Label(self, text="Main Page", font=LARGE_FONT)
        label.grid(row=0, column=0, columnspan=2, sticky=tk.W, padx=15)
        #self.grid_rowconfigure(1, weight=1)
        #self.grid_columnconfigure(1, weight=1)

        controller.init_buttons(self)
