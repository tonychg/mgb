#include "platform/io.h"
#include "platform/mm.h"
#include "mgb/memory.h"
#include <stdio.h>
#include <stdlib.h>

void memory_reset(struct memory *mem)
{
	for (int i = 0; i <= 0xFFFF; i++) {
		if ((i >= 0xC000) && (i < 0xE000)) {
			if ((i & 0x8) ^ ((i & 0x800) >> 8)) {
				mem->ram[i] = 0x0f;
			} else {
				mem->ram[i] = 0xff;
			}
		} else if (i == P1_JOYP) {
			mem->ram[i] = 0xff;
		} else if (i == IF) {
			mem->ram[i] = 0xe1;
		} else {
			mem->ram[i] = 0x00;
		}
	}
}

int load_rom(struct memory *mem, char *path)
{
	FILE *file;
	size_t size_in_bytes;
	u8 buffer[MEMORY_SIZE] = { 0 };
	if (!mem)
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
		mem->ram[i] = buffer[i];
	fclose(file);
	return 0;
err:
	fclose(file);
	return -1;
}

void dump_memory(struct memory *mem)
{
	for (int i = 0; i <= MEMORY_SIZE; i++) {
		u8 byte = mem->ram[i];
		if ((i + 32) < MEMORY_SIZE) {
			if ((i + 1) % 32 == 0 && i > 0)
				printf("\n%04X-%04X | ", i, i + 32);
			else if ((i + 1) % 8 == 0 && i > 0)
				printf(" ");
		}
		if (byte)
			printf("%02X", byte);
		else
			printf("..");
	}
	printf("\n");
}

void dump_memory_to_file(struct memory *mem, char *filename)
{
	FILE *fptr;
	fptr = fopen(filename, "w");
	if (!fptr) {
		printf("Failed to open %s\n", filename);
		return;
	}
	if (!fwrite(mem->ram, MEMORY_SIZE, sizeof(u8), fptr)) {
		printf("Failed to write save\n");
	}
	fclose(fptr);
}

void print_addr(struct memory *mem, u16 addr)
{
	u8 byte = mem->ram[addr];
	printf("$%02X [%08b] %d\n", byte, byte, byte);
}

void request_interrupt(struct memory *mem, enum sm83_irq number)
{
	u8 irq_reqs = mem->ram[IF];
	irq_reqs |= 1 << number;
	mem->ram[IF] = irq_reqs;
}

struct hreg_print_helper {
	const char *label;
	const enum hardware_register addr;
};

// clang-format off
static const struct hreg_print_helper helpers[] = {
	{ "JOYP", P1_JOYP },
	{ "IF", IF },
	{ "IE", IE },
	{ "DIV", DIV },
	{ "TIMA", TIMA },
	{ "TMA", TMA },
	{ "TAC", TAC },
	{ "LCDC", LCDC_LCD },
	{ "STAT", STAT_LCD },
	{ "SCY", SCY },
	{ "SCX", SCX },
	{ "LYC", LY_LCD },
	{ "LY", LY_LCD },
	{ "DMA", DMA_OAM_DMA },
	{ "BGP", BGP_BG },
	{ "OBJP0", OBP0_OBJ },
	{ "OBJP1", OBP1_OBJ },
	{ "WY", WY },
	{ "WX", WX },
};
// clang-format on

void print_hardware_registers(struct memory *mem)
{
	const char *format = "%1$6s $%2$04X : $%3$02X (%3$08b)";
	for (int i = 0; i < ARRAY_SIZE(helpers); i++) {
		if (i == 1 || (i > 2 && (i - 1) % 2 == 0))
			printf("\n");
		const struct hreg_print_helper helper = helpers[i];
		printf(format, helper.label, helper.addr,
		       mem->ram[helper.addr]);
	}
	printf("\n");
}
