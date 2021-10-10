#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "io_handler.h"
size_t find_id(struct sockaddr_in *addr, struct ProcessInfo *processInfos,
               size_t n_process);
void run(struct sockaddr_in receiver_addr, int sock_fd,
         struct ConfigInfo configInfo, char *events, size_t process_id,
         struct ProcessInfo *processInfos, size_t n_process);

#ifdef __cplusplus
}
#endif