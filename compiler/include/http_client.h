#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

char* http_request(const char *method, const char *url, const char *data, const char **headers, int header_count);

#endif
