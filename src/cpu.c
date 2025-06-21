#include "cpu.h"
#include "alloc.h"
#include <stdio.h>
#include <string.h>

void cpu_reset(Cpu *cpu)
{
	cpu->sp = 0xFFFE;
	cpu->pc = 0x100;

	register_set_value(cpu->af, 0x01B0);
	register_set_value(cpu->bc, 0x0013);
	register_set_value(cpu->de, 0x00D8);
	register_set_value(cpu->hl, 0x014D);

	cpu->instruction = 0;
	cpu->cycles = 0;
	cpu->halted = false;
	cpu->state = CPU_CORE_FETCH;
	cpu->multiplier = 1;
}

Cpu *cpu_init(void)
{
	Cpu *cpu;

	if ((cpu = (Cpu *)malloc(sizeof(Cpu))) == NULL)
		return NULL;
	if ((cpu->af = register_create()) == NULL)
		return NULL;
	if ((cpu->bc = register_create()) == NULL)
		return NULL;
	if ((cpu->de = register_create()) == NULL)
		return NULL;
	if ((cpu->hl = register_create()) == NULL)
		return NULL;
	return cpu;
}

void cpu_release(Cpu *cpu)
{
	register_release(cpu->af);
	register_release(cpu->bc);
	register_release(cpu->de);
	register_release(cpu->hl);
	zfree(cpu);
}

unsigned cpu_get_z(Cpu *cpu)
{
	return (cpu->af->low & 0b10000000) >> 7;
}

unsigned cpu_get_n(Cpu *cpu)
{
	return (cpu->af->low & 0b01000000) >> 6;
}

unsigned cpu_get_h(Cpu *cpu)
{
	return (cpu->af->low & 0b00100000) >> 5;
}

unsigned cpu_get_c(Cpu *cpu)
{
	return (cpu->af->low & 0b00010000) >> 4;
}

void cpu_pc_decrement(Cpu *cpu)
{
	cpu->pc--;
}

void cpu_pc_increment(Cpu *cpu)
{
	cpu->pc++;
}

void cpu_tick(Cpu *cpu)
{
}

void cpu_debug(Cpu *cpu)
{
	printf("   Z = %d | N = %d\n", cpu_get_c(cpu), cpu_get_n(cpu));
	printf("   H = %d | C = %d\n", cpu_get_h(cpu), cpu_get_c(cpu));
	printf("A = 0x%02X | F = 0x%02X\n", cpu->af->high, cpu->af->low);
	printf("%08b | %08b\n", cpu->af->high, cpu->af->low);
	printf("B = 0x%02X | C = 0x%02X\n", cpu->bc->high, cpu->bc->low);
	printf("%08b | %08b\n", cpu->bc->high, cpu->bc->low);
	printf("D = 0x%02X | E = 0x%02X\n", cpu->de->high, cpu->de->low);
	printf("%08b | %08b\n", cpu->de->high, cpu->de->low);
	printf("H = 0x%02X | L = 0x%02X\n", cpu->hl->high, cpu->hl->low);
	printf("%08b | %08b\n", cpu->hl->high, cpu->hl->low);
	printf("    SP = 0x%04X\n", cpu->pc);
	printf("    PC = 0x%04X\n", cpu->pc);
}

u16 cpu_read_word(Cpu *cpu, Cartridge *cartridge)
{
	u16 word = cartridge->buffer[cpu->pc];
	cpu_pc_increment(cpu);
	word |= cartridge->buffer[cpu->pc] << 8;
	cpu_pc_increment(cpu);
	return word;
}

u8 cpu_read_byte(Cpu *cpu, Cartridge *cartridge)
{
	u8 byte = cartridge->buffer[cpu->pc];
	cpu_pc_increment(cpu);
	return byte;
}

void cpu_jump_word(Cpu *cpu, u16 r16)
{
	cpu->pc = r16;
}

void cpu_instruction(Cpu *cpu, Cartridge *cartridge)
{
	u8 opcode = cartridge->buffer[cpu->pc];
	const char *opcode_repr = cpu_opcode_repr(opcode);
	int length = cpu_instruction_length(opcode);

	printf("0x%02X -> %s (%d)", opcode, opcode_repr, length);
	cpu_pc_increment(cpu);
	if (length == 3) {
		u16 r16 = cpu_read_word(cpu, cartridge);
		printf(" $%04X\n", r16);
		if (!strcmp(opcode_repr, "JP"))
			cpu->pc = r16;
	} else if (length == 2) {
		u8 r8 = cpu_read_byte(cpu, cartridge);
		printf(" $%02X\n", r8);
	} else {
		printf("\n");
	}
}

#ifdef TEST
#include "tests.h"

void test_cpu()
{
	printf("# Testing cpu.c\n");
	Cpu *cpu = cpu_init();
	assert(cpu != NULL);
	cpu_reset(cpu);
	cpu_debug(cpu);
	cpu_release(cpu);
}
#endif
