#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "perfect_links.h"

#include <stdio.h>

void beb_init(struct sockaddr_in *addrs, size_t n);
void beb_broadcast(int sock_fd, const char *message);
void beb_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                 socklen_t *sender_len, char *message);

#ifdef __cplusplus
}
#endif
