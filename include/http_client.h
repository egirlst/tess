#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

/**
 * Cross-platform HTTP client for Tess language
 * Native implementation - no external dependencies required
 * - Windows: Uses WinHTTP (built into Windows)
 * - Linux/macOS: Uses native sockets (HTTP only, HTTPS requires libcurl)
 */

/**
 * Make an HTTP request
 * @param method HTTP method (GET, POST, PUT, DELETE)
 * @param url Full URL (http:// or https://)
 * @param data Request body data (NULL for GET/DELETE, string for POST/PUT)
 * @param headers Array of header strings (NULL if no custom headers)
 * @param header_count Number of headers
 * @return Response body as string (caller must free), or error message
 */
char* http_request(const char *method, const char *url, const char *data, const char **headers, int header_count);

#endif /* HTTP_CLIENT_H */

