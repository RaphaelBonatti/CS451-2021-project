#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "uniform_reliable_broadcast.h"

void lcb_init(int sock_fd, size_t _process_id, struct sockaddr_in *addrs,
              size_t _n_processes, struct LCAUSALConfigInfo *_config_info);
void lcb_destroy();
void lcb_broadcast(int sock_fd, const char *message, size_t n);
bool check_local_causality(uint *vec, size_t id);
void lcb_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                 socklen_t *sender_len, char *message);

#ifdef __cplusplus
}
#endif