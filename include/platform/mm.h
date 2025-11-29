#ifndef _MM_H
#define _MM_H

#include "platform/types.h"
#include <stdio.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct byte_array {
    u8 *bytes;
    size_t size;
};

void panic(char *msg);
void zfree(void *ptr);
struct byte_array *allocate_array(size_t size);
void destroy_array(struct byte_array *array);
u8 get(struct byte_array *array, u16 index);
void set(struct byte_array *array, u16 index, u8 byte);

#endif
