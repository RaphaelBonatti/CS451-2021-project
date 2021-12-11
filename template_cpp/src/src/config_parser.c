#include <string.h>

#include "config_parser.h"

size_t get_file_size(FILE *fp) {
  long fsize = 0;

  fseek(fp, 0L, SEEK_END);
  fsize = ftell(fp);
  fseek(fp, 0L, SEEK_SET);

  return (size_t)fsize;
}

void parse_pl_config(FILE *fp, struct ConfigInfo *configInfo) {
  struct PLConfigInfo *pl_config = (struct PLConfigInfo *)configInfo;
  if (fscanf(fp, "%lu %lu", &(pl_config->n_messages),
             &(pl_config->receiver_id)) < 0) {
    printf("Error! matching failure while reading the file");
    exit(1);
  }
}

void parse_fifo_config(FILE *fp, struct ConfigInfo *configInfo) {
  struct FIFOConfigInfo *fifo_config = (struct FIFOConfigInfo *)configInfo;
  if (fscanf(fp, "%lu", &(fifo_config->n_messages)) < 0) {
    printf("Error! matching failure while reading the file");
    exit(1);
  }
}

void populate_causal_processes(char *str,
                               struct LCAUSALConfigInfo *lcausal_config,
                               size_t process_id) {
  char *token = strtok(str, " ");
  uint causal_process_id = 0;

  while (token) {
    sscanf(token, "%u", &causal_process_id);
    lcausal_config->causal_processes[process_id]
                                    [lcausal_config->n_processes[process_id]] =
        causal_process_id;
    ++lcausal_config->n_processes[process_id];
    token = strtok(NULL, " ");
  }
}

void print_causal_processes(struct LCAUSALConfigInfo *lcausal_config) {
  for (size_t i = 0; i < MAX_PROCESSES; ++i) {
    for (size_t j = 0; j < lcausal_config->n_processes[i]; ++j) {
      printf("%lu ", lcausal_config->causal_processes[i][j]);
    }
    printf("\n");
  }
}

void parse_lcausal_config(FILE *fp, struct ConfigInfo *configInfo) {
  size_t process_id = 0;
  size_t fsize = get_file_size(fp);
  char *buffer = calloc(fsize, sizeof(char));

  struct LCAUSALConfigInfo *lcausal_config =
      (struct LCAUSALConfigInfo *)configInfo;
  if (fscanf(fp, "%lu%[^\n]", &(lcausal_config->n_messages), buffer) <= 0) {
    printf("Error! matching failure while reading the file");
    exit(1);
  }

  int num = 1;

  while ((num = fscanf(fp, "%lu%[^\n]", &process_id, buffer)) != EOF &&
         num != 0) {
    populate_causal_processes(buffer, lcausal_config, process_id);
  }

  // print_causal_processes(lcausal_config);

  free(buffer);
}

void parse_config(struct ConfigInfo *configInfo, const char *filename) {
  FILE *fp;

  if ((fp = fopen(filename, "r")) == NULL) {
    printf("Error! could not open the file");
    exit(1);
  }

  if (configInfo->type == PERFECT_LINK) {
    parse_pl_config(fp, configInfo);
  } else if (configInfo->type == FIFO) {
    parse_fifo_config(fp, configInfo);
  } else if (configInfo->type == L_CAUSAL) {
    parse_lcausal_config(fp, configInfo);
  }

  fclose(fp);
}