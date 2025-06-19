#include "cpu.h"
#include <stdlib.h>

void cpu_reset(cpu *cpu)
{
	cpu->a = 0;
	cpu->z = 0;
	cpu->n = 0;
	cpu->h = 0;
	cpu->c = 0;
	register_set_value(cpu->bc, 0);
	register_set_value(cpu->de, 0);
	register_set_value(cpu->hl, 0);
	cpu->sp = 0;
	cpu->pc = 0;

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
	if ((cpu->bc = register_create()) == NULL)
		return NULL;
	if ((cpu->de = register_create()) == NULL)
		return NULL;
	if ((cpu->hl = register_create()) == NULL)
		return NULL;
	return cpu;
}

void cpu_tick(cpu *cpu)
{
}

void cpu_release(cpu *cpu)
{
	register_release(cpu->bc);
	register_release(cpu->de);
	register_release(cpu->hl);
	free(cpu);
	cpu = NULL;
}

#ifdef TEST
#include "test.h"

void test_cpu()
{
	printf("# Testing cpu.c\n");
	cpu *cpu = cpu_init();
	cpu_reset(cpu);
	cpu_release(cpu);
}
#endif
