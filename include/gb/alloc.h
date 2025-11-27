#ifndef _ALLOC_H
#define _ALLOC_H

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

void zfree(void *ptr);

#endif
