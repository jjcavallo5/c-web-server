#ifndef ROUTER_H
#define ROUTER_H

#include "http.h"
#include <netdb.h>

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

#endif /* ROUTER_H */
