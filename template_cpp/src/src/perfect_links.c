#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "perfect_links.h"
#include "stubborn_links.h"

#define MAX_PROCESSES 128
#define MAX_PORTS 1000
#define INIT_ACK_SIZE 1048576 // 2^20 Bytes

struct AckTable {
  bool *acks;
  size_t size;
  uint n_ack;
};

// keep a boolean for each message of each process represented by its seq_num
static struct AckTable ack_table[MAX_PROCESSES];
static uint seq_num_table[MAX_PROCESSES];

uint find_process_id(struct sockaddr_in addr) {
  static uint n_process = 1;
  static uint processTable[MAX_PROCESSES][MAX_PORTS] = {0};
  uint addr_hash = addr.sin_addr.s_addr % MAX_PROCESSES;
  uint port_hash = addr.sin_port % MAX_PORTS;

  uint id = processTable[addr_hash][port_hash];

  if (id != 0) {
    return id;
  }

  processTable[addr_hash][port_hash] = n_process;
  ++n_process;

  return n_process - 1;
}

void pl_init(int sock_fd) {
  for (size_t i = 0; i < MAX_PROCESSES; ++i) {
    ack_table[i].acks = calloc(INIT_ACK_SIZE, sizeof(bool));
    ack_table[i].size = INIT_ACK_SIZE;
    ack_table[i].n_ack = 0;
    seq_num_table[i] = 0;
  }

  printf("pl_init sock_fd: %d\n", sock_fd);

  sl_init(sock_fd);
}

void pl_destroy() {
  for (size_t i = 0; i < MAX_PROCESSES; ++i) {
    free(ack_table[i].acks);
  }

  sl_destroy();
}

void pl_realloc(size_t process_index) {
  printf("Reallocating!\n");
  size_t new_size = 2 * ack_table[process_index].size;
  bool *prev = ack_table[process_index].acks;
  ack_table[process_index].acks =
      realloc(ack_table[process_index].acks, new_size * sizeof(bool));

  if (!(ack_table[process_index].acks)) {
    free(prev);
    exit(1);
  }

  ack_table[process_index].size = new_size;
}

void pl_send(int sock_fd, struct sockaddr_in receiver_addr,
             const char *message) {
  uint process_index = find_process_id(receiver_addr) - 1;
  uint seq_num = seq_num_table[process_index];

  char buffer[MAX_HEADER_SIZE + MAX_CHARS + 1] = {0};
  char block[MAX_CHARS + 1] = {0};
  const char *cursor = message;
  // At string terminator
  const char *limit = message + strlen(message);
  uintptr_t chars_left = 0;

  // Break message into blocks to send arbitrary long messages
  while (cursor < limit) {
    if (limit > cursor + MAX_CHARS) {
      memcpy(block, cursor, MAX_CHARS);
      cursor += MAX_CHARS;
    } else {
      block[limit - cursor] = '\0';
      chars_left = (uintptr_t)limit - (uintptr_t)cursor;
      memcpy(block, cursor, chars_left);
      cursor = limit;
    }

    // Concatenate seq_num and message
    // TODO: Maybe useful to check if it went well
    snprintf(buffer, MAX_HEADER_SIZE + MAX_CHARS + 1, "m|%u|%s", seq_num,
             block);

    // Send the new message
    sl_send(sock_fd, receiver_addr, buffer);
    ++seq_num_table[process_index];
  }
}

static void print_ack_table() {
  for (uint j = 0; j < 4; ++j) {
    printf("table [%u] : ", j + 1);
    for (uint i = 0; i < 7; ++i) {
      printf("%u | ", ack_table[j].acks[i]);
    }
    printf("\n");
  }
}

void pl_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                socklen_t *sender_len, char *message) {
  char type = '\0';
  uint seq_num = 0;
  uint process_index = 0;

  // sl_deliver and check if the message was already delivered (i.e. same
  // sender_addr and seq_num)
  do {
    sl_deliver(sock_fd, sender_addr, sender_len, message);

    // Decode the packet: type | seq | message
    sscanf(message, "%c|%u|%s", &type, &seq_num, message);

    // only care if it's a message type
    if (type == 'm') {
      process_index = find_process_id(*sender_addr) - 1;
    }
  } while (type != 'm' || ack_table[process_index].acks[seq_num]);

  // Check size of the array
  if (ack_table[process_index].n_ack >= ack_table[process_index].size) {
    pl_realloc(process_index);
  }

  // Ack the packet
  ack_table[process_index].acks[seq_num] = true;
  ack_table[process_index].n_ack += 1;

  // print_ack_table();

  // will return the message and sender_addr (and its length)
}