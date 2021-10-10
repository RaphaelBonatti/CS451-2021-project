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

size_t find_id(struct sockaddr_in *addr, struct ProcessInfo *processInfos,
               size_t n_process) {
  for (size_t i = 0; i < n_process; i++) {
    if (addr->sin_addr.s_addr == processInfos[i].ip &&
        addr->sin_port == processInfos[i].port) {
      return processInfos[i].id;
    }
  }

  return 0;
}

void run(struct sockaddr_in receiver_addr, int sock_fd,
         struct ConfigInfo configInfo, char *events, size_t process_id,
         struct ProcessInfo *processInfos, size_t n_process) {
  printf("Inside run.\n");
  char event[EVENT_SIZE] = {0};
  char buffer[MESSAGE_SIZE] = {0};
  struct sockaddr_in sender_addr;

  printf("Check sock failure.\n");
  // TODO: check sock failure

  if (is_receiver(configInfo.receiver_id, process_id)) {
    // TODO: listen to messages
    size_t sender_id = 0;

    int bind_check = bind(sock_fd, (const struct sockaddr *)&receiver_addr,
                          sizeof(receiver_addr));
    // TODO: check bind failure

    socklen_t sender_len = sizeof(sender_addr);

    // TODO: use while true?
    while (1) {
      printf("Receiving...\n");
      ssize_t n = recvfrom(sock_fd, buffer, MESSAGE_SIZE, 0,
                           (struct sockaddr *)&sender_addr, &sender_len);

      sender_id = find_id(&sender_addr, processInfos, n_process);

      // Received
      printf("buffer : '%s'\n", buffer);
      snprintf(event, EVENT_SIZE, "d %lu %s\n", sender_id, buffer);
      printf("log: '%s'\n", event);
      log_events(events, event);
    }

  } else {
    // TODO: starts at 1?
    size_t seq_n = 1;

    for (size_t i = 0; i < configInfo.n_messages; i++) {
      printf("Sending...\n");

      snprintf(buffer, MESSAGE_SIZE, "%lu", seq_n);
      socklen_t receiver_len = sizeof(receiver_addr);
      ssize_t send_check =
          sendto(sock_fd, buffer, strlen(buffer), 0,
                 (struct sockaddr *)&receiver_addr, receiver_len);
      // TODO: check send failure

      printf("sent: %ld bytes\n", send_check);
      snprintf(event, EVENT_SIZE, "b %lu\n", seq_n);
      // Sent
      log_events(events, event);
      ++seq_n;
    }
  }
}