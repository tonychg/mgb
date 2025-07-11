#ifndef _FS_H
#define _FS_H

#include "types.h"
#include <stdio.h>

size_t fs_size(FILE *file);
u8 *fs_read(FILE *file, size_t size_in_bytes);
u8 *readfile(char *path);

#endif
