#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_CURL
#include <curl/curl.h>
#endif
#include "tess.h"

#define REPO_API_URL "https://api.tesslang.org/packages"
#define REPO_DOWNLOAD_URL "https://api.tesslang.org/download"

#ifdef HAVE_CURL
struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        return 0;
    }
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}
#endif

int repository_search(const char *query) {
#ifdef HAVE_CURL
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    char url[512];
    snprintf(url, sizeof(url), "%s/search?q=%s", REPO_API_URL, query);
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            printf("Search results for '%s':\n%s\n", query, chunk.memory);
            free(chunk.memory);
            curl_easy_cleanup(curl);
            return 0;
        } else {
            printf("Error searching repository: %s\n", curl_easy_strerror(res));
            free(chunk.memory);
            curl_easy_cleanup(curl);
            return 1;
        }
    }
#else
    (void)query;
    printf("Repository search requires libcurl. Install libcurl and rebuild.\n");
    return 1;
#endif
    return 0;
}

int repository_download(const char *package_name, const char *version) {
#ifdef HAVE_CURL
    CURL *curl;
    CURLcode res;
    FILE *file;
    char filename[256];
    char url[512];
    
    /* Install to local project .tess_packages directory */
    snprintf(filename, sizeof(filename), ".tess_packages/%s.tess", package_name);
    snprintf(url, sizeof(url), "%s/%s", REPO_DOWNLOAD_URL, package_name);
    if (version) {
        snprintf(url, sizeof(url), "%s/%s@%s", REPO_DOWNLOAD_URL, package_name, version);
    }
    
    /* Create packages directory if it doesn't exist */
    #ifdef _WIN32
    if (access(".tess_packages", F_OK) != 0) {
        _mkdir(".tess_packages");
    }
    #else
    if (access(".tess_packages", F_OK) != 0) {
        mkdir(".tess_packages", 0755);
    }
    #endif
    
    file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Cannot create package file\n");
        return 1;
    }
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        
        fclose(file);
        
        if (res == CURLE_OK) {
            printf("Package '%s' downloaded successfully\n", package_name);
            curl_easy_cleanup(curl);
            return 0;
        } else {
            printf("Error downloading package: %s\n", curl_easy_strerror(res));
            remove(filename);
            curl_easy_cleanup(curl);
            return 1;
        }
    }
    
    fclose(file);
#else
    (void)package_name;
    (void)version;
    printf("Package download requires libcurl. Install libcurl and rebuild.\n");
    return 1;
#endif
    return 0;
}

