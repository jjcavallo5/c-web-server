#include "router.h"
#include "http.h"
#include "server.h"
#include <netdb.h>
#include <string.h>

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

void router_static(Router *router, const char *root) {}

void router_dispatch(Router *router, HTTPRequest *req, HTTPResponse *res) {
  for (int i = 0; i < router->_num_routes; i++) {
    if (req->method == router->routes[i].method) {
      if (strcmp(req->path, router->routes[i].path) == 0) {
        router->routes[i].handler(req, res);
      }
    }
  }
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

// int main() {
//   Router app = {.host = "127.0.0.1", .port = "8080"};
//
//   router_get(&app, "/index.html", get_handler);
//
//   router_run(&app);
// }
