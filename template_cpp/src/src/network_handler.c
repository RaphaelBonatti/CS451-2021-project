
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "io_handler.h"
#include "network_handler.h"

#define MESSAGE_SIZE 256
#define EVENT_SIZE 256

void run(struct sockaddr_in receiver_addr, int sock_fd,
         struct ConfigInfo configInfo, char *events, unsigned long process_id) {
  printf("Inside run.");
  char event[EVENT_SIZE] = {0};
  // TODO: starts at 1?
  int seq_n = 1;
  char buffer[MESSAGE_SIZE];
  struct sockaddr_in sender_addr;

  printf("Check sock failure.");
  // TODO: check sock failure

  if (is_receiver(configInfo.receiver_id, process_id)) {
    // TODO: listen to messages
    int sender_id = 0;

    int bind_check = bind(sock_fd, (const struct sockaddr *)&receiver_addr,
                          sizeof(receiver_addr));
    // TODO: check bind failure

    socklen_t sender_len = sizeof(sender_addr);

    // TODO: use while true?
    while (1) {
      printf("Receiving...");
      ssize_t n = recvfrom(sock_fd, buffer, MESSAGE_SIZE, 0,
                           (struct sockaddr *)&sender_addr, &sender_len);

      ++seq_n;

      // Received
      snprintf(event, EVENT_SIZE, "d %d %d\n", sender_id, seq_n);
      log_events(events, event);
    }

  } else {
    // TODO: send messages, content is seq_n

    for (size_t i = 0; i < configInfo.n_messages; i++) {
      printf("Sending...");
      snprintf(event, EVENT_SIZE, "b %d\n", seq_n);
      socklen_t receiver_len = sizeof(receiver_addr);
      ssize_t send_check = sendto(sock_fd, event, strlen(event), 0,
                                  (struct sockaddr *)&sock_fd, receiver_len);
      // TODO: check send failure

      ++seq_n;
      // Sent
      log_events(events, event);
    }
  }
}