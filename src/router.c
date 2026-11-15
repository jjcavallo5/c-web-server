#include "http.h"
#include <netdb.h>
#include <stdio.h>

typedef struct Router {
  char host[NI_MAXHOST];
  char port[NI_MAXSERV];
  void (*route)(struct HTTPRequest *);
} Router;

void handler(struct HTTPRequest *req) {
  printf("Handled route: %s", req->path);
}

void router_start(Router *router) {}

int main() {
  Router app = {.host = "127.0.0.1", .port = "8080", handler};

  struct HTTPRequest req = {
      .path = "/index.html", .extension = "html", .method = GET};

  app.route(&req);
}
