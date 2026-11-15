#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
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

#include "http.h"

#define PORT "8080"

int create_server_socket(char *port) {
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  struct addrinfo *result;
  if (getaddrinfo(NULL, port, &hints, &result) != 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  int server_socket =
      socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);

  if (server_socket <= 0) {
    printf("Failed to create socket\n");
    exit(EXIT_FAILURE);
  }

  int enable = 1;
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &enable,
                 sizeof(enable)) != 0) {
    perror("setsockopt");
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

ssize_t read_file(const char *file_path, char *file_buffer,
                  size_t buffer_size) {
  // Read file
  int fd = open(file_path, O_RDONLY);
  ssize_t bytes_read = read(fd, file_buffer, buffer_size);
  file_buffer[bytes_read] = '\0';
  close(fd);
  return bytes_read;
}

void accept_client(int server_socket_fd, char *serve_from) {
  // Listen
  struct sockaddr client;
  char buff[8192];

  socklen_t client_size = sizeof(client);
  int client_socket_fd = accept(server_socket_fd, &client, &client_size);
  ssize_t bytes_recieved = recv(client_socket_fd, &buff, sizeof(buff) - 1, 0);
  if (bytes_recieved > 0) {
    buff[bytes_recieved] = '\0';
  }
  printf("Message recieved: \n\n%s\n", buff);

  // Parse HTTP method
  struct HTTPRequest request;
  memset(&request, 0, sizeof(request));
  parse_http_request(buff, &request);
  char serve_file_path[4096];
  snprintf(serve_file_path, sizeof(serve_file_path), "%s%s", serve_from,
           request.path);

  // Send Response Headers
  char header_buffer[4096];
  const char *content_type = get_content_type(request.extension);
  create_response_headers(header_buffer, sizeof(header_buffer), content_type);
  send(client_socket_fd, header_buffer, strlen(header_buffer), 0);

  // Send Response Body
  char resp_buffer[8192];
  memset(&resp_buffer, 0, sizeof(resp_buffer));
  ssize_t resp_bytes =
      read_file(serve_file_path, resp_buffer, sizeof(resp_buffer));
  send(client_socket_fd, resp_buffer, resp_bytes, 0);

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
