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
