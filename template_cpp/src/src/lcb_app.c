#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "io_handler.h"
#include "lcb_app.h"
#include "localized_causal_broadcast.h"

#define _XOPEN_SOURCE_EXTENDED 1
#include <unistd.h>

#define EVENT_SIZE 512
#define N_MESSAGES 256
#define FILENAME_SIZE 256

static int sock_fd;
static char filename[FILENAME_SIZE] = {0};
// struct sockaddr_in *app_addresses;
static struct sockaddr_in app_addresses[MAX_PROCESS];
static struct ConfigInfo app_configInfo;
static struct ProcessInfo *app_processInfos;
static size_t app_n_process;
static size_t app_process_id;
static struct sockaddr_in *app_process_address;

// pthread_t tsender;
static pthread_t treceiver;
static int receive_forever;

void app_init(const char *_filename, const char *configpath, size_t process_id,
              struct ProcessInfo *processInfos, size_t n_process) {
  receive_forever = 1;
  strncpy(filename, _filename, FILENAME_SIZE - 1);

  app_n_process = n_process;
  app_process_id = process_id;
  app_processInfos = processInfos;

  for (size_t i = 0; i < n_process; ++i) {
    get_sockaddr_by_id(&(app_addresses[i]), i + 1, processInfos, n_process);
  }

  // Create UDP socket for the current process
  if ((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    fprintf(stderr, "Error, socket creation failed. errno = %d\n", errno);
    exit(1);
  }

  init_config_info(&app_configInfo, configpath);

  // Init temporary log variable
  init_io_handler();

  // Initialize best effort broadcast with the list of processes for performance
  lcb_init(sock_fd, app_process_id, app_addresses, n_process);
}

void app_destroy() {
  receive_forever = 0;

  // Unblock thread
  shutdown(sock_fd, SHUT_RDWR);

  // pthread_cancel(treceiver); // kill thread
  // printf("Wainting pthread join\n");
  // pthread_cancel(treceiver);
  pthread_join(treceiver, NULL);
  if (close(sock_fd)) {
    fprintf(stderr, "Error: socket cannot be closed.");
  }
  lcb_destroy();
  // free(app_addresses);
  free(app_processInfos);
  write_output(filename);
  destroy_events();
}

size_t get_id_by_sockaddr(struct sockaddr_in *sockaddr,
                          struct ProcessInfo *processInfos, size_t n_process) {
  for (size_t i = 0; i < n_process; ++i) {
    if (sockaddr->sin_port ==
        processInfos[i]
            .port) { // TODO: test more than just port,
                     // sockaddr->sin_addr.s_addr == processInfos[i].ip &&
      return processInfos[i].id;
    }
  }
  return 0;
}

void get_sockaddr_by_id(struct sockaddr_in *sockaddr, size_t id,
                        struct ProcessInfo *processInfos, size_t n_process) {
  // I make no assumption about id location in the array (i.e. less efficient,
  // but more agnostic towards config)
  for (size_t i = 0; i < n_process; ++i) {
    if (processInfos[i].id == id) {
      sockaddr->sin_family = AF_INET;
      sockaddr->sin_port = processInfos[i].port;
      sockaddr->sin_addr.s_addr = processInfos[i].ip;
      break;
    }
  }
}

void *run_receiver(void *_args) {
  char buffer[MESSAGE_SIZE] = {0};
  struct sockaddr_in sender_addr;
  size_t sender_id = 0;
  // Important to give the size of the struct
  socklen_t sender_len = sizeof(sender_addr);
  uint delivered[MAX_PROCESS] = {0};
  struct PAM **pam_table = pass_table();

  while (receive_forever) {
    // Wait for and deliver messages
    lcb_deliver(sock_fd, &sender_addr, &sender_len, buffer);
  }
  pthread_exit(NULL);
  return NULL;
}

void run_sender() {
  char buffer[MESSAGE_SIZE] = {0};
  size_t message_n = 1;

  for (size_t i = 0; i < app_configInfo.n_messages; i++) {
    // Reset buffer and event to avoid left over chars
    memset(buffer, 0, MESSAGE_SIZE);

    // Create message to send
    snprintf(buffer, MESSAGE_SIZE, "%lu", message_n);

    // Log the message to send
    log_send_events(buffer);

    // broadcast message
    lcb_broadcast(sock_fd, buffer);
    ++message_n;
  }
}

void run_receiver_sender(size_t process_id) {
  // Run one thread for sender and one for receiver
  pthread_create(&treceiver, NULL, run_receiver, NULL);
  run_sender();
}

void app_run() {
  struct sockaddr_in process_addr;
  struct sockaddr_in loop_process_addr;

  // Get network information about current process, assuming IPV4
  get_sockaddr_by_id(&process_addr, app_process_id, app_processInfos,
                     app_n_process);

  // Bind the socket to the corresponding address
  if (bind(sock_fd, (const struct sockaddr *)&process_addr,
           sizeof(process_addr)) < 0) {
    fprintf(stderr, "Error, bind failed. errno = %d\n", errno);
    exit(1);
  }

  // Start receiver or sender, depending on the configuration file
  run_receiver_sender(app_process_id);
}