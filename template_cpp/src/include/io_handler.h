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
  int m;
  int i;
};

void init_config_info(struct ConfigInfo *configInfo, const char *filename);

void write_output(const char *log, const char *filename);

void log_events(char *events, const char *event);

#ifdef __cplusplus
}
#endif