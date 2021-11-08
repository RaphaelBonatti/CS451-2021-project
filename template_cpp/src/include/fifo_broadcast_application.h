#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "io_handler.h"

struct ProcessInfo {
  size_t id;
  in_addr_t ip;
  in_port_t port;
};

struct SenderArgs {
  char *buffer;
  int sock_fd;
};

struct ReceiverArgs {
  char *buffer;
  int sock_fd;
};

void app_destroy();
size_t get_id_by_sockaddr(struct sockaddr_in *addr,
                          struct ProcessInfo *processInfos, size_t n_process);
void get_sockaddr_by_id(struct sockaddr_in *sockaddr, size_t id,
                        struct ProcessInfo *processInfos, size_t n_process);
void *run_receiver(void *_args);
void *run_sender(void *_args);
void run_receiver_sender(int sock_fd, size_t process_id, char *sender_buffer,
                         char *receiver_buffer);
void run(int *sock_fd, struct ConfigInfo *configInfo, size_t process_id,
         struct ProcessInfo *processInfos, size_t n_process);

#ifdef __cplusplus
}
#endif