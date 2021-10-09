#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/**
 * @brief
 *
 */
struct ConfigInfo {
  unsigned long n_messages;
  unsigned long receiver_id;
};

void init_config_info(struct ConfigInfo *configInfo, const char *filename);
void write_output(const char *log, const char *filename);
void log_events(char *events, const char *event);
int is_receiver(unsigned long receiver_id, long unsigned host_id);

#ifdef __cplusplus
}
#endif