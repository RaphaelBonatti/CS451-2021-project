#include <stdlib.h>
#include <string.h>

#include "uniform_reliable_broadcast.h"

#define INIT_TABLE_SIZE 8192

static uint pam_table_size[MAX_PROCESS] = {0};
static uint pam_table_n[MAX_PROCESS] = {0};
static size_t n_processes;
static size_t process_id;
static struct sockaddr_in *addresses;

struct PAM **pass_table() {
  return pam_table;
}

void urb_init(int sock_fd, size_t _process_id, struct sockaddr_in *addrs,
              size_t _n_processes) {
  process_id = _process_id;
  n_processes = _n_processes;
  addresses = addrs;

  for (uint i = 0; i <= _n_processes + 1; ++i) { // + 1 because of the 0
    pam_table[i] = calloc(INIT_TABLE_SIZE, sizeof(struct PAM));
    pam_table_size[i] = INIT_TABLE_SIZE;
    pam_table_n[i] = 0;
  }

  beb_init(sock_fd, addrs, n_processes);
}

void urb_destroy() {
  for (uint i = 0; i <= n_processes; ++i) {
    free(pam_table[i]);
  }

  beb_destroy();
}

void urb_broadcast(int sock_fd, const char *message) {
  char buffer[MAX_CHARS] = {0};
  static uint count = 0;

  in_port_t port = addresses[process_id - 1].sin_port;

  pam_alloc(process_id, process_id, count, message);
  snprintf(buffer, MAX_CHARS, "%d|%u|%s", port, count, message);
  beb_broadcast(sock_fd, buffer);
  ++count;
}

void check_pam_table(size_t os, uint num) {
  if (pam_table_size[os] < num) { // TODO: may need some adjustments
    struct PAM *prev = pam_table[os];

    /* Expand the available memory with realloc */
    pam_table[os] = realloc(pam_table[os], 2 * pam_table_size[os]);

    /* Check to see if we were successful */
    if (pam_table[os] == NULL) {
      /* We were not successful, so display a message */
      printf("Could not re-allocate required memory\n");
      free(prev);
      /* And exit */
      exit(1);
    }

    pam_table_size[os] *= 2;
  }
}

void pam_alloc(size_t s, size_t os, uint num, const char *content) {
  struct PAM *pam = &pam_table[os][num];
  pam->delivered = false;
  pam->ack[s] = 1;
  pam->sum = 1;
  strncpy(pam->content, content, MAX_CHARS);
  pam_table_n[os]++;
}

void print_pam_table() {
  printf("[\n");
  for (size_t i = 0; i < n_processes + 1; ++i) {
    printf("Original sender: %ld, size: %d\n", i, pam_table_n[i]);
    for (size_t j = 0; j < pam_table_n[i]; ++j) {
      struct PAM pam = pam_table[i][j];
      printf("Packet %ld, acks: ", j);
      for (size_t k = 0; k < n_processes + 1; ++k) {
        printf("%d|", pam.ack[k]);
      }
      printf("\n");
    }
    printf("\n");
  }
  printf("}\n");
}

void urb_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                 socklen_t *sender_len, char *message) {
  char buffer[MAX_CHARS] = {0};
  struct PAM *pam = NULL; // pending, ack, message
  uint num = 0;           // seq_num
  in_port_t os_port = 0;  // original sender port
  size_t os = 0;          // original sender
  size_t s = 0;           // sender

  do {
    // print_pam_table();

    beb_deliver(sock_fd, sender_addr, sender_len, message);

    strncpy(buffer, message, MAX_CHARS);
    // Decode packet: type | seq
    sscanf(message, "%hu|%u|%s", &os_port, &num, message);

    s = get_index(*sender_addr);
    os = (uint)(htons(os_port) - SHIFT_MODULO) % MAX_PROCESS;

    // printf("message %s, s index: %ld, os_port: %hu, num: %u, m: %s\n\n",
    // buffer,
    //        s, htons(os_port), num, message);

    if (os <= n_processes && s <= n_processes) {
      check_pam_table(os, num);

      // if (pam_table_n[os] >= num) {

      pam = &(pam_table[os][num]);

      if (pam->sum > 0) {
        if (!pam->ack[s]) {
          pam->ack[s] = true;
          pam->sum++;
        }
      } else {
        pam_alloc(s, os, num, message);
        beb_broadcast(sock_fd, buffer);
      }
      // }
    }
  } while (!pam || (pam->sum <= n_processes / 2) || // FIXME: round division up?
           (pam->delivered == true));

  sender_addr->sin_port = os_port; // Port of true original sender
  pam->delivered = true;
}