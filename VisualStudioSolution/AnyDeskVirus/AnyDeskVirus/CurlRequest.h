#define CURL_STATICLIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

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


struct Server getIpAndPort()
{
    CURL* curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    struct Server server;

    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, "https://sc-initiation-5d638-default-rtdb.firebaseio.com/.json");

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);

    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* get it! */
    res = curl_easy_perform(curl_handle);

    /* check for errors */
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));

        printf("Can't get the ip and port!");
        exit(1);
    }
    else {
        /*
         * Now, our chunk.memory points to a memory block that is chunk.size
         * bytes big and contains the remote file.
         *
         * Do something nice with it!
         */
        int ipStartIndex = 0, ipEndIndex = 0, portStartIndex = 0, portEndIndex = 0;

        for (size_t i = 0; i < chunk.size; i++)
        {
            if (chunk.memory[i] == 'i' && chunk.memory[i + 1] == 'p') {
                ipStartIndex = i + 5;
            }
            if (chunk.memory[i] == 'p' && chunk.memory[i + 1] == 'o' && chunk.memory[i + 2] == 'r' && chunk.memory[i + 3] == 't') {
                ipEndIndex = i - 4;
                portStartIndex = i + 7;
                portEndIndex = chunk.size - 3;
            }
        }

        strncpy_s(server.IP, sizeof(server.IP), chunk.memory + ipStartIndex, ipEndIndex - ipStartIndex + 1);
        strncpy_s(server.PORT, sizeof(server.PORT), chunk.memory + portStartIndex, portEndIndex - portStartIndex + 1);
        return server;
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);

    if (chunk.memory)
        free(chunk.memory);

    /* we're done with libcurl, so clean it up */
    curl_global_cleanup();
}