#include "gb/alloc.h"

void zfree(void *ptr)
{
	if (ptr != NULL) {
		free(ptr);
		ptr = NULL;
	}
}
