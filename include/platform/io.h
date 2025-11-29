#ifndef _IO_H
#define _IO_H

#include "platform/types.h"
#include <stdio.h>

size_t fs_size(FILE *file);
u8 *fs_read(FILE *file, size_t size_in_bytes);
u8 *readfile(char *path);

#endif
