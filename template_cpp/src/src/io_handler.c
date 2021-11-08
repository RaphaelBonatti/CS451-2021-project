#include <io_handler.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#define EVENTS_SIZE 130000 // 8192
#define INIT_SIZE 256
// The safety size is used to protect against overflow from
// this kind of event: trying to put "d %lu %s\n" into events,
// where %s size (in bytes) is known but %lu is not.
// lu is 32 bits so 64 is sufficient
#define SAFETY_SIZE 64
// TODO: make it dynamic. Is it is ok to use global variable?

struct EventLog sender_events;
struct EventLog receiver_events;
struct EventLog order_events;
pthread_mutex_t lock;
uint num_order = 0;

void init_config_info(struct ConfigInfo *configInfo, const char *filename) {
  FILE *fp;

  if ((fp = fopen(filename, "r")) == NULL) {
    printf("Error! could not open the file");
    exit(1);
  }

  if (fscanf(fp, "%lu", &(configInfo->n_messages)) < 0) {
    printf("Error! matching failure while reading the file");
    exit(1);
  }

  fclose(fp);
}

void init_io_handler() {
  init_events(&sender_events);
  init_events(&receiver_events);
  init_events(&order_events);
}

void init_events(struct EventLog *events) {
  printf("init event\n");
  events->buffer = calloc(INIT_SIZE, sizeof(char));

  if (!events->buffer) {
    printf("Error! Calloc failed to allocate.");
    exit(1);
  }

  events->size = INIT_SIZE;
  // Empty string
  events->buffer[0] = '\0';
}

void destroy_events() {
  free(sender_events.buffer);
  free(receiver_events.buffer);
}

void realloc_events(struct EventLog *events) {
  // printf("Reallocating!\n");
  size_t new_size = 2 * events->size;
  char *prev = events->buffer;
  // printf("new size %lu\nprev %p\n", new_size, prev);
  events->buffer = realloc(events->buffer, new_size * sizeof(char));
  if (!events->buffer) {
    printf("Freeing!\n");
    free(prev);
    exit(1);
  }
  events->size = new_size;
}

void write_output(const char *filename) {
  FILE *fp;
  char *sender_line;
  char *sender_temp;
  char *receiver_line;
  char *receiver_temp;
  char *buffer;

  if ((fp = fopen(filename, "w")) == NULL) {
    printf("Error! could not open the file");
    exit(1);
  }

  buffer =
      calloc(strlen(sender_events.buffer) + strlen(receiver_events.buffer) + 1,
             sizeof(char));
  sender_line = __strtok_r(sender_events.buffer, "\n", &sender_temp);
  receiver_line = __strtok_r(receiver_events.buffer, "\n", &receiver_temp);

  for (uint i = 0; i < num_order; ++i) {
    if (order_events.buffer[i]) {
      strcat(strcat(buffer, sender_line), "\n");
      sender_line = __strtok_r(NULL, "\n", &sender_temp);
    } else {
      strcat(strcat(buffer, receiver_line), "\n");
      receiver_line = __strtok_r(NULL, "\n", &receiver_temp);
    }
  }

  if (fprintf(fp, "%s", buffer) < 0) {
    printf("Error! could not write to file");
    exit(1);
  }

  free(buffer);
  fclose(fp);
}

size_t check_available_size(struct EventLog *events, char *buffer) {
  if (!events->buffer) {
    printf("Error! events pointer is null");
    exit(1);
  }

  if (!buffer) {
    printf("Error! buffer pointer is null");
    exit(1);
  }

  size_t used_size = strlen(events->buffer);

  // reallocate while the size is too small
  while (events->size - used_size < strlen(buffer) + SAFETY_SIZE) {
    realloc_events(events);
  }

  // return available size
  return events->size - used_size;
}

void log_deliver_events(char *buffer, size_t sender_id) {

  pthread_mutex_lock(&lock);
  order_events.buffer[num_order] = 0;
  ++num_order;
  if (num_order >= order_events.size) {
    realloc_events(&order_events);
  }
  pthread_mutex_unlock(&lock);

  size_t available_size = check_available_size(&receiver_events, buffer);
  char *start =
      receiver_events.buffer + (receiver_events.size - available_size);

  // char *end = events + malloc_usable_size(events);

  snprintf(start, available_size, "d %lu %s\n", sender_id, buffer);
}

void log_send_events(char *buffer) {

  pthread_mutex_lock(&lock);
  order_events.buffer[num_order] = 1;
  ++num_order;
  if (num_order >= order_events.size - 1) {
    realloc_events(&order_events);
  }
  pthread_mutex_unlock(&lock);

  size_t available_size = check_available_size(&sender_events, buffer);
  char *start = sender_events.buffer + (sender_events.size - available_size);

  // char *end = events + malloc_usable_size(events);

  snprintf(start, available_size, "b %s\n", buffer);
}

int is_receiver(unsigned long receiver_id, unsigned long host_id) {
  return receiver_id == host_id;
}