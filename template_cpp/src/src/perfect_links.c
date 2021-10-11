#include <stdio.h>
#include <string.h>
#include <time.h>

#include "perfect_links.h"
#include "stubborn_links.h"

#define N_PACKETS 8192

struct Packet delivered[N_PACKETS];
size_t n_packets;

// TODO: will you use it?
// needed to use nanosleep
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);

int packet_exists(const char *message, struct sockaddr_in *sender_addr) {
  for (size_t i = 0; i < n_packets; ++i) {
    if (strcmp(message, delivered[i].message) == 0 &&
        sender_addr->sin_addr.s_addr ==
            delivered[i].sender_addr.sin_addr.s_addr &&
        sender_addr->sin_port == delivered[i].sender_addr.sin_port) {
      return 1;
    }
  }
  return 0;
}

void pl_init() { n_packets = 0; }

void pl_send(int sock_fd, struct sockaddr_in *receiver_addr,
             const char *message) {
  sl_send(sock_fd, receiver_addr, message);

  //   struct timespec ts;
  //   int res = 0;
  //   int msec = 500;
  //   ts.tv_sec = msec / 1000;
  //   ts.tv_nsec = (msec % 1000) * 1000000;
  //   res = nanosleep(&ts, &ts);
}

void pl_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                socklen_t *sender_len, char *message) {
  do {
    sl_deliver(sock_fd, sender_addr, sender_len, message);
  } while (packet_exists(message, sender_addr));
  // Deep copy needed for char*
  strncpy(delivered[n_packets].message, message, strlen(message));
  delivered[n_packets].sender_addr.sin_addr.s_addr =
      sender_addr->sin_addr.s_addr;
  delivered[n_packets].sender_addr.sin_port = sender_addr->sin_port;
  ++n_packets;
}