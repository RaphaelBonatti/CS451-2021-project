#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 199309L
#include <time.h>

#include "linked_list.h"
#include "serializer.h"
#include "stubborn_links.h"

static struct List hashmap[MAX_PROCESSES];
static size_t n_packets = 0;
static struct sockaddr_in addresses[MAX_PROCESSES];
static uint largest_index = 0;
static pthread_t tSendForever = 0;
static bool send_forever = true;
static int sock_fd;

int nanosleep(const struct timespec *req, struct timespec *rem);

void *sl_send_forever(void *_args) {
  struct Packet *packet;

  struct timespec ts;
  ts.tv_sec = 0; // FIXME: the 1 sec is for debug reasons
  ts.tv_nsec = 1000000;

  while (send_forever) {
    for (size_t i = 0; i <= largest_index; ++i) {
      packet = hashmap[i].head;

      if (packet) {
        if (packet->ack) {
          deleteFirst(&hashmap[i]);
          // printList(&hashmap[i]);
          --n_packets;
        } else {
          ssize_t send_check =
              sendto(sock_fd, packet->message, packet->n, 0,
                     (struct sockaddr *)&addresses[i], sizeof(addresses[i]));

          if (send_check < 0) {
            // probably sock_fd has been closed so we can quit the while loop
            send_forever = false;
            continue;
          }
        }
      }
    }
    nanosleep(&ts, &ts);
  }

  pthread_exit(NULL);
  return NULL;
}

void sl_init(int _sock_fd) {
  sock_fd = _sock_fd;

  for (size_t i = 0; i < MAX_PROCESSES; ++i) {
    initList(&hashmap[i]);
    addresses[i].sin_port = 0;
    // addresses[i] = NULL;
  }

  largest_index = 0;
  send_forever = true;

  pthread_create(&tSendForever, NULL, sl_send_forever, NULL);
}

void sl_destroy() {
  send_forever = false;
  pthread_join(tSendForever, NULL);

  for (size_t i = 0; i <= largest_index; ++i) {
    while (!isEmpty(&hashmap[i])) {
      deleteFirst(&hashmap[i]);
    }
  }
}

uint get_index(struct sockaddr_in addr) {
  return (uint)(htons(addr.sin_port) - SHIFT_MODULO) % MAX_PROCESSES;
}

void sl_send(int sock_fd, struct sockaddr_in receiver_addr, const char *message,
             size_t n, uint num) {
  // char message_type = '\0';
  // uint num = 0; // seq_num to ack

  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 1100000;

  uint index = get_index(receiver_addr);

  if (addresses[index].sin_port == 0) {
    // copy address
    addresses[index].sin_addr.s_addr = receiver_addr.sin_addr.s_addr;
    addresses[index].sin_family = receiver_addr.sin_family;
    addresses[index].sin_port = receiver_addr.sin_port;
    if (index > largest_index) {
      largest_index = index;
    }
  }

  insertLast(&hashmap[index], num, false, message, n);

  ++n_packets;
  nanosleep(&ts, &ts);
}

size_t sl_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                  socklen_t *sender_len, char *message) {
  ssize_t n = 0;
  char type = '\0';
  uint seq_num = 0;

  while (type != 'm') {
    // Clean message to avoid left over chars
    memset(message, 0, MAX_CHARS);

    if ((n = recvfrom(sock_fd, message, MAX_CHARS, 0,
                      (struct sockaddr *)sender_addr, sender_len)) <= 0) {
      // fprintf(stderr, "Error, recvfrom failed. errno = %d\n", errno);
      pthread_exit(NULL);
    } else {

      // Decode packet: type | seq
      // sscanf(message, "%c|%u|", &type, &seq_num);
      deserialize(2, message, &type, sizeof(char), &seq_num, sizeof(uint));

      if (type == 'a') {
        uint index = get_index(*sender_addr);
        struct Packet *packet = hashmap[index].head;

        // Make sure that there is a packet to ack, that the seq_num is the same
        // and that it has not been acked
        if (packet && packet->num == seq_num && packet->ack == false) {
          packet->ack = true;
        }
      } else if (type == 'm') {
        char ack_buffer[MAX_ACK_SIZE] = {0};
        char message_type = 'a';

        // prepare ack message for received message.
        // Encode: type(a), seq_num
        serialize(2, ack_buffer, &message_type, sizeof(char), &seq_num,
                  sizeof(uint));

        // Send ack message
        ssize_t send_check =
            sendto(sock_fd, ack_buffer, MAX_ACK_SIZE, 0,
                   (struct sockaddr *)sender_addr, sizeof(*sender_addr));
      }
    }
  }

  return (size_t)n;
}