#include <stdlib.h>
#include <string.h>

#include "io_handler.h"
#include "localized_causal_broadcast.h"

static uint v_clock[MAX_PROCESS];
static uint lsn;
static size_t process_id;
static size_t n_processes;
static char *serialized;       // Serialization of the clock vector
static char *broadcast_buffer; // For storing clock vector and message to send
static char *deliver_buffer;   // For sender clock vector

void lcb_init(int sock_fd, size_t _process_id, struct sockaddr_in *addrs,
              size_t _n_processes) {
  lsn = 0;
  n_processes = _n_processes + 1;
  process_id = _process_id;

  size_t len = n_processes * sizeof(uint) + 1; //  + 1 for the null terminator

  serialized = calloc(len, sizeof(char));
  broadcast_buffer = calloc(len + MAX_CHARS, sizeof(char));
  deliver_buffer = calloc(len + MAX_CHARS, sizeof(char));
  urb_init(sock_fd, _process_id, addrs, _n_processes);
}

void lcb_destroy() {
  free(serialized);
  free(broadcast_buffer);
  free(deliver_buffer);
  urb_destroy();
}

void serialize_vector_clock(const uint *vec) {

  printf("%u\n", vec[process_id]);

  memcpy(serialized, vec, n_processes * sizeof(uint));
  serialized[n_processes * sizeof(uint)] = '\0';

  printf("serialized: %s\n", serialized);

  // for (size_t i = 0; i < n_processes; ++i) {
  //   printf("%u\n", vec[i]);
  //   *((uint *)(serialized + i * sizeof(uint))) = vec[i];
  // }
}

void lcb_broadcast(int sock_fd, const char *message) {
  uint w_clock[MAX_PROCESS] = {0};
  size_t metadata_len = n_processes * sizeof(uint);

  // TODO: may have problem as it starts at 0, but first id is 1, in this case
  // count n_process + 1 to be sure
  memcpy(w_clock, v_clock, metadata_len);
  printf("%u\n", lsn);
  w_clock[process_id] = lsn;
  ++lsn;
  serialize_vector_clock(w_clock);

  printf("id:%ld ,serialized: %s\n", process_id, serialized);

  memcpy(broadcast_buffer, serialized, metadata_len);

  // TODO: may need to memset buffer to 0
  snprintf(broadcast_buffer + metadata_len, MAX_CHARS, "|%s", message);

  printf("lcb_broadcast: %s\n", broadcast_buffer);
  urb_broadcast(sock_fd, broadcast_buffer,
                metadata_len + sizeof(char) + strlen(message));
}

void deserialize_vector_clock(char *const chrs, uint *vec) {
  for (size_t i = 0; i < n_processes; ++i) {
    vec[i] = *((uint *)(chrs + i * sizeof(uint)));
  }
}

bool check_condition(uint *vec) {
  for (size_t i = 0; i < n_processes; ++i) {
    if (vec[i] > v_clock[i]) {
      return false;
    }
  }

  return true;
}

void lcb_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                 socklen_t *sender_len, char *message) {
  struct PAM **pam_table = pass_table();
  static uint delivered[MAX_PROCESS] = {
      0}; // FIXME: This approach forces a FIFO queue, but space requirement is
          // minimized
  uint w_clock[MAX_PROCESS] = {0};
  char content[MAX_CHARS]; // FIXME: Need to adapt all message lengths
  size_t metadata_len = n_processes * sizeof(uint);

  urb_deliver(sock_fd, sender_addr, sender_len, message);

  printf("message: %s\n", message);

  for (size_t i = 0; i <= n_processes; ++i) {
    if (pam_table[i]) {
      printf("pam table\n");
      while (pam_table[i][delivered[i]].delivered == true) {
        printf("checking table\n");
        memcpy(deliver_buffer, pam_table[i][delivered[i]].content,
               metadata_len);

        sscanf(pam_table[i][delivered[i]].content + metadata_len, "|%s",
               content);
        deserialize_vector_clock(deliver_buffer, w_clock);
        if (check_condition(w_clock)) {
          ++v_clock[i];
          ++delivered[i];

          printf("content %s\n", content);

          // Log the delivered message
          // TODO: Need a buffer to keep the messages to
          // deliver, then we avoid logging here
          log_deliver_events(content, i);
        } else {
          break;
        }
      }
    }
  }

  printf("Quit lcb deliver\n");
}