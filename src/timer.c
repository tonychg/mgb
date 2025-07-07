#include "timer.h"
#include <stdio.h>

TimerControl timer_tac(Cpu *cpu)
{
	u8 tac;
	TimerControl tc;

	tac = MEM_READ(cpu, TAC);
	tc.tima_enabled = (tac & (1 << 2)) == 0 ? false : true;
	tc.clock_rate = CLOCK_RATE[tac & 0b11];
	return tc;
}

void timer_debug(Cpu *cpu)
{
	printf("DIV = %d TIMA = %d ", cpu->memory->bus[DIV],
	       cpu->memory->bus[TIMA]);
	printf("TAC = %03b TMA = %d\n", cpu->memory->bus[TAC],
	       cpu->memory->bus[TMA]);
}

void timer_reset_div(Cpu *cpu)
{
	MEM_WRITE(cpu, DIV, 0x0);
}
