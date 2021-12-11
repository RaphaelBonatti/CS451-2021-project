#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "common.h"

#define MESSAGE_SIZE 256
// type + sparator + max uint digits + separator
#define MAX_HEADER_SIZE 13
// max chars (without string terminator)
#define MAX_CHARS 256
#define N_SEND 1000
#define MAX_ACK_SIZE sizeof(char) + sizeof(uint)
#define SHIFT_MODULO 120

typedef unsigned int uint;

void *sl_send_forever(void *_args);
void sl_init(int sock_fd);
void sl_destroy();
uint get_index(struct sockaddr_in addr);
void sl_send(int sock_fd, struct sockaddr_in receiver_addr, const char *message,
             size_t n, uint num);
size_t sl_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                  socklen_t *sender_len, char *message);

#ifdef __cplusplus
}
#endif