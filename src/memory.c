#include "memory.h"
#include "alloc.h"
#include <string.h>
#include <stdio.h>

Memory *memory_init()
{
	Memory *memory;

	if ((memory = (Memory *)malloc(sizeof(Memory))) == NULL)
		return NULL;
	memory->bus = (u8 *)calloc(0xFFFF + 1, sizeof(u8));
	if (memory->bus == NULL)
		return NULL;
	memory->vram_bank = 0;
	memory->wram_bank = 0;
	return memory;
}

void memory_bind_cartridge(Memory *memory, Cartridge *cartridge)
{
	memcpy(memory->bus, cartridge->buffer, 0x7FFF);
}

u8 memory_hardware_register(Memory *memory, enum HardwareRegister r)
{
	return memory->bus[r];
}

void memory_release(Memory *memory)
{
	zfree(memory->bus);
	zfree(memory);
}

void memory_debug(Memory *memory, u16 start, u16 end)
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

#ifdef TEST
#include "tests.h"

void test_memory()
{
	printf("# Testing memory.c\n");
	Memory *memory = memory_init();
	u8 joyp_reg = memory_hardware_register(memory, P1_JOYP);
	assert(joyp_reg == 0);
	memory_release(memory);
}
#endif
