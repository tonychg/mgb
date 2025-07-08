#ifndef __FS_H__
#define __FS_H__

#include "types.h"
#include <stdio.h>

size_t fs_size(FILE *file);
u8 *fs_read(FILE *file, size_t size_in_bytes);

#endif
