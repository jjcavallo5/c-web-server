#include "../src/http.h"
#include "../src/router.h"

#include <stdio.h>

void test_handler(HTTPRequest *req, HTTPResponse *resp) {
  printf("%s\n", req->path);
}

int main() {
  Router app = {"127.0.0.1", "8080"};
  router_get(&app, "/test", test_handler);
}
