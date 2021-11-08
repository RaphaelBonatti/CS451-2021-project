#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "best_effort_broadcast.h"
#include "fifo_broadcast_application.h"
#include "io_handler.h"
#include "perfect_links.h"

#define EVENT_SIZE 512
#define N_MESSAGES 256

struct sockaddr_in *app_addresses;
struct ConfigInfo *app_configInfo;
struct ProcessInfo *app_processInfos;
size_t app_n_process;
size_t app_process_id;
struct sockaddr_in *app_process_address;

void app_destroy() { free(app_addresses); }

size_t get_id_by_sockaddr(struct sockaddr_in *sockaddr,
                          struct ProcessInfo *processInfos, size_t n_process) {
  for (size_t i = 0; i < n_process; ++i) {
    if (sockaddr->sin_addr.s_addr == processInfos[i].ip &&
        sockaddr->sin_port == processInfos[i].port) {
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
  struct ReceiverArgs *args = (struct ReceiverArgs *)_args;
  struct sockaddr_in sender_addr;
  size_t sender_id = 0;
  // Important to give the size of the struct
  socklen_t sender_len = sizeof(sender_addr);

  while (1) {
    // TODO: check receive failure?
    // Wait for and deliver messages
    beb_deliver(args->sock_fd, &sender_addr, &sender_len, args->buffer);

    // Get sender id
    sender_id =
        get_id_by_sockaddr(&sender_addr, app_processInfos, app_n_process);

    // Log the delivered message
    log_deliver_events(args->buffer, sender_id);
  }

  return NULL;
}

void *run_sender(void *_args) {
  struct SenderArgs *args = (struct SenderArgs *)_args;
  size_t message_n = 1;

  for (size_t i = 0; i < app_configInfo->n_messages; i++) {
    // Reset buffer and event to avoid left over chars
    memset(args->buffer, 0, MESSAGE_SIZE);

    // Create message to send
    snprintf(args->buffer, MESSAGE_SIZE, "%lu", message_n);

    // Log the message to send
    log_send_events(args->buffer);

    // broadcast message
    beb_broadcast(args->sock_fd, args->buffer);
    ++message_n;
  }

  printf("Sender finished!\n");
  return NULL;
}

void run_receiver_sender(int sock_fd, size_t process_id, char *sender_buffer,
                         char *receiver_buffer) {
  pthread_t tsender;
  pthread_t treceiver;

  struct SenderArgs sender_args = {sender_buffer, sock_fd};
  struct ReceiverArgs receiver_args = {receiver_buffer, sock_fd};

  // Run one thread for sender and one for receiver
  pthread_create(&tsender, NULL, run_sender, (void *)&sender_args);
  pthread_create(&treceiver, NULL, run_receiver, (void *)&receiver_args);

  // Main thread wait
  pthread_join(treceiver, NULL);
  pthread_join(tsender, NULL);
}

void run(int *sock_fd, struct ConfigInfo *configInfo, size_t process_id,
         struct ProcessInfo *processInfos, size_t n_process) {
  app_configInfo = configInfo;
  app_processInfos = processInfos;
  app_n_process = n_process;
  app_process_id = process_id;
  char sender_buffer[MESSAGE_SIZE] = {0};
  char receiver_buffer[MESSAGE_SIZE] = {0};
  struct sockaddr_in process_addr;
  struct sockaddr_in loop_process_addr;

  // Get network information about current process, assuming IPV4
  get_sockaddr_by_id(&process_addr, process_id, processInfos, n_process);

  // Create UDP socket for the current process
  if ((*sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    fprintf(stderr, "Error, socket creation failed. errno = %d\n", errno);
    exit(1);
  }

  // Bind the socket to the corresponding address
  if (bind(*sock_fd, (const struct sockaddr *)&process_addr,
           sizeof(process_addr)) < 0) {
    fprintf(stderr, "Error, bind failed. errno = %d\n", errno);
    exit(1);
  }

  app_addresses = calloc(n_process, sizeof(struct sockaddr_in));

  for (size_t i = 0; i < n_process; ++i) {
    get_sockaddr_by_id(&(app_addresses[i]), i + 1, processInfos, n_process);
  }

  // Initialize (or reset) perfect link
  pl_init();

  // Initialize best effort broadcast with the list of processes for performance
  beb_init(app_addresses, n_process);

  // Start receiver or sender, depending on the configuration file
  run_receiver_sender(*sock_fd, process_id, sender_buffer, receiver_buffer);
}