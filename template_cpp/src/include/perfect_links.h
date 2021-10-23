#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "stubborn_links.h"

typedef unsigned int uint;

struct Packet {
  uint seq_num;
  struct sockaddr_in sender_addr;
};

bool packet_exists(struct sockaddr_in *sender_addr, uint seq_num);
void pl_init();
void pl_destroy();
void pl_realloc(size_t process_num);
void pl_send(int sock_fd, struct sockaddr_in *receiver_addr,
             const char *message);
void pl_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                socklen_t *sender_len, char *message);

#ifdef __cplusplus
}
#endif