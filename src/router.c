#include "http.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROUTES 32

typedef enum RouterErrors {
  ROUTER_MAXRTS,
} RouterErrors;

typedef void (*HTTPHandler)(struct HTTPRequest *);

typedef struct Route {
  enum HTTPMethod method;
  char path[4096];
  HTTPHandler handler;
} Route;

typedef struct Router {
  char host[NI_MAXHOST];
  char port[NI_MAXSERV];
  Route routes[MAX_ROUTES];
  int _num_routes;
} Router;

void get_handler(struct HTTPRequest *req) {
  printf("GET: Handled route: %s\n", req->path);
}

void post_handler(struct HTTPRequest *req) {
  printf("POST: Handled route: %s\n", req->path);
}

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

void router_put(Router *router, const char *route, HTTPHandler handler) {}

void router_static(Router *router, const char *root) {}

void router_dispatch(Router *router, struct HTTPRequest *req) {
  for (int i = 0; i < router->_num_routes; i++) {
    if (req->method == router->routes[i].method) {
      if (strcmp(req->path, router->routes[i].path) == 0) {
        router->routes[i].handler(req);
      }
    }
  }
}

void router_start(Router *router) {}

int main() {
  Router app = {.host = "127.0.0.1", .port = "8080"};

  router_get(&app, "/index.html", get_handler);
  router_post(&app, "/index.html", post_handler);

  struct HTTPRequest req = {
      .path = "/index.html", .extension = "html", .method = POST};

  router_dispatch(&app, &req);
}
