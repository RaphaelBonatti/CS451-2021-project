#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <common.h>
#include <stdio.h>
#include <stdlib.h>

enum ConfigType { PERFECT_LINK, FIFO, L_CAUSAL };

struct ConfigInfo {
  enum ConfigType type;
};

struct PLConfigInfo {
  enum ConfigType type; // Base class, first field.
  size_t n_messages;    // Number of messages to broadcast.
  size_t receiver_id;   // In of the processes receiving the messages.
};

struct FIFOConfigInfo {
  enum ConfigType type; // Base class, first field
  size_t n_messages;    // Number of messages to broadcast.
};

struct LCAUSALConfigInfo {
  enum ConfigType type; // Base class, first field
  size_t process_id;    // Id of current process
  size_t n_messages;    // Number of messages to broadcast.

  // Number of processes which affect causality
  size_t n_processes[MAX_PROCESSES];

  // Processes which affect causality
  size_t causal_processes[MAX_PROCESSES][MAX_PROCESSES];
};

size_t get_file_size(FILE *fp);
void parse_pl_config(FILE *fp, struct ConfigInfo *configInfo);
void parse_fifo_config(FILE *fp, struct ConfigInfo *configInfo);
void populate_causal_processes(char *str,
                               struct LCAUSALConfigInfo *lcausal_config,
                               size_t process_id);
void print_causal_processes(struct LCAUSALConfigInfo *lcausal_config);
void parse_lcausal_config(FILE *fp, struct ConfigInfo *configInfo);
void parse_config(struct ConfigInfo *configInfo, const char *filename);

#ifdef __cplusplus
}
#endif