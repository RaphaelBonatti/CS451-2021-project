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
char *events;
size_t events_size;

void init_config_info(struct ConfigInfo *configInfo, const char *filename) {
  FILE *fp;

  if ((fp = fopen(filename, "r")) == NULL) {
    printf("Error! could not open the file");
    exit(1);
  }

  if (fscanf(fp, "%lu %lu", &(configInfo->n_messages),
             &(configInfo->receiver_id)) < 0) {
    printf("Error! matching failure while reading the file");
    exit(1);
  }

  fclose(fp);
}

void init_events() {
  printf("init event\n");
  events = calloc(INIT_SIZE, sizeof(char));

  if (!events) {
    printf("Error! Calloc failed to allocate.");
    exit(1);
  }

  events_size = INIT_SIZE;
  // Empty string
  events[0] = '\0';
}

void destroy_events() { free(events); }

void realloc_events() {
  printf("Reallocating!\n");
  size_t new_size = 2 * events_size;
  char *prev = events;
  printf("new size %lu\nprev %p\n", new_size, prev);
  events = realloc(events, new_size * sizeof(char));
  if (!events) {
    printf("Freeing!\n");
    free(prev);
    exit(1);
  }
  events_size = new_size;
}

void write_output(const char *filename) {
  FILE *fp;

  if ((fp = fopen(filename, "w")) == NULL) {
    printf("Error! could not open the file");
    exit(1);
  }

  if (fprintf(fp, "%s", events) < 0) {
    printf("Error! could not write to file");
    exit(1);
  }

  fclose(fp);
}

size_t check_available_size(char *buffer) {
  if (!events) {
    printf("Error! events pointer is null");
    exit(1);
  }

  if (!buffer) {
    printf("Error! buffer pointer is null");
    exit(1);
  }

  size_t used_size = strlen(events);

  if (events_size - used_size < strlen(buffer) + SAFETY_SIZE) {
    realloc_events();
  }

  return events_size - used_size;
}

void log_deliver_events(char *buffer, size_t sender_id) {

  size_t available_size = check_available_size(buffer);
  char *start = events + (events_size - available_size);

  // char *end = events + malloc_usable_size(events);

  snprintf(start, available_size, "d %lu %s\n", sender_id, buffer);
}

void log_send_events(char *buffer) {

  size_t available_size = check_available_size(buffer);
  char *start = events + (events_size - available_size);

  // char *end = events + malloc_usable_size(events);

  snprintf(start, available_size, "b %s\n", buffer);
}

int is_receiver(unsigned long receiver_id, unsigned long host_id) {
  return receiver_id == host_id;
}