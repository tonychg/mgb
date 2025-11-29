#include "platform/io.h"
#include "emu/memory.h"
#include <stdlib.h>

void memory_reset(struct shared *memory)
{
	for (int i = 0; i <= 0xFFFF; i++) {
		write_u8(memory, i, 0x00);
		if ((i >= 0xC000) && (i < 0xE000)) {
			if ((i & 0x8) ^ ((i & 0x800) >> 8)) {
				write_u8(memory, i, 0x0f);
			} else {
				write_u8(memory, i, 0xff);
			}
		}
	}
}

struct shared *allocate_memory()
{
	struct shared *memory = (struct shared *)malloc(sizeof(struct shared));
	if (!memory)
		return NULL;
	if (!(memory->array = allocate_array(MEMORY_SIZE))) {
		printf("failed to allocated new array\n");
		goto err;
	}
	memory_reset(memory);
	return memory;
err:
	zfree(memory);
	return NULL;
}

void destroy_memory(struct shared *memory)
{
	destroy_array(memory->array);
	zfree(memory);
}

u8 load_u8(struct shared *memory, u16 addr)
{
	if (!memory)
		panic("try to access unalocated memory");
	return get(memory->array, addr);
}

void write_u8(struct shared *memory, u16 addr, u8 value)
{
	if (!memory)
		panic("try to access unalocated memory");
	set(memory->array, addr, value);
}

int load_rom(struct shared *memory, char *path)
{
	FILE *file;
	size_t size_in_bytes;
	u8 buffer[MEMORY_SIZE] = { 0 };
	if (!memory)
		return -1;
	file = fopen(path, "r");
	if (!file)
		return -1;
	size_in_bytes = fs_size(file);
	if (size_in_bytes > MEMORY_SIZE)
		goto err;
	if (fread(buffer, sizeof(u8), MEMORY_SIZE, file) == 0)
		goto err;
	for (int i = 0; i < MEMORY_SIZE; i++)
		write_u8(memory, i, buffer[i]);
	fclose(file);
	return 0;
err:
	fclose(file);
	return -1;
}

void dump_memory(struct shared *memory)
{
	for (int i = 0; i <= MEMORY_SIZE; i++) {
		u8 byte = load_u8(memory, i);
		if (byte)
			printf("%02X", byte);
		else
			printf("..");
		if ((i + 1) % 32 == 0 && i > 0)
			printf("\n");
		else if ((i + 1) % 8 == 0 && i > 0)
			printf(" ");
	}
}

void print_addr(struct shared *memory, u16 addr)
{
	u8 byte = load_u8(memory, addr);
	printf("$%02X [%08b] %d\n", byte, byte, byte);
}
