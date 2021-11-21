#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MESSAGE_SIZE 256

struct Packet {
  uint32_t num;
  bool ack;
  char message[MESSAGE_SIZE];
  struct Packet *next;
};

struct List {
  struct Packet *head;
  struct Packet *current;
};

void initList(struct List *list);
void printList(struct List *list);
void insertLast(struct List *list, uint32_t num, bool ack, const char *message);
void deleteFirst(struct List *list);
bool isEmpty(struct List *list);
int length(struct List *list);

#ifdef __cplusplus
}
#endif