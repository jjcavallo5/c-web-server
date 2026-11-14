#include <asm-generic/errno.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define PORT "8080"

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

int create_server_socket(char *port) {
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  struct addrinfo *result;
  int addr_info = getaddrinfo(NULL, port, &hints, &result);
  int server_socket =
      socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);

  if (server_socket <= 0) {
    printf("Failed to create socket\n");
    exit(EXIT_FAILURE);
  }

  int bind_res = bind(server_socket, result->ai_addr, result->ai_addrlen);

  if (bind_res != 0) {
    printf("Failed to bind socket: %d\n", errno);
    perror("bind");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(result);

  listen(server_socket, 50);
  return server_socket;
}

void create_response_headers(char *buffer, size_t buffer_size,
                             const char *content_type) {
  char format[] = "HTTP/1.1 200 OK\r\n"
                  "Connection: close\r\n"
                  "Content-Type: %s\r\n\r\n\0";
  snprintf(buffer, buffer_size, format, content_type);
}

enum HTTPMethod { GET, POST };

struct HTTPRequest {
  enum HTTPMethod method;
  char path[4096];
  char extension[64];
};

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

ssize_t read_file(const char *file_path, char *file_buffer,
                  size_t buffer_size) {
  // Read file
  int fd = open(file_path, O_RDONLY);
  ssize_t bytes_read = read(fd, file_buffer, buffer_size);
  file_buffer[bytes_read] = '\0';
  close(fd);
  return bytes_read;
}

void print_request(struct HTTPRequest *request) {
  const char *format = "Request Method:\t%s\n"
                       "Request Path:\t%s\n"
                       "Request Extension:\t%s\n";
  printf(format, request->method, request->path, request->extension);
}

void accept_client(int server_socket_fd, char *serve_from) {
  // Listen
  struct sockaddr *client;
  char buff[8192];
  socklen_t client_len = sizeof(*client);

  int client_socket_fd = accept(server_socket_fd, client, &client_len);
  ssize_t bytes_recieved = recv(client_socket_fd, &buff, sizeof(buff) - 1, 0);
  if (bytes_recieved > 0) {
    buff[bytes_recieved] = '\0';
  }
  printf("Message recieved: \n\n%s\n", buff);

  // Parse HTTP method
  struct HTTPRequest request;
  memset(&request, 0, sizeof(request));
  parse_http_request(buff, &request);
  char serve_file_path[2048];
  snprintf(serve_file_path, sizeof(serve_file_path), "%s%s", serve_from,
           request.path);

  // Send Response Headers
  char header_buffer[4096];
  const char *content_type = get_content_type(request.extension);
  create_response_headers(header_buffer, sizeof(header_buffer), content_type);
  int bytes_sent =
      send(client_socket_fd, header_buffer, strlen(header_buffer), 0);

  // Send Response Body
  char resp_buffer[8192];
  memset(&resp_buffer, 0, sizeof(resp_buffer));
  ssize_t resp_bytes =
      read_file(serve_file_path, resp_buffer, sizeof(resp_buffer));
  bytes_sent = send(client_socket_fd, resp_buffer, resp_bytes, 0);

  // Close connection
  close(client_socket_fd);
}

int main() {
  // Create server
  printf("Starting server...\n");
  int server_socket_fd = create_server_socket(PORT);
  printf("Server listening on port %s\n", PORT);

  // Listen and accept connections
  while (1) {
    accept_client(server_socket_fd, "./public");
  }
}
