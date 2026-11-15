#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
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

  if (bind(server_socket, result->ai_addr, result->ai_addrlen)) {
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

int accept_client(int server_socket_fd) {
  // Accept Client
  struct sockaddr_storage client;
  socklen_t client_size = sizeof(client);
  int client_fd =
      accept(server_socket_fd, (struct sockaddr *)&client, &client_size);

  // Get connection info
  char host[1024];
  char port[64];
  if (getnameinfo((struct sockaddr *)&client, client_size, host, sizeof(host),
                  port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV)) {
    perror("getnameinfo");
    exit(EXIT_FAILURE);
  }
  printf("New connection: %s:%s\n", host, port);

  // Return client
  return client_fd;
}

void serve_client(int client_fd, char *serve_from, char *raw_request) {
  // Parse HTTP method
  struct HTTPRequest request;
  memset(&request, 0, sizeof(request));
  parse_http_request(raw_request, &request);
  char serve_file_path[4096];
  snprintf(serve_file_path, sizeof(serve_file_path), "%s%s", serve_from,
           request.path);

  // Create Response Body
  char resp_buffer[8192];
  memset(&resp_buffer, 0, sizeof(resp_buffer));
  ssize_t resp_bytes =
      read_file(serve_file_path, resp_buffer, sizeof(resp_buffer));

  // Send Response Headers
  char header_buffer[4096];
  const char *content_type = get_content_type(request.extension);
  create_response_headers(header_buffer, sizeof(header_buffer), content_type,
                          resp_bytes);
  send(client_fd, header_buffer, strlen(header_buffer), 0);

  // Send Response Body
  send(client_fd, resp_buffer, resp_bytes, 0);
}

void close_connection(int fd, struct pollfd *pfds, nfds_t num_open_fds) {
  nfds_t fd_idx = -1;
  for (nfds_t i = 0; i < num_open_fds; i++) {
    if (pfds[i].fd == fd)
      fd_idx = i;
  }

  if (fd_idx == -1) {
    printf("WARNING: Failed to close file %d\n", fd);
    return;
  }

  // Close file descriptor
  if (close(fd)) {
    perror("close");
  }

  // Handle pfds array
  for (nfds_t j = fd_idx; j < num_open_fds - 1; j++) {
    pfds[j] = pfds[j + 1];
  }

  printf("Connection to fd %d closed.\n", fd);
}

int main(void) {
  const char *serve_from = "./public";

  // Create server
  printf("Starting server...\n");
  int server_socket_fd = create_server_socket(PORT);
  printf("Server listening on port %s\n", PORT);

  // Set up multiplexing
  nfds_t num_open_fds = 1;
  struct pollfd pfds[32 + 1]; // Accept up to 32 clients (maybe?)
  pfds[0].fd = server_socket_fd;
  pfds[0].events = POLLIN;

  // Listen and accept connections
  while (poll(pfds, num_open_fds, -1)) {
    for (nfds_t i = 0; i < num_open_fds; i++) {
      // Listening socket, new connection
      if (i == 0 && (pfds[i].revents & POLLIN) && num_open_fds < 33) {
        pfds[num_open_fds].fd = accept_client(server_socket_fd);
        pfds[num_open_fds].events = POLLIN;
        num_open_fds++;
      }

      // Client socket, read available
      if (i > 0 && (pfds[i].revents & POLLIN)) {
        char request_buffer[8192];
        ssize_t bytes_recieved =
            recv(pfds[i].fd, &request_buffer, sizeof(request_buffer) - 1, 0);
        if (bytes_recieved > 0) {
          request_buffer[bytes_recieved] = '\n';
          serve_client(pfds[i].fd, (char *)serve_from, request_buffer);
        } else {
          close_connection(pfds[i].fd, pfds, num_open_fds--);
        }
      }

      // Client socket, possible hangup
      else if (pfds[i].revents & POLLHUP) {
        close_connection(pfds[i].fd, pfds, num_open_fds--);
      }
    }
  }
}
