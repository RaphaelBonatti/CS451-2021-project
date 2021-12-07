// Inspired from:
// https://www.tutorialspoint.com/data_structures_algorithms/linked_list_program_in_c.htm

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"

void initList(struct List *list) {
  list->head = NULL;
  list->current = NULL;
}

// display the list
void printList(struct List *list) {
  struct Packet *ptr = list->head;
  printf("\n[ ");

  // start from the beginning
  while (ptr != NULL) {
    printf("(%d, %d, %s) ", ptr->num, ptr->ack, ptr->message);
    ptr = ptr->next;
  }

  printf(" ]");
}

// insert link at the first location
void insertLast(struct List *list, uint32_t num, bool ack,
                const char *message, size_t n) {
  // create a link
  struct Packet *link = (struct Packet *)malloc(sizeof(struct Packet));

  link->num = num;
  link->ack = ack;
  link->n = n;
  strncpy(link->message, message, MESSAGE_SIZE - 1);
  link->next = NULL;

  if (list->head != NULL) {
    list->current->next = link;
    list->current = link;
  } else {
    list->head = link;
    list->current = link;
  }
}

// delete first item
void deleteFirst(struct List *list) {

  // save reference to first link
  struct Packet *tempLink = list->head;

  // mark next to first link as first
  list->head = list->head->next;

  // free the deleted link
  free(tempLink);
}

// is list empty
bool isEmpty(struct List *list) { return list->head == NULL; }

int length(struct List *list) {
  int length = 0;
  struct Packet *current;

  for (current = list->head; current != NULL; current = current->next) {
    length++;
  }

  return length;
}