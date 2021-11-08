#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <stubborn_links.h>

#define MAX_ACK_SIZE 15

bool acked = 0;
uint ack_seq_num = 0; // seq_num to ack
in_port_t ack_sin_port;

void sl_send(int sock_fd, struct sockaddr_in *receiver_addr,
             const char *message) {
  // Get the seq_num to ack
  sscanf(message, "m|%u|", &ack_seq_num);

  // Get the port to ack
  ack_sin_port = receiver_addr->sin_port;

  // Block code until ack is received
  acked = 0;

  do {
    // TODO: check send failure?
    ssize_t send_check =
        sendto(sock_fd, message, strlen(message), 0,
               (struct sockaddr *)receiver_addr, sizeof(*receiver_addr));
  } while (!acked);
}

void sl_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                socklen_t *sender_len, char *message) {
  // Clean message to avoid left over chars
  memset(message, 0, MAX_CHARS);

  if (recvfrom(sock_fd, message, MAX_CHARS, 0, (struct sockaddr *)sender_addr,
               sender_len) <= 0) {
    fprintf(stderr, "Error, recvfrom failed. errno = %d\n", errno);
  } else {
    char type = '\0';
    uint seq_num = 0;

    // Decode packet: type | seq
    sscanf(message, "%c|%u|", &type, &seq_num);

    if (type == 'a' && ack_seq_num == seq_num &&
        sender_addr->sin_port == ack_sin_port && acked == 0) {
      // Not absolutely necessary to reset shared variable, but avoid that we
      // enter again just after setting acked to 0, which enables acked and we
      // may loose a packet.
      ack_sin_port = 0;
      ack_seq_num = 0;
      acked = 1;
    } else if (type == 'm') {
      char ack_buffer[MAX_ACK_SIZE] = {0};

      // ack the received message
      snprintf(ack_buffer, MAX_ACK_SIZE, "a|%u", seq_num);

      ssize_t send_check =
          sendto(sock_fd, ack_buffer, strlen(ack_buffer), 0,
                 (struct sockaddr *)sender_addr, sizeof(*sender_addr));
    }
  }
}