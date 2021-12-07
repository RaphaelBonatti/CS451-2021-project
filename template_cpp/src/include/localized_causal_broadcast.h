#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "uniform_reliable_broadcast.h"

void lcb_init(int sock_fd, size_t _process_id, struct sockaddr_in *addrs,
              size_t _n_processes);
void lcb_destroy();
void serialize_vector_clock(const uint *vec);
void lcb_broadcast(int sock_fd, const char *message);
void deserialize_vector_clock(char *const chrs, uint *vec);
bool check_condition(uint* vec);
void lcb_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                 socklen_t *sender_len, char *message);

#ifdef __cplusplus
}
#endif