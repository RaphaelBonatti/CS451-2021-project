#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include <stubborn_links.h>

pthread_t tlistener;

void *sl_receive_ack(void *_args) {
  struct Args *args = (struct Args *)_args;
  char ack_buffer[MESSAGE_SIZE + 3] = {0};
  char buffer[MESSAGE_SIZE] = {0};

  // Create the message to confirm
  snprintf(ack_buffer, MESSAGE_SIZE + 3, "ack%s", args->message);

  do {
    // We do not care about the sender address in this case,
    // we just check that the message is the same
    ssize_t n = recvfrom(args->sock_fd, buffer, MESSAGE_SIZE, 0, NULL, 0);
  } while (strncmp(buffer, ack_buffer, strlen(ack_buffer)));

  // set ack to 1 so that main thread is unblocked
  *(args->ack) = 1;
  return NULL;
}

void sl_send(int sock_fd, struct sockaddr_in *receiver_addr,
             const char *message) {
  short ack = 0;
  struct Args args = {sock_fd, message, &ack};
  pthread_t tlistener;

  // New thread needed to avoid deadlock (thread will wait for ack)
  pthread_create(&tlistener, NULL, sl_receive_ack, (void *)&args);

  // blocking code until ack is received
  while (!ack) {
    // TODO: check send failure?
    ssize_t send_check =
        sendto(sock_fd, message, strlen(message), 0,
               (struct sockaddr *)receiver_addr, sizeof(*receiver_addr));
  }

  pthread_join(tlistener, NULL);
}

void sl_deliver(int sock_fd, struct sockaddr_in *sender_addr,
                socklen_t *sender_len, char *message) {
  // Clean message to avoid left over chars
  memset(message, 0, MESSAGE_SIZE);

  if (recvfrom(sock_fd, message, MESSAGE_SIZE, 0,
               (struct sockaddr *)sender_addr, sender_len) <= 0) {
    fprintf(stderr, "Error, recvfrom failed. errno = %d\n", errno);
  } else {
    // FIXME: It is overkill to send the whole message should have packet id
    // Send ack
    char ack_buffer[MESSAGE_SIZE + 3] = {0};
    snprintf(ack_buffer, MESSAGE_SIZE + 3, "ack%s", message);
    ssize_t send_check =
        sendto(sock_fd, ack_buffer, strlen(ack_buffer), 0,
               (struct sockaddr *)sender_addr, sizeof(*sender_addr));
  }
}