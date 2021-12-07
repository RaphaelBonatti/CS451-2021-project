#include "best_effort_broadcast.h"
#include "fifo_broadcast_application.h"

static struct sockaddr_in *addresses;
static size_t n_addresses;

void beb_init(int sock_fd, struct sockaddr_in *addrs, size_t n) {
  addresses = addrs;
  n_addresses = n;

  pl_init(sock_fd);
}

void beb_destroy() { pl_destroy(); }

void beb_broadcast(int sock_fd, const char *message, size_t n) {
  for (size_t i = 0; i < n_addresses; ++i) {
    pl_send(sock_fd, addresses[i], message, n);
  }
}

size_t beb_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                 socklen_t *sender_len, char *message) {
  return pl_deliver(sock_fd, sender_addr, sender_len, message);
}
