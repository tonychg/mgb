#include "gb/alloc.h"
#include <stdlib.h>

void zfree(void *ptr)
{
	if (ptr != NULL) {
		free(ptr);
		ptr = NULL;
	}
}
