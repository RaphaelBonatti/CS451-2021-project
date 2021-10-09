#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "io_handler.h"

void run(struct sockaddr_in receiver_addr, int sock_fd,
         struct ConfigInfo configInfo, char *events, unsigned long process_id);

#ifdef __cplusplus
}
#endif