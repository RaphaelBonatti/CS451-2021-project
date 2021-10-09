#include <io_handler.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

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

  printf("Value of m=%lu, i=%lu\n", configInfo->n_messages,
         configInfo->receiver_id);
  fclose(fp);
}

void write_output(const char *log_events, const char *filename) {
  FILE *fp;

  printf("Creating %s file", filename);
  if ((fp = fopen(filename, "w")) == NULL) {
    printf("Error! could not open the file");
    exit(1);
  }

  if (fprintf(fp, "%s", log_events) < 0) {
    printf("Error! could not write to file");
    exit(1);
  }

  fclose(fp);
}

void log_events(char *events, const char *event) {
  if (!events) {
    printf("Error! events pointer is null");
    exit(1);
  }

  if (!event) {
    printf("Error! event pointer is null");
    exit(1);
  }

  // TODO: Dynamic allocation
  size_t events_byte_size = malloc_usable_size(events);
  size_t events_string_size = strlen(events);
  strncat(events, event, events_byte_size - events_string_size);
}

int is_receiver(unsigned long receiver_id, unsigned long host_id) {
  return receiver_id == host_id;
}