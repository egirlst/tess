#include "http_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use libcurl if defined */
#ifdef USE_LIBCURL
#include <curl/curl.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
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

static char* http_request_libcurl(const char *method, const char *url, const char *data, const char **headers, int header_count) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    
    if (!curl_handle) {
        free(chunk.memory);
        return strdup("HTTP Error: Failed to init curl");
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Tess Language HTTP Client/1.0");

    if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
        if (data) {
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);
        }
    } else if (strcmp(method, "PUT") == 0) {
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "PUT");
        if (data) {
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);
        }
    } else if (strcmp(method, "DELETE") == 0) {
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    struct curl_slist *header_list = NULL;
    if (headers && header_count > 0) {
        for (int i = 0; i < header_count; i++) {
            header_list = curl_slist_append(header_list, headers[i]);
        }
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, header_list);
    }

    res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: curl_easy_perform() failed: %s", curl_easy_strerror(res));
        free(chunk.memory);
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        return error;
    }

    curl_slist_free_all(header_list);
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    return chunk.memory;
}
#endif

#ifdef _WIN32
/* Windows: Use WinHTTP (built into Windows, no external dependencies) */
#include <windows.h>
#include <winhttp.h>
#ifdef _MSC_VER
#pragma comment(lib, "winhttp.lib")
#endif

static char* http_request_win32(const char *method, const char *url, const char *data, const char **headers, int header_count) {
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    char *response = NULL;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    DWORD dwStatusCode = 0;
    DWORD dwStatusCodeSize = sizeof(DWORD);
    
    /* Parse URL */
    char hostname[256] = {0};
    char path[2048] = {0};
    INTERNET_PORT port = INTERNET_DEFAULT_HTTP_PORT;
    int is_https = 0;
    
    if (strncmp(url, "https://", 8) == 0) {
        is_https = 1;
        port = INTERNET_DEFAULT_HTTPS_PORT;
        url += 8;
    } else if (strncmp(url, "http://", 7) == 0) {
        url += 7;
    }
    
    /* Extract hostname and path */
    const char *path_start = strchr(url, '/');
    if (path_start) {
        size_t host_len = path_start - url;
        if (host_len >= sizeof(hostname)) host_len = sizeof(hostname) - 1;
        memcpy(hostname, url, host_len);
        hostname[host_len] = '\0';
        strncpy(path, path_start, sizeof(path) - 1);
    } else {
        strncpy(hostname, url, sizeof(hostname) - 1);
        path[0] = '/';
        path[1] = '\0';
    }
    
    /* Check for port in hostname */
    char *colon = strchr(hostname, ':');
    if (colon) {
        *colon = '\0';
        port = (INTERNET_PORT)atoi(colon + 1);
    }
    
    /* Initialize WinHTTP */
    wchar_t wUserAgent[] = L"Tess Language HTTP Client/1.0";
    wchar_t wHostname[256];
    wchar_t wPath[2048];
    wchar_t wMethod[16];
    
    MultiByteToWideChar(CP_UTF8, 0, hostname, -1, wHostname, 256);
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wPath, 2048);
    MultiByteToWideChar(CP_UTF8, 0, method, -1, wMethod, 16);
    
    hSession = WinHttpOpen(wUserAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    if (!hSession) {
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: Failed to initialize WinHTTP");
        return error;
    }
    
    /* Connect to server */
    hConnect = WinHttpConnect(hSession, wHostname, port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: Failed to connect to host");
        return error;
    }
    
    /* Create request */
    hRequest = WinHttpOpenRequest(hConnect, wMethod, wPath, NULL, NULL, NULL,
                                   is_https ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: Failed to create request");
        return error;
    }
    
    /* Add custom headers if provided */
    if (headers && header_count > 0) {
        wchar_t wHeaders[4096] = {0};
        for (int i = 0; i < header_count; i++) {
            wchar_t wHeader[512];
            MultiByteToWideChar(CP_UTF8, 0, headers[i], -1, wHeader, 512);
            wcscat(wHeaders, wHeader);
            wcscat(wHeaders, L"\r\n");
        }
        WinHttpAddRequestHeaders(hRequest, wHeaders, -1, WINHTTP_ADDREQ_FLAG_ADD);
    }
    
    /* Send request */
    LPVOID request_data = NULL;
    DWORD data_len = 0;
    if (data && (strcmp(method, "POST") == 0 || strcmp(method, "PUT") == 0)) {
        /* Send data as UTF-8 bytes, not wide chars */
        request_data = (LPVOID)data;
        data_len = (DWORD)strlen(data);
    }
    
    if (!WinHttpSendRequest(hRequest, NULL, 0, request_data, data_len, data_len, 0)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: Failed to send request");
        return error;
    }
    
    /* Wait for response */
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: Failed to receive response");
        return error;
    }
    
    /* Get status code */
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                       NULL, &dwStatusCode, &dwStatusCodeSize, NULL);
    
    /* Read response data */
    size_t response_size = 0;
    size_t response_capacity = 4096;
    response = malloc(response_capacity);
    if (!response) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: Out of memory");
        return error;
    }
    
    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            break;
        }
        
        if (dwSize == 0) break;
        
        /* Ensure we have enough space */
        if (response_size + dwSize + 1 > response_capacity) {
            response_capacity = (response_size + dwSize) * 2;
            char *new_response = realloc(response, response_capacity);
            if (!new_response) {
                free(response);
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                char *error = malloc(256);
                snprintf(error, 256, "HTTP Error: Out of memory");
                return error;
            }
            response = new_response;
        }
        
        dwDownloaded = 0;
        if (!WinHttpReadData(hRequest, response + response_size, dwSize, &dwDownloaded)) {
            break;
        }
        
        response_size += dwDownloaded;
    } while (dwSize > 0);
    
    response[response_size] = '\0';
    
    /* Cleanup */
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    
    return response;
}

#else
/* Linux/macOS: Native socket-based HTTP client */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

static char* http_request_unix(const char *method, const char *url, const char *data, const char **headers, int header_count) {
    int sockfd = -1;
    char *response = NULL;
    char hostname[256] = {0};
    char path[2048] = {0};
    int port = 80;
    int is_https = 0;
    
    /* Parse URL */
    if (strncmp(url, "https://", 8) == 0) {
        is_https = 1;
        port = 443;
        url += 8;
    } else if (strncmp(url, "http://", 7) == 0) {
        url += 7;
    }
    
    /* Extract hostname and path */
    const char *path_start = strchr(url, '/');
    if (path_start) {
        size_t host_len = path_start - url;
        if (host_len >= sizeof(hostname)) host_len = sizeof(hostname) - 1;
        memcpy(hostname, url, host_len);
        hostname[host_len] = '\0';
        strncpy(path, path_start, sizeof(path) - 1);
    } else {
        strncpy(hostname, url, sizeof(hostname) - 1);
        path[0] = '/';
        path[1] = '\0';
    }
    
    /* Check for port in hostname */
    char *colon = strchr(hostname, ':');
    if (colon) {
        *colon = '\0';
        port = atoi(colon + 1);
    }
    
    if (is_https) {
        /* HTTPS not implemented in basic socket version - return error */
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: HTTPS not supported in native implementation. Use http:// URLs or install libcurl for HTTPS support.");
        return error;
    }
    
    /* Resolve hostname */
    struct hostent *server = gethostbyname(hostname);
    if (!server) {
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: Failed to resolve hostname %s", hostname);
        return error;
    }
    
    /* Create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: Failed to create socket");
        return error;
    }
    
    /* Connect to server */
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: Failed to connect to %s:%d", hostname, port);
        return error;
    }
    
    /* Build HTTP request */
    char request[8192];
    int req_len = snprintf(request, sizeof(request),
        "%s %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: Tess Language HTTP Client/1.0\r\n"
        "Connection: close\r\n",
        method, path, hostname);
    
    /* Add custom headers */
    if (headers && header_count > 0) {
        for (int i = 0; i < header_count && req_len < (int)sizeof(request) - 100; i++) {
            int added = snprintf(request + req_len, sizeof(request) - req_len, "%s\r\n", headers[i]);
            if (added > 0) req_len += added;
        }
    }
    
    /* Add Content-Length for POST/PUT */
    if (data && (strcmp(method, "POST") == 0 || strcmp(method, "PUT") == 0)) {
        req_len += snprintf(request + req_len, sizeof(request) - req_len,
                           "Content-Length: %zu\r\n", strlen(data));
    }
    
    /* End headers */
    req_len += snprintf(request + req_len, sizeof(request) - req_len, "\r\n");
    
    /* Add body if present */
    if (data && (strcmp(method, "POST") == 0 || strcmp(method, "PUT") == 0)) {
        int data_len = strlen(data);
        if (req_len + data_len < (int)sizeof(request)) {
            memcpy(request + req_len, data, data_len);
            req_len += data_len;
        }
    }
    
    /* Send request */
    if (send(sockfd, request, req_len, 0) < 0) {
        close(sockfd);
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: Failed to send request");
        return error;
    }
    
    /* Read response */
    size_t response_size = 0;
    size_t response_capacity = 4096;
    response = malloc(response_capacity);
    if (!response) {
        close(sockfd);
        char *error = malloc(256);
        snprintf(error, 256, "HTTP Error: Out of memory");
        return error;
    }
    
    char buffer[4096];
    int headers_done = 0;
    int content_length = -1;
    
    while (1) {
        int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) break;
        buffer[n] = '\0';
        
        if (!headers_done) {
            /* Find end of headers */
            char *header_end = strstr(buffer, "\r\n\r\n");
            if (header_end) {
                headers_done = 1;
                /* Extract Content-Length if present */
                char *cl_header = strstr(buffer, "Content-Length:");
                if (cl_header) {
                    content_length = atoi(cl_header + 15);
                }
                /* Skip to body */
                char *body_start = header_end + 4;
                int body_len = n - (body_start - buffer);
                if (body_len > 0) {
                    if (response_size + body_len + 1 > response_capacity) {
                        response_capacity = (response_size + body_len) * 2;
                        char *new_response = realloc(response, response_capacity);
                        if (!new_response) {
                            free(response);
                            close(sockfd);
                            char *error = malloc(256);
                            snprintf(error, 256, "HTTP Error: Out of memory");
                            return error;
                        }
                        response = new_response;
                    }
                    memcpy(response + response_size, body_start, body_len);
                    response_size += body_len;
                }
            }
        } else {
            /* Reading body */
            if (response_size + n + 1 > response_capacity) {
                response_capacity = (response_size + n) * 2;
                char *new_response = realloc(response, response_capacity);
                if (!new_response) {
                    free(response);
                    close(sockfd);
                    char *error = malloc(256);
                    snprintf(error, 256, "HTTP Error: Out of memory");
                    return error;
                }
                response = new_response;
            }
            memcpy(response + response_size, buffer, n);
            response_size += n;
            
            /* Stop if we got all content */
            if (content_length > 0 && (int)response_size >= content_length) {
                break;
            }
        }
    }
    
    response[response_size] = '\0';
    close(sockfd);
    
    return response;
}
#endif

/* Main HTTP request function - uses native implementation */
char* http_request(const char *method, const char *url, const char *data, const char **headers, int header_count) {
#ifdef USE_LIBCURL
    return http_request_libcurl(method, url, data, headers, header_count);
#else
#ifdef _WIN32
    return http_request_win32(method, url, data, headers, header_count);
#else
    return http_request_unix(method, url, data, headers, header_count);
#endif
#endif
}
