#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "io_handler.h"
#include "network_handler.h"
#include "perfect_links.h"

#define EVENT_SIZE 512
#define N_MESSAGES 256

size_t get_id_by_sockaddr(struct sockaddr_in *sockaddr,
                          struct ProcessInfo *processInfos, size_t n_process) {
  for (size_t i = 0; i < n_process; ++i) {
    if (sockaddr->sin_addr.s_addr == processInfos[i].ip &&
        sockaddr->sin_port == processInfos[i].port) {
      return processInfos[i].id;
    }
  }
  return 0;
}

void get_sockaddr_by_id(struct sockaddr_in *sockaddr, size_t id,
                        struct ProcessInfo *processInfos, size_t n_process) {
  // I make no assumption about id location in the array (i.e. less efficient,
  // but more agnostic towards config)
  for (size_t i = 0; i < n_process; ++i) {
    if (processInfos[i].id == id) {
      sockaddr->sin_family = AF_INET;
      sockaddr->sin_port = processInfos[i].port;
      sockaddr->sin_addr.s_addr = processInfos[i].ip;
      break;
    }
  }
}

void run_receiver(int sock_fd, char *events, struct ProcessInfo *processInfos,
                  size_t n_process, char *buffer, char *event) {
  struct sockaddr_in sender_addr;
  size_t sender_id = 0;
  // Important to give the size of the struct
  socklen_t sender_len = sizeof(sender_addr);

  while (1) {
    // TODO: check receive failure?
    pl_deliver(sock_fd, &sender_addr, &sender_len, buffer);
    sender_id = get_id_by_sockaddr(&sender_addr, processInfos, n_process);

    // Clean event to avoid left over chars
    memset(event, 0, EVENT_SIZE);

    snprintf(event, EVENT_SIZE, "d %lu %s\n", sender_id, buffer);
    log_events(events, event);
  }
}

void run_sender(int sock_fd, struct ConfigInfo *configInfo, char *events,
                struct ProcessInfo *processInfos, size_t n_process,
                char *buffer, char *event) {
  size_t seq_n = 1;
  struct sockaddr_in receiver_addr;
  // TODO: may be useful to use an array of acks
  // short acks[N_MESSAGES] = {0};

  get_sockaddr_by_id(&receiver_addr, configInfo->receiver_id, processInfos,
                     n_process);
  for (size_t i = 0; i < configInfo->n_messages; i++) {
    // Reset buffer and event to avoid left over chars
    memset(buffer, 0, MESSAGE_SIZE);
    memset(event, 0, EVENT_SIZE);

    snprintf(buffer, MESSAGE_SIZE, "%lu", seq_n);
    pl_send(sock_fd, &receiver_addr, buffer);
    snprintf(event, EVENT_SIZE, "b %lu\n", seq_n);
    log_events(events, event);
    ++seq_n;
  }
}

void run_receiver_sender(int sock_fd, struct ConfigInfo *configInfo,
                         char *events, size_t process_id,
                         struct ProcessInfo *processInfos, size_t n_process,
                         char *buffer, char *event) {
  if (is_receiver(configInfo->receiver_id, process_id)) {
    run_receiver(sock_fd, events, processInfos, n_process, buffer, event);
  } else {
    run_sender(sock_fd, configInfo, events, processInfos, n_process, buffer,
               event);
  }
}

void run(int *sock_fd, struct ConfigInfo *configInfo, char *events,
         size_t process_id, struct ProcessInfo *processInfos,
         size_t n_process) {
  char event[EVENT_SIZE] = {0};
  char buffer[MESSAGE_SIZE] = {0};
  struct sockaddr_in process_addr;

  get_sockaddr_by_id(&process_addr, process_id, processInfos, n_process);

  if ((*sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    fprintf(stderr, "Error, socket creation failed. errno = %d\n", errno);
    exit(1);
  }

  if (bind(*sock_fd, (const struct sockaddr *)&process_addr,
           sizeof(process_addr)) < 0) {
    fprintf(stderr, "Error, bind failed. errno = %d\n", errno);
    exit(1);
  }

  pl_init();

  run_receiver_sender(*sock_fd, configInfo, events, process_id, processInfos,
                      n_process, buffer, event);
}