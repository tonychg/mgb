#include "platform/io.h"
#include "platform/mm.h"
#include "platform/types.h"
#include <stdlib.h>
#include <stdio.h>

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

u8 *readfile(char *path)
{
	FILE *file;
	size_t size_in_bytes;
	u8 *buffer = NULL;

	file = fopen(path, "r");
	if (!file)
		return NULL;
	size_in_bytes = fs_size(file);
	if (size_in_bytes > 0)
		buffer = fs_read(file, size_in_bytes);
	return buffer;
}
