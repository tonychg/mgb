#include "emu/sm83.h"
#include "emu/memory.h"

struct interrupt_struct {
	const char *description;
	const enum sm83_irq number;
	const u16 vector;
};

// clang-format off
const static struct interrupt_struct interrupts[] = {
	{ "VBLANK", IRQ_VBLANK, VEC_VBLANK },
	{ "LCD", IRQ_LCD, VEC_LCD },
	{ "TIMER", IRQ_TIMER, VEC_TIMER },
	{ "SERIAL", IRQ_SERIAL, VEC_SERIAL },
	{ "JOYPAD", IRQ_JOYPAD, VEC_JOYPAD },
};
// clang-format on

u8 sm83_irq_ack(struct sm83_core *cpu)
{
	u8 irq_regs;
	u8 irq_reqs;

	if (!cpu->ime)
		return 0;
	irq_reqs = cpu->memory->load8(cpu, IF);
	irq_regs = cpu->memory->load8(cpu, IE) & irq_reqs;
	for (int i = 0; i < ARRAY_SIZE(interrupts); i++) {
		struct interrupt_struct interrupt = interrupts[i];
		u8 bitmask = 1 << interrupt.number;
		if ((irq_regs & bitmask) != 0) {
			irq_reqs ^= bitmask;
			printf("[%lu] Acknowledge %s interrupt\n",
				   cpu->cycles, interrupt.description);
			cpu->memory->write8(cpu, IF, irq_reqs);
			return interrupt.vector;
		}
	}
	return 0;
}
