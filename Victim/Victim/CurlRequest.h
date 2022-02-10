#pragma once

#define CURL_STATICLIB
#define REQUEST_FAILED_ERROR 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <stdbool.h>

struct MemoryStruct {
    char* memory;
    size_t size;
};

struct Server {
    char IP[50]; // If we will have ipv6 address
    char PORT[6]; // the maximum port is 65535 - 5 characters + null terminator
};

static size_t
WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    char* ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int getRequest(char * url, struct MemoryStruct *response) {
    /*
    Gets get request

    param 1: the url to get
    param 2: pointer to the response object which will contain the response
    
    return:
        on sucess - 0
        else - error
    */


    CURL* curl_handle;
    CURLcode res;
    bool isThereError = false;

    response->memory = malloc(1);  /* will be grown as needed by the realloc above */
    response->size = 0;    /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)response);

    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* get it! */
    res = curl_easy_perform(curl_handle);

    /* check for errors */
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));


        free(response->memory);
        isThereError = true;
    }
    

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);

    /* we're done with libcurl, so clean it up */
    curl_global_cleanup();

    if (isThereError) {
        return REQUEST_FAILED_ERROR;
    }
    return 0;
}

int getIpAndPort(struct Server *server)
{
    /*
    Gets the ip and the port of the server

    param 1: will contain the response

    on sucess - 0
    else - error

    
    */
    struct MemoryStruct response;    
    
    if (getRequest("https://sc-initiation-5d638-default-rtdb.firebaseio.com/.json", &response) != 0) {
        return REQUEST_FAILED_ERROR;
    }
    else {
        int ipStartIndex = 0, ipEndIndex = 0, portStartIndex = 0, portEndIndex = 0;

        for (size_t i = 0; i < response.size; i++)
        {
            if (response.memory[i] == 'i' && response.memory[i + 1] == 'p') {
                ipStartIndex = i + 5;
            }
            if (response.memory[i] == 'p' && response.memory[i + 1] == 'o' && response.memory[i + 2] == 'r' && response.memory[i + 3] == 't') {
                ipEndIndex = i - 4;
                portStartIndex = i + 7;
                portEndIndex = i + 11;
            }
        }

        strncpy_s(server->IP, sizeof(server->IP), response.memory + ipStartIndex, ipEndIndex - ipStartIndex + 1);
        strncpy_s(server->PORT, sizeof(server->PORT), response.memory + portStartIndex, portEndIndex - portStartIndex + 1);
        free(response.memory);
        return 0;
    }    
}


char* getHashOfAnyDeskFile()
{
    /*
    Returns the hash of the anydesk file

    return:
    on sucess - pointer to hash string
    on error - pointer to NULL
    */
    
    char* hash = NULL;
    struct MemoryStruct response;
    if (getRequest("https://sc-initiation-5d638-default-rtdb.firebaseio.com/hash.json", &response) != 0) {
        return hash;
    }
    else {
        hash = calloc(sizeof(char), response.size);
        strncpy_s(hash, response.size, response.memory + 1, response.size - 2);
        free(response.memory);
        return hash;
    }
}


char* getFileHash(char* filePath) {
    /*
    Returns hash of file

    param 1: the file to hash
    */

    unsigned char buffer_md5[MD5_DIGEST_LENGTH], data[1024];
    char* md5_as_string = calloc(200, sizeof(char)); // TODO: check how much size this has to be
    int i, bytes;;
    FILE* inFile;
    MD5_CTX mdContext;

    fopen_s(&inFile, filePath, "rb");

    // If file does not exist
    if (inFile == NULL) {
        return false;
    }

    // Calculates the hash
    MD5_Init(&mdContext);
    while ((bytes = fread(data, 1, sizeof(data), inFile)) != 0)
        MD5_Update(&mdContext, data, bytes);
    MD5_Final(buffer_md5, &mdContext);
    fclose(inFile);

    // Converts the hash to string
    char buf[32];
    for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf_s(buf, sizeof(buf), "%02x", buffer_md5[i]);
        strcat_s(md5_as_string, 200, buf);
    }


    return md5_as_string;
}