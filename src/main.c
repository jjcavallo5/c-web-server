#include "router.h"
#include <stdio.h>
#include <string.h>

void get_handler(HTTPRequest *req, HTTPResponse *res) {
  printf("Handling Request: %s\n", req->path);
  res->status = OK;
  create_response_headers(&res->headers, 11, "text/plain");
  strcpy(res->body, "It worked!\n");
}

int main(void) {
  Router app = {.host = "127.0.0.1", .port = "8080"};

  router_get(&app, "/endpoint", get_handler);

  router_static(&app, "./public");
  router_run(&app);
}
