#include "serializer.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

void serialize(int count, ...) {
  // argument list
  va_list list;
  char *ptr = NULL;
  void *data_ptr = NULL;
  size_t data_size = 0;
  size_t size = 0;

  // init list
  va_start(list, count);

  ptr = va_arg(list, void *);

  for (int i = 0; i < count; ++i) {
    data_ptr = va_arg(list, void *);
    data_size = va_arg(list, size_t);

    memcpy(ptr + size, data_ptr, data_size);
    size += data_size;
  }

  // End of argument list traversal
  va_end(list);
}

void deserialize(int count, ...) {
  // argument list
  va_list list;
  char *ptr = NULL;
  void *data_ptr = NULL;
  size_t data_size = 0;
  size_t size = 0;

  // init list
  va_start(list, count);

  ptr = va_arg(list, void *);

  for (int i = 0; i < count; ++i) {
    data_ptr = va_arg(list, void *);
    data_size = va_arg(list, size_t);

    memcpy(data_ptr, ptr + size, data_size);
    size += data_size;
  }

  // End of argument list traversal
  va_end(list);
}