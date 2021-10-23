#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "perfect_links.h"
#include "stubborn_links.h"

#define MAX_PACKETS 100
#define MAX_BUCKETS 10
#define MAX_PROCESSES 128
#define MAX_CHARS 256
#define INIT_ACK_SIZE 1048576 // 2^20 Bytes

struct AckTable {
  bool *acks;
  size_t size;
};

// keep a boolean for each message of each process represented by its seq_num
struct AckTable ack_table[MAX_PROCESSES];

bool packet_exists(struct sockaddr_in *sender_addr, uint seq_num) {
  // TODO: generalize the way to get process_num
  // Determine the bucket that the port hashes to.
  size_t process_num = ntohs(sender_addr->sin_port) % MAX_PROCESSES;
  return ack_table[process_num].acks[seq_num];
}

void pl_init() {
  for (size_t i = 0; i < MAX_PROCESSES; ++i) {
    ack_table[i].acks = calloc(INIT_ACK_SIZE, sizeof(bool));
    ack_table[i].size = INIT_ACK_SIZE;
  }
}

void pl_destroy() {
  for (size_t i = 0; i < MAX_PROCESSES; ++i) {
    free(ack_table[i].acks);
  }
}

void pl_realloc(size_t process_num) {
  printf("Reallocating!\n");
  size_t new_size = 2 * ack_table[process_num].size;
  bool *prev = ack_table[process_num].acks;
  ack_table[process_num].acks =
      realloc(ack_table[process_num].acks, new_size * sizeof(bool));

  if (!(ack_table[process_num].acks)) {
    free(prev);
    exit(1);
  }

  ack_table[process_num].size = new_size;
}

void pl_send(int sock_fd, struct sockaddr_in *receiver_addr,
             const char *message) {
  // Concatenate seq_num and message
  static uint seq_num = 0;
  char buffer[MAX_CHARS] = {0};
  // TODO: Maybe useful to check if it went well
  snprintf(buffer, MAX_CHARS, "%u|%s", seq_num, message);

  // Send the new message
  sl_send(sock_fd, receiver_addr, buffer);
  ++seq_num;
}

void pl_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                socklen_t *sender_len, char *message) {
  static uint seq_num = 0;

  do {
    sl_deliver(sock_fd, sender_addr, sender_len, message);
    sscanf(message, "%u|%s", &seq_num, message);
  } while (packet_exists(sender_addr, seq_num));

  size_t process_num = ntohs(sender_addr->sin_port) % MAX_PROCESSES;

  // Check size of the array
  if (seq_num >= ack_table[process_num].size) {
    pl_realloc(process_num);
  }

  // Ack the packet
  ack_table[process_num].acks[seq_num] = true;

  // // will return the message and sender_addr (and its length)
}