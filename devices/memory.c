#include "platform/io.h"
#include "emu/memory.h"
#include "platform/mm.h"
#include <stdlib.h>

void memory_reset(struct shared *memory)
{
	for (int i = 0; i <= 0xFFFF; i++) {
		if ((i >= 0xC000) && (i < 0xE000)) {
			if ((i & 0x8) ^ ((i & 0x800) >> 8)) {
				write_u8(memory, i, 0x0f);
			} else {
				write_u8(memory, i, 0xff);
			}
		} else if (i == P1_JOYP) {
			write_u8(memory, i, 0xcf);
		} else if (i == IF) {
			write_u8(memory, i, 0xe1);
		} else {
			write_u8(memory, i, 0x00);
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
	if (addr == P1_JOYP) {
		u8 current = load_u8(memory, P1_JOYP);
		u8 result = value | (current & 0xf);
		value = result;
	}
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

void request_interrupt(struct shared *memory, enum sm83_irq number)
{
	u8 irq_reqs = load_u8(memory, IF);
	irq_reqs |= 1 << number;
	memory->array->bytes[IF] = irq_reqs;
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

void print_hardware_registers(struct shared *memory)
{
	const char *format = "%1$6s $%2$04X : $%3$02X (%3$08b)";
	u8 *mem = memory->array->bytes;
	for (int i = 0; i < ARRAY_SIZE(helpers); i++) {
		if (i == 1 || (i > 2 && (i - 1) % 2 == 0))
			printf("\n");
		const struct hreg_print_helper helper = helpers[i];
		printf(format, helper.label, helper.addr, mem[helper.addr]);
	}
	printf("\n");
}
