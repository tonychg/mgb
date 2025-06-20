#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>

void cpu_reset(cpu *cpu)
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

cpu *cpu_init(void)
{
	struct cpu *cpu;

	if ((cpu = (struct cpu *)malloc(sizeof(struct cpu))) == NULL)
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

void cpu_release(cpu *cpu)
{
	register_release(cpu->af);
	register_release(cpu->bc);
	register_release(cpu->de);
	register_release(cpu->hl);
	free(cpu);
	cpu = NULL;
}

unsigned cpu_get_z(cpu *cpu)
{
	return (cpu->af->low & 0b10000000) >> 7;
}

unsigned cpu_get_n(cpu *cpu)
{
	return (cpu->af->low & 0b01000000) >> 6;
}

unsigned cpu_get_h(cpu *cpu)
{
	return (cpu->af->low & 0b00100000) >> 5;
}

unsigned cpu_get_c(cpu *cpu)
{
	return (cpu->af->low & 0b00010000) >> 4;
}

void cpu_pc_decrement(cpu *cpu)
{
	cpu->pc--;
}

void cpu_pc_increment(cpu *cpu)
{
	cpu->pc++;
}

void cpu_tick(cpu *cpu)
{
}

void cpu_debug(cpu *cpu)
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

#ifdef TEST
#include "tests.h"

void test_cpu()
{
	printf("# Testing cpu.c\n");
	cpu *cpu = cpu_init();
	assert(cpu != NULL);
	cpu_reset(cpu);
	cpu_debug(cpu);
	cpu_release(cpu);
}
#endif
