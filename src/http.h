#ifndef HTTP_H
#define HTTP_H

#include <sys/types.h>

enum HTTPMethod { GET, POST };

struct HTTPRequest {
  char path[4096];
  char extension[256];
  enum HTTPMethod method;
};

const char *get_content_type(char *extension);

void parse_http_request(char *request_buffer, struct HTTPRequest *request);

void create_response_headers(char *buffer, size_t buffer_size,
                             const char *content_type, size_t content_size);

#endif /* HTTP_H */
