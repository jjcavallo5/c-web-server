#ifndef HTTP_H
#define HTTP_H

#include <linux/limits.h>
#include <sys/types.h>

typedef enum HTTPMethod { GET, POST, PUT, DELETE } HTTPMethod;

typedef enum HTTPStatus {
  OK = 200,
  NOT_FOUND = 404,
  SERVER_ERROR = 500,
} HTTPStatus;

typedef struct HTTPRequest {
  char path[PATH_MAX];
  char extension[256];
  enum HTTPMethod method;
} HTTPRequest;

typedef struct HTTPHeaders {
  char content_type[64];
  size_t content_length;
} HTTPHeaders;

typedef struct HTTPResponse {
  HTTPStatus status;
  HTTPHeaders headers;
  char body[8192];
} HTTPResponse;

const char *get_content_type(char *extension);

void parse_http_request(char *request_buffer, struct HTTPRequest *request);

void form_http_response(char *out_buffer, ssize_t buffer_size,
                        HTTPResponse *response);

void create_response_headers(HTTPHeaders *headers, ssize_t content_length,
                             const char *content_type);

void send_404(HTTPResponse *res);

#endif /* HTTP_H */
