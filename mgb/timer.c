#include "mgb/memory.h"
#include "mgb/timer.h"

void sm83_update_timer_registers(struct sm83_core *cpu)
{
	// Be careful to bypass the reset rule
	// https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf
	u8 reg_div = cpu->memory.load8(cpu, DIV);
	u8 reg_tac = cpu->memory.load8(cpu, TAC);

	cpu->internal_divider += cpu->multiplier;
	if (cpu->internal_divider >= SM83_FREQ / DIV_PERIOD) {
		cpu->internal_divider -= SM83_FREQ / DIV_PERIOD;
		reg_div++;
		cpu->memory.write8(cpu, DIV, reg_div);
	}
	// Is timer disabled
	if ((reg_tac >> 2) != 1)
		return;
	u64 period = tima_periods[reg_tac & 3];
	cpu->internal_timer += cpu->multiplier;
	while (cpu->internal_timer >= period) {
		u8 reg_tima = cpu->memory.load8(cpu, TIMA);
		reg_tima++;
		cpu->memory.write8(cpu, TIMA, reg_tima);
		// TIMA overflow
		if (!reg_tima) {
			// Request interrupt
			u8 irq_reqs = cpu->memory.load8(cpu, IF);
			cpu->memory.write8(cpu, IF, irq_reqs | 1 << IRQ_TIMER);
		}
		cpu->internal_timer -= period;
	}
}
