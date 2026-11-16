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

#include "router.h"
#include "server.h"

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

void close_connection(int fd, struct pollfd *pfds, nfds_t num_open_fds) {
  int fd_idx = -1;
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

int run_http_server(Router *router, TcpCallback callback) {
  // Create server
  printf("Starting server...\n");
  int server_socket_fd = create_server_socket(router->port);
  printf("Server listening on port %s\n", router->port);

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
        char out_buffer[8192];
        ssize_t bytes_recieved =
            recv(pfds[i].fd, &request_buffer, sizeof(request_buffer) - 1, 0);
        if (bytes_recieved > 0) {
          request_buffer[bytes_recieved] = '\0';

          callback(router, request_buffer, out_buffer);

          send(pfds[i].fd, out_buffer, strlen(out_buffer), 0);
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

  return 0;
}
