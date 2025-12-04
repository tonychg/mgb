#include "mgb/sm83.h"
#include "mgb/memory.h"

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

void debug_interrupt(struct sm83_core *cpu, struct interrupt_struct interrupt,
		     u8 if_reg, u8 bitmask)
{
	if (interrupt.number != IRQ_VBLANK) {
		char disasm[256];
		sm83_disassemble(cpu, disasm);
		printf("[%lu] [%s] Acknowledge %s interrupt irqs: %08b bitmask: %08b\n",
		       cpu->cycles, disasm, interrupt.description, if_reg,
		       bitmask);
	}
}

u8 sm83_irq_ack(struct sm83_core *cpu)
{
	u8 irqs;
	u8 if_reg;

	if (!cpu->ime)
		return 0;
	if_reg = cpu->memory.load8(cpu, IF);
	irqs = cpu->memory.load8(cpu, IE) & if_reg;
	for (int i = 0; i < ARRAY_SIZE(interrupts); i++) {
		struct interrupt_struct interrupt = interrupts[i];
		u8 bitmask = 1 << interrupt.number;
		if ((irqs & bitmask) != 0) {
			if_reg &= ~bitmask;
			cpu->memory.write8(cpu, IF, if_reg);
			return interrupt.vector;
		}
	}
	return 0;
}
