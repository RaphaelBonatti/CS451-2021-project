#include <best_effort_broadcast.h>

struct sockaddr_in *addresses;
size_t n_addresses;

void beb_init(struct sockaddr_in *addrs, size_t n) {
  addresses = addrs;
  n_addresses = n;
}

void beb_broadcast(int sock_fd, const char *message) {
  for (size_t i = 0; i < n_addresses; ++i) {
    // printf("pl send %s to %lu\n", message, i + 1);
    pl_send(sock_fd, &(addresses[i]), message);
  }
}

void beb_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                 socklen_t *sender_len, char *message) {
  pl_deliver(sock_fd, sender_addr, sender_len, message);
  // printf("pl deliver %s from %u\n", message, htons(sender_addr->sin_port));
}
