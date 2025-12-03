#include "platform/mm.h"
#include <stdio.h>
#include <stdlib.h>

void panic(char *msg)
{
	printf("PANIC: %s\n", msg);
}

void zfree(void *ptr)
{
	if (ptr != NULL) {
		free(ptr);
		ptr = NULL;
	}
}

struct byte_array *allocate_array(size_t size)
{
	struct byte_array *array =
		(struct byte_array *)malloc(sizeof(struct byte_array));
	if (!array)
		return NULL;
	array->bytes = (u8 *)calloc(sizeof(u8), size);
	array->size = size;
	if (!array->bytes)
		goto err;
	return array;
err:
	zfree(array);
	return NULL;
}

void destroy_array(struct byte_array *array)
{
	zfree(array->bytes);
	zfree(array);
}

u8 get(struct byte_array *array, u16 index)
{
	char msg[256] = "";
	if (!array || !array->bytes)
		panic("unalocated array");
	if (array->size < index) {
		sprintf(msg, "get: index %d out of range: %lu", index,
			array->size);
		panic(msg);
	}
	return array->bytes[index];
}

void set(struct byte_array *array, u16 index, u8 byte)
{
	char msg[256] = "";
	if (!array || !array->bytes)
		panic("unalocated array");
	if (array->size < index) {
		sprintf(msg, "set: index %d out of range: %lu", index,
			array->size);
		panic(msg);
	}
	array->bytes[index] = byte;
}
