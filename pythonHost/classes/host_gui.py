import socket
import threading
import tkinter as tk
from tkinter.messagebox import showerror, showinfo


class HostGUI:

    victimsList = []

    def __init__(self, opertaionListItems, startButtonCallAble, victimsList=[]):
        self.window = tk.Tk()
        self.window.title("Python Shell Virus")

        self.victimsListBox = victimsList
        self.startButtonCallAble = startButtonCallAble

        # for scrolling vertically
        yscrollbar = tk.Scrollbar(self.window)
        yscrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        # Headers Labels
        tk.Label(text="Welcome To The Python Virus Shell!").pack()
        tk.Label(text="What Would You Like To Do?").pack()

        # Opertaion list
        variable = tk.StringVar(self.window)
        variable.set(opertaionListItems[0])  # default value
        opertaionList = tk.OptionMenu(
            self.window, variable, *opertaionListItems)

        opertaionList.pack()

        # Victims list
        self.victimsListBox = tk.Listbox(self.window, selectmode="single",
                                         yscrollcommand=yscrollbar.set)
        # Widget expands horizontally and
        # vertically by assigning both to
        # fill option
        self.victimsListBox.pack(padx=10, pady=10,
                                 expand=tk.YES, fill="both")
        x = list(self.victimsList)

        for each_item in range(len(x)):
            self.victimsListBox.insert(tk.END, x[each_item].getAddr())
            self.victimsListBox.itemconfig(each_item, bg="lime")

        # Attach listbox to vertical scrollbar
        yscrollbar.config(command=self.victimsListBox.yview)

        # Start Button
        startBtn = tk.Button(
            text="Start!",
            command=lambda:  threading.Thread(target=self.startButtonOnClick, args=(
                variable.get(), self.victimsListBox.get(tk.ACTIVE))).start()

        )
        startBtn.pack()

    def startButtonOnClick(self, selectedOption, selectedVictimAddr):

        selectedVictim = None
        for i in self.victimsList:
            if i.getAddr() == selectedVictimAddr:
                selectedVictim = i
                break

        if selectedVictim == None:
            self.showError("Error", "Please choose a victim!")
        else:
            try:
                self.startButtonCallAble(selectedOption, selectedVictim)
            except socket.error:
                self._removeFromVictimsList(selectedVictim.getAddr())
                self.victimsList.remove(selectedVictim)

    def addVictim(self, victim):
        self.victimsList.append(victim)
        self._addToVictimsList(victim.getAddr())

    def build(self):
        print("Building UI...")
        self.window.mainloop()

    def _addToVictimsList(self, value):
        self.victimsListBox.insert(tk.END, value)

    def _removeFromVictimsList(self, value):
        idx = self.victimsListBox.get(0, tk.END).index(value)
        self.victimsListBox.delete(idx)

    def showError(self, title, desc):
        tk.Tk().withdraw()  # TODO: check that becuase i saw this on the stackoverflow example
        showerror(title, desc)

    def showInfo(self, title, desc):
        tk.Tk().withdraw()  # TODO: check that becuase i saw this on the stackoverflow example
        showinfo(title, desc)
