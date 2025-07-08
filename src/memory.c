#include "gb/memory.h"
#include "gb/alloc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct memory *memory_init()
{
	struct memory *memory;

	if ((memory = (struct memory *)malloc(sizeof(struct memory))) == NULL)
		return NULL;
	memory->bus = (u8 *)calloc(0xFFFF + 1, sizeof(u8));
	if (memory->bus == NULL)
		return NULL;
	memory->vram_bank = 0;
	memory->wram_bank = 0;
	return memory;
}

void memory_reset(struct memory *memory)
{
	for (int i = 0; i <= 0xFFFF; i++) {
		memory->bus[i] = 0x00;
		if ((i >= 0xC000) && (i < 0xE000)) {
			if ((i & 0x8) ^ ((i & 0x800) >> 8)) {
				memory->bus[i] = 0x0f;
			} else {
				memory->bus[i] = 0xff;
			}
		}
	}
}

u8 memory_read(struct memory *memory, u16 addr)
{
	return memory->bus[addr];
}

void memory_write(struct memory *memory, u16 addr, u8 byte)
{
	// Reset Divider register on write
	// https://gbdev.io/pandocs/Timer_and_Divider_Registers.html
	if (addr == DIV) {
		memory->bus[addr] = 0;
	}
	memory->bus[addr] = byte;
}

void memory_write_word(struct memory *memory, u16 addr, u16 word,
		       bool big_endian)
{
	u8 high = word >> 8;
	u8 low = 0xFF & word;

	if (big_endian) {
		memory->bus[addr] = high;
		memory->bus[addr + 1] = low;
	} else {
		memory->bus[addr] = low;
		memory->bus[addr + 1] = high;
	}
}

void memory_bind_cartridge(struct memory *memory, struct cartridge *cartridge)
{
	memcpy(memory->bus, cartridge->buffer, 0x7FFF);
}

u8 memory_hardware_register(struct memory *memory, enum hardware_register reg)
{
	return memory_read(memory, reg);
}

void memory_release(struct memory *memory)
{
	zfree(memory->bus);
	zfree(memory);
}

void memory_debug(struct memory *memory, u16 start, u16 end)
{
	for (int i = 0; start + i <= end; i++) {
		if (memory->bus[start + i] != 0)
			printf("%02X", memory->bus[start + i]);
		else
			printf("..");
		if ((i + 1) % 32 == 0 && i > 0)
			printf("\n");
		else if ((i + 1) % 8 == 0 && i > 0)
			printf(" ");
	}
	printf("\n");
}

void memory_dump(struct memory *memory)
{
	FILE *file;

	if ((file = fopen("data/dump.gb", "w")) == NULL)
		return;
	fwrite(memory->bus, 0xFFFF, sizeof(u8), file);
	fclose(file);
}

#ifdef TEST
#include "gb/tests.h"

void test_memory()
{
	printf("# Testing memory.c\n");
	struct memory *memory = memory_init();
	u8 joyp_reg = memory_hardware_register(memory, P1_JOYP);
	assert(joyp_reg == 0);
	memory_release(memory);
}
#endif
