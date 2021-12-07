#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "best_effort_broadcast.h"

struct PAM {
  bool delivered; // True if it can be urb delivered to the upper layer
  uint sum; // how many processes beb delivered this packet, optimization to avoid traversing the array
  bool ack[MAX_PROCESS];   // TODO: dynamic, which process acked the packet
  char content[MAX_CHARS]; // TODO: dynamic, content of the packet
};

struct PAM **pass_table();
void urb_init(int sock_fd, size_t process_id, struct sockaddr_in *addrs,
              size_t n_processes);
void urb_destroy();
void urb_broadcast(int sock_fd, const char *message, size_t n);
void check_pam_table(size_t os, uint num);
void pam_alloc(size_t s, size_t os, uint num, const char *content);
void print_pam_table();
void urb_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                 socklen_t *sender_len, char *message);

#ifdef __cplusplus
}
#endif