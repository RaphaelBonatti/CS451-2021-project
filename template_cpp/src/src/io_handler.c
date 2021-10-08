#include <io_handler.h>
#include <stdio.h>

void init_config_info(struct ConfigInfo *configInfo, const char *filename) {
  FILE *fp;

  if ((fp = fopen(filename, "r")) == NULL) {
    printf("Error! could not open the file");
    exit(1);
  }

  if (fscanf(fp, "%d %d", &(configInfo->m), &(configInfo->i)) < 0) {
    printf("Error! matching failure while reading the file");
    exit(1);
  }

  printf("Value of m=%d, i=%d\n", configInfo->m, configInfo->i);
  fclose(fp);
}

void write_output(const char *log_events, const char *filename) {
  FILE *fp;

  if ((fp = fopen(filename, "w")) == NULL) {
    printf("Error! could not open the file");
    exit(1);
  }

  if (fprintf(fp, log_events) < 0) {
    printf("Error! could not write to file");
    exit(1);
  }

  fclose(fp);
}

void log_events(char *events, const char *event) {
  if (!events) {

  }
}