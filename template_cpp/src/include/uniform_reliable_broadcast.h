#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "best_effort_broadcast.h"

struct PAM {
  bool delivered;
  uint sum;
  bool ack[MAX_PROCESS];   // TODO: dynamic
  char content[MAX_CHARS]; // TODO: dynamic
};

struct PAM *pam_table[MAX_PROCESS]; // TODO: Find another way to pass in order

struct PAM **pass_table();
void urb_init(int sock_fd, size_t process_id, struct sockaddr_in *addrs,
              size_t n_processes);
void urb_destroy();
void urb_broadcast(int sock_fd, const char *message);
void check_pam_table(size_t os, uint num);
void pam_alloc(size_t s, size_t os, uint num, const char *content);
void print_pam_table();
void urb_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                 socklen_t *sender_len, char *message);

#ifdef __cplusplus
}
#endif