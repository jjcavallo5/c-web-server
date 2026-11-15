#include <sys/types.h>
#ifndef SERVER_H
#define SERVER_H

#include "router.h"

typedef int (*TcpCallback)(Router *router, char *in_buffer,
                           ssize_t in_buffer_len, char *out_buffer);

int run_http_server(Router *router, TcpCallback callback);

#endif /* SERVER_H */
