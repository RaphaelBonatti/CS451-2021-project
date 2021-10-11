#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>
#include <stdlib.h>

/**
 * @brief
 *
 */
struct ConfigInfo {
  size_t n_messages;
  size_t receiver_id;
};

void init_config_info(struct ConfigInfo *configInfo, const char *filename);
void write_output(const char *log, const char *filename);
void log_events(char *events, const char *event);
int is_receiver(size_t receiver_id, size_t host_id);

#ifdef __cplusplus
}
#endif