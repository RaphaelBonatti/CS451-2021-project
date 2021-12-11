#include <stdlib.h>
#include <string.h>

#include "config_parser.h"
#include "localized_causal_broadcast.h"
#include "logger.h"
#include "serializer.h"

struct LCAUSALConfigInfo *config_info;
static uint v_clock[MAX_PROCESSES];
static uint lsn;
static size_t process_id;
static size_t n_processes;
static char *broadcast_buffer; // For storing clock vector and message to send
static char *deliver_buffer;   // For sender clock vector

void lcb_init(int sock_fd, size_t _process_id, struct sockaddr_in *addrs,
              size_t _n_processes, struct LCAUSALConfigInfo *_config_info) {
  lsn = 0;
  n_processes = _n_processes + 1;
  process_id = _process_id;

  size_t len = n_processes * sizeof(uint) + 1; //  + 1 for the null terminator

  broadcast_buffer = calloc(len + MAX_CHARS, sizeof(char));
  deliver_buffer = calloc(len + MAX_CHARS, sizeof(char));

  config_info = _config_info;

  urb_init(sock_fd, _process_id, addrs, _n_processes);
}

void lcb_destroy() {
  free(broadcast_buffer);
  free(deliver_buffer);
  urb_destroy();
}

void lcb_broadcast(int sock_fd, const char *message, size_t n) {
  uint w_clock[MAX_PROCESSES] = {0};
  size_t metadata_len = n_processes * sizeof(uint);

  // TODO: may have problem as it starts at 0, but first id is 1, in this case
  // count n_process + 1 to be sure
  memcpy(w_clock, v_clock, metadata_len);
  w_clock[process_id] = lsn;
  ++lsn;

  // TODO: may need to memset buffer to 0
  // Encode: w_clock, message
  serialize(2, broadcast_buffer, w_clock, n_processes * sizeof(uint), message,
            n);

  urb_broadcast(sock_fd, broadcast_buffer,
                metadata_len + sizeof(char) + strlen(message));
}

// bool check_causality(uint *vec) {
//   for (size_t i = 0; i < n_processes; ++i) {
//     if (vec[i] > v_clock[i]) {
//       return false;
//     }
//   }

//   return true;
// }

bool check_local_causality(uint *vec, size_t id) {
  size_t index = 0;

  // Check causility for process "id", because it depends on itself.
  if (vec[id] > v_clock[id]) {
    return false;
  }

  // Check causility for processes which affect process "id".
  for (size_t i = 0; i < config_info->n_processes[id]; ++i) {
    index = config_info->causal_processes[id][i];
    if (vec[index] > v_clock[index]) {
      return false;
    }
  }

  return true;
}

void lcb_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                 socklen_t *sender_len, char *message) {
  struct PAM **pam_table = pass_table();
  static uint delivered[MAX_PROCESSES] = {
      0}; // FIXME: This approach forces a FIFO queue, but space requirement is
          // minimized
  uint w_clock[MAX_PROCESSES] = {0};
  char content[MAX_CHARS]; // FIXME: Need to adapt all message lengths
  size_t metadata_len = n_processes * sizeof(uint);
  bool continue_looping = true;

  urb_deliver(sock_fd, sender_addr, sender_len, message);

  while (continue_looping) {
    continue_looping = false;

    for (size_t i = 0; i <= n_processes; ++i) {
      if (pam_table[i]) {
        while (pam_table[i][delivered[i]].delivered == true) {

          deserialize(2, pam_table[i][delivered[i]].content, w_clock,
                      n_processes * sizeof(uint), content,
                      pam_table[i][delivered[i]].n -
                          n_processes * sizeof(uint));

          if (check_local_causality(w_clock, i)) {
            ++v_clock[i];
            ++delivered[i];

            // May have new messages to lcb deliver
            continue_looping = true;

            // FOR DEBUG
            // for (size_t i = 0; i < n_processes; ++i) {
            //   printf("v:%ld, val:%u\n", i, v_clock[i]);
            // }

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
  }
}