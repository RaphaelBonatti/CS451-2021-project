#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "io_handler.h"
#include "network_handler.h"

#define MESSAGE_SIZE 256
#define EVENT_SIZE 512

size_t get_id_by_sockaddr(struct sockaddr_in *sockaddr,
                          struct ProcessInfo *processInfos, size_t n_process) {
  for (size_t i = 0; i < n_process; ++i) {

    // printf("s_addr: %u, process_addr: %u, sin_port: %u, process_port: %u \n",
    //        sockaddr->sin_addr.s_addr, processInfos[i].ip, sockaddr->sin_port,
    //        processInfos[i].port);

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
      // printf("s_addr: %u, process_addr: %u, sin_port: %u, process_port: %u
      // \n",
      //        sockaddr->sin_addr.s_addr, processInfos[i].ip,
      //        sockaddr->sin_port, processInfos[i].port);
      break;
    }
  }
}

void run(int sock_fd, struct ConfigInfo configInfo, char *events,
         size_t process_id, struct ProcessInfo *processInfos,
         size_t n_process) {
  // printf("Inside run.\n");
  char event[EVENT_SIZE] = {0};
  char buffer[MESSAGE_SIZE] = {0};
  struct sockaddr_in process_addr;

  get_sockaddr_by_id(&process_addr, process_id, processInfos, n_process);

  // TODO: check bind failure
  int bind_check = bind(sock_fd, (const struct sockaddr *)&process_addr,
                        sizeof(process_addr));

  // printf("Check sock failure.\n");
  // TODO: check sock failure

  if (is_receiver(configInfo.receiver_id, process_id)) {
    // TODO: listen to messages
    struct sockaddr_in sender_addr;
    size_t sender_id = 0;

    socklen_t sender_len = sizeof(sender_addr);

    // TODO: use while true?
    while (1) {
      // printf("Receiving...\n");
      ssize_t n = recvfrom(sock_fd, buffer, MESSAGE_SIZE, 0,
                           (struct sockaddr *)&sender_addr, &sender_len);

      sender_id = get_id_by_sockaddr(&sender_addr, processInfos, n_process);

      // Received
      // printf("buffer : '%s'\n", buffer);
      snprintf(event, EVENT_SIZE, "d %lu %s\n", sender_id, buffer);

      // Reset buffer to avoid left over chars
      memset(buffer, 0, MESSAGE_SIZE);
      // printf("log: '%s'\n", event);
      log_events(events, event);
    }

  } else {
    // TODO: starts at 1?
    size_t seq_n = 1;
    struct sockaddr_in receiver_addr;

    get_sockaddr_by_id(&receiver_addr, configInfo.receiver_id, processInfos,
                       n_process);

    for (size_t i = 0; i < configInfo.n_messages; i++) {
      // printf("Sending...\n");

      snprintf(buffer, MESSAGE_SIZE, "%lu", seq_n);
      socklen_t receiver_len = sizeof(receiver_addr);
      ssize_t send_check =
          sendto(sock_fd, buffer, strlen(buffer), 0,
                 (struct sockaddr *)&receiver_addr, receiver_len);
      // TODO: check send failure

      // printf("sent: %ld bytes\n", send_check);
      snprintf(event, EVENT_SIZE, "b %lu\n", seq_n);
      // Sent
      log_events(events, event);
      ++seq_n;
    }
  }
}