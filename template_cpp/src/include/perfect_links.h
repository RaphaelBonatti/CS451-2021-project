#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stubborn_links.h"

struct Packet {
  char message[MESSAGE_SIZE];
  struct sockaddr_in sender_addr;
};

int packet_exists(const char *message, struct sockaddr_in *sender_addr);
void pl_init();
void pl_send(int sock_fd, struct sockaddr_in *receiver_addr,
             const char *message);
void pl_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                socklen_t *sender_len, char *message);

#ifdef __cplusplus
}
#endif