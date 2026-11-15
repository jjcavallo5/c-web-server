#include "router.h"
#include "http.h"
#include "server.h"
#include <fcntl.h>
#include <linux/limits.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_ROUTES 32

int router_register(Router *router, enum HTTPMethod method, const char *path,
                    HTTPHandler handler) {
  if (router->_num_routes >= MAX_ROUTES) {
    return ROUTER_MAXRTS;
  }

  Route route = {.method = method, .handler = handler};
  strcpy(route.path, path);
  router->routes[router->_num_routes++] = route;

  return 0;
}

int router_get(Router *router, const char *path, HTTPHandler handler) {
  return router_register(router, GET, path, handler);
}

int router_post(Router *router, const char *path, HTTPHandler handler) {
  return router_register(router, POST, path, handler);
}

int router_put(Router *router, const char *path, HTTPHandler handler) {}

void serve_static(const char *file_path, HTTPRequest *req, HTTPResponse *res) {
  // Read file
  int fd = open(file_path, O_RDONLY);
  ssize_t bytes_read = read(fd, res->body, 8191);
  close(fd);
  res->body[bytes_read] = '\0';

  // Create headers
  create_response_headers(&res->headers, bytes_read,
                          get_content_type(req->extension));
}

void router_static(Router *router, const char *root) {
  realpath(root, router->static_path);
}

void router_dispatch(Router *router, HTTPRequest *req, HTTPResponse *res) {
  // Priority 1: User-defined endpoints
  for (int i = 0; i < router->_num_routes; i++) {
    if (req->method == router->routes[i].method) {
      if (strcmp(req->path, router->routes[i].path) == 0) {
        printf("Matched %s with %s\n", req->path, router->routes[i].path);
        router->routes[i].handler(req, res);
        return;
      }
    }
  }

  // Priority 2: Registered static files
  if (req->method == GET) {
    char resolved_path[PATH_MAX];
    snprintf(resolved_path, PATH_MAX, "%s%s", router->static_path, req->path);
    if (access(resolved_path, F_OK) == 0) {
      serve_static(resolved_path, req, res);
      return;
    }
  }

  // No match, 404
  send_404(req, res);
}

int server_interface(Router *router, char *in_buffer, ssize_t in_buffer_len,
                     char *out_buffer) {
  // Parse HTTP method
  struct HTTPRequest request;
  memset(&request, 0, sizeof(request));
  parse_http_request(in_buffer, &request);

  // Dispatch
  HTTPResponse response;
  router_dispatch(router, &request, &response);

  // Save output
  form_http_response(out_buffer, 8192, &response);

  return 0;
}

void router_run(Router *router) { run_http_server(router, server_interface); }
