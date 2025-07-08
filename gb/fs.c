#include "gb/fs.h"
#include "gb/alloc.h"
#include <stdlib.h>

size_t fs_size(FILE *file)
{
	size_t size_in_bytes;
	fseek(file, 0L, SEEK_END);
	size_in_bytes = ftell(file);
	fseek(file, 0L, SEEK_SET);
	return size_in_bytes;
}

u8 *fs_read(FILE *file, size_t size_in_bytes)
{
	u8 *buffer;

	if ((buffer = (u8 *)calloc(size_in_bytes, sizeof(u8))) == NULL)
		return NULL;
	if (fread(buffer, sizeof(u8), size_in_bytes, file) == 0) {
		zfree(buffer);
		return NULL;
	}
	fclose(file);
	return buffer;
}
