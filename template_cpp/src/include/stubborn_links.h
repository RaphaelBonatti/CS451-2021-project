#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MESSAGE_SIZE 256
// type + sparator + max uint digits + separator
#define MAX_HEADER_SIZE 13
// max chars (without string terminator)
#define MAX_CHARS 256
#define N_SEND 1000

typedef unsigned int uint;

void sl_send(int sock_fd, struct sockaddr_in *receiver_addr,
             const char *message);
void sl_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                socklen_t *sender_len, char *message);

#ifdef __cplusplus
}
#endif