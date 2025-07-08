#include "gb/timer.h"
#include <stdio.h>

struct timer_control timer_tac(struct cpu *cpu)
{
	u8 tac;
	struct timer_control tc;

	tac = MEM_READ(cpu, TAC);
	tc.tima_enabled = (tac & (1 << 2)) == 0 ? false : true;
	tc.clock_rate = CLOCK_RATE[tac & 0b11];
	return tc;
}

void timer_debug(struct cpu *cpu)
{
	printf("DIV = %d TIMA = %d ", cpu->memory->bus[DIV],
	       cpu->memory->bus[TIMA]);
	printf("TAC = %03b TMA = %d\n", cpu->memory->bus[TAC],
	       cpu->memory->bus[TMA]);
}

void timer_reset_div(struct cpu *cpu)
{
	MEM_WRITE(cpu, DIV, 0x0);
}
