#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

struct ProcessInfo {
  size_t id;
  in_addr_t ip;
  in_port_t port;
};

void app_init(const char *filename, const char *configpath, size_t process_id,
              struct ProcessInfo *processInfos, size_t n_process);
void app_destroy();
size_t get_id_by_sockaddr(struct sockaddr_in *addr,
                          struct ProcessInfo *processInfos, size_t n_process);
void get_sockaddr_by_id(struct sockaddr_in *sockaddr, size_t id,
                        struct ProcessInfo *processInfos, size_t n_process);
void *run_receiver(void *_args);
void run_sender();
void run_receiver_sender(size_t process_id);
void app_run();

#ifdef __cplusplus
}
#endif