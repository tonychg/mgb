#ifndef _MM_H
#define _MM_H

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

void panic(char *msg);
void zfree(void *ptr);

#endif
