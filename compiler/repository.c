#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "repository.h"
#include "http_client.h"

#define REPO_URL "https://raw.githubusercontent.com/tess-lang/packages/main"

int repository_download(const char *package_name, const char *version) {
    char url[1024];
    if (version) {
        snprintf(url, sizeof(url), "%s/%s/%s/package.tess", REPO_URL, package_name, version);
    } else {
        snprintf(url, sizeof(url), "%s/%s/latest/package.tess", REPO_URL, package_name);
    }
    
    printf("Downloading from %s...\n", url);
    
    char *content = http_request("GET", url, NULL, NULL, 0);
    if (!content || strncmp(content, "HTTP Error", 10) == 0) {
        if (content) free(content);
        return 0; 
    }
    
    char path[1024];
    snprintf(path, sizeof(path), ".tess_packages/%s.tess", package_name);
    
    FILE *file = fopen(path, "w");
    if (!file) {
        free(content);
        return 0;
    }
    
    fputs(content, file);
    fclose(file);
    free(content);
    
    return 1;
}

int repository_search(const char *query) {
    printf("Searching for '%s'...\n", query);
    printf("(Search not yet implemented)\n");
    return 0;
}
