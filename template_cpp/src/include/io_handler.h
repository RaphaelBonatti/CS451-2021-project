#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>

typedef unsigned int uint;

struct PlConfigInfo {
  size_t n_messages;
  size_t receiver_id;
};

struct ConfigInfo {
  size_t n_messages;
};

struct EventLog {
  char *buffer;
  size_t size;
};

void init_config_info(struct ConfigInfo *configInfo, const char *filename);
void init_io_handler();
void init_events(struct EventLog *events);
void destroy_events();
void realloc_events(struct EventLog *events);
void write_output(const char *filename);
size_t check_available_size(struct EventLog *events, char *buffer);
void log_deliver_events(char *buffer, size_t sender_id);
void log_send_events(char *buffer);
int is_receiver(size_t receiver_id, size_t host_id);

#ifdef __cplusplus
}
#endif