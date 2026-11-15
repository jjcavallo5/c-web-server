#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *get_content_type(char *extension) {
  if (strcmp(extension, "html") == 0)
    return "text/html";
  if (strcmp(extension, "ico") == 0)
    return "image/x-icon";
  if (strcmp(extension, "json") == 0)
    return "application/json";
  if (strcmp(extension, "css") == 0)
    return "text/css";
  if (strcmp(extension, "js") == 0)
    return "text/javascript";
  if (strcmp(extension, "png") == 0)
    return "image/png";
  if (strcmp(extension, "jpg") == 0)
    return "image/jpeg";
  if (strcmp(extension, "jpeg") == 0)
    return "image/jpeg";
  return "text/plain";
}

void parse_http_request(char *request_buffer, struct HTTPRequest *request) {
  // Parse Method (GET and POST supported for now)
  if (strncmp(request_buffer, "GET", 3) == 0) {
    request->method = GET;
  } else if (strncmp(request_buffer, "POST", 4) == 0) {
    request->method = POST;
  } else {
    printf("Request method not recognized\n");
    exit(EXIT_FAILURE);
  }

  // Parse Path
  char *first_space = strchr(request_buffer, ' ');
  int start_idx = first_space - request_buffer + 1;
  char *second_space = strchr(request_buffer + start_idx, ' ');
  int path_len = second_space - first_space - 1;
  strncpy(request->path, request_buffer + start_idx, path_len);

  // Parse Extention
  char *dot = strrchr(request->path, '.');
  int idx_of_dot = dot - request->path;
  int extension_len = strlen(request->path) - idx_of_dot - 1;
  strncpy(request->extension, request->path + idx_of_dot + 1, extension_len);
}

void print_request(struct HTTPRequest *request) {
  const char *format = "Request Method:\t%s\n"
                       "Request Path:\t%s\n"
                       "Request Extension:\t%s\n";
  printf(format, request->method, request->path, request->extension);
}

void create_response_headers(char *buffer, size_t buffer_size,
                             const char *content_type, size_t content_size) {
  char format[] = "HTTP/1.1 200 OK\r\n"
                  "Connection: keep-alive\r\n"
                  "Content-Length: %d\r\n"
                  "Content-Type: %s\r\n\r\n";
  snprintf(buffer, buffer_size, format, content_size, content_type);
}
