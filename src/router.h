#ifndef ROUTER_H
#define ROUTER_H

#include "http.h"
#include <linux/limits.h>
#include <netdb.h>

#define MAX_ROUTES 32

typedef enum RouterErrors {
  ROUTER_MAXRTS,
} RouterErrors;

typedef void (*HTTPHandler)(HTTPRequest *, HTTPResponse *);

typedef struct Route {
  enum HTTPMethod method;
  char path[4096];
  HTTPHandler handler;
} Route;

typedef struct Router {
  char host[NI_MAXHOST];
  char port[NI_MAXSERV];
  Route routes[MAX_ROUTES];
  char static_path[PATH_MAX];
  int _num_routes;
} Router;

int router_get(Router *router, const char *path, HTTPHandler handler);
int router_post(Router *router, const char *path, HTTPHandler handler);
int router_put(Router *router, const char *path, HTTPHandler handler);
int router_delete(Router *router, const char *path, HTTPHandler handler);
void router_static(Router *router, const char *root);
void router_run(Router *router);

#endif /* ROUTER_H */
