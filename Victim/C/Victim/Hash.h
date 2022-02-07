#pragma once
#include <stdio.h>
#include <openssl/md5.h>

unsigned char* calculateHash(char* filePath)
{
    unsigned char c[MD5_DIGEST_LENGTH];
    //char* filename = "C:\\Users\\Matan\\Documents\\CProjects\\AnyDeskVirus\\anyDeskFile\\AnyDesk.exe";
    int i;
    FILE* inFile;
    fopen_s(&inFile, filePath, "rb");
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];

    if (inFile == NULL) {
        printf("%s can't be opened.\n", filePath);
        return NULL;
    }

    MD5_Init(&mdContext);
    while ((bytes = fread(data, 1, 1024, inFile)) != 0)
        MD5_Update(&mdContext, data, bytes);
    MD5_Final(c, &mdContext);
    for (i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", c[i]);
    printf(" %s\n", filePath);
    fclose(inFile);
    return c;
}