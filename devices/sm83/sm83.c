#include "emu/sm83.h"
#include "emu/memory.h"
#include "platform/mm.h"
#include <stdlib.h>

static struct sm83_core *sm83_cpu_alloc(void)
{
	struct sm83_core *cpu;

	cpu = (struct sm83_core *)malloc(sizeof(struct sm83_core));
	if (!cpu)
		goto exit;
	cpu->memory = (struct sm83_memory *)malloc(sizeof(struct sm83_memory));
	if (!cpu->memory)
		goto free_cpu;
	cpu->parent = NULL;
	return cpu;

free_cpu:
	zfree(cpu);
exit:
	return NULL;
}

static const int CLOCK_RATE[4] = {
	9,
	7,
	5,
	3,
};

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

static u8 sm83_irq_ack(struct sm83_core *cpu)
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
			if (interrupt.number != IRQ_VBLANK)
				printf("[%lu] Acknowledge %s interrupt\n", cpu->cycles,
				       interrupt.description);
			cpu->memory->write8(cpu, IF, irq_reqs);
			return interrupt.vector;
		}
	}
	return 0;
}

static void sm83_update_timer_registers(struct sm83_core *cpu)
{
	u16 t_cycles = cpu->cycles * 4;
	u8 tima_next, tima, tma;
	u8 divider = t_cycles >> 8;
	u8 tac = cpu->memory->load8(cpu, 0xFF07);
	u8 clock_offset = CLOCK_RATE[(tac & 0b11)];

	// Be careful to bypass the reset rule
	// https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf
	cpu->memory->write8(cpu, 0xFF04, divider);
	if ((tac >> 2) == 1) {
		tima_next = divider >> clock_offset;
		tima = cpu->memory->load8(cpu, 0xFF05);
		if (!tima_next && tima == 0xFF) {
			// Load TMA into TIMA after overflow
			tma = cpu->memory->load8(cpu, 0xFF06);
			u8 irq_reqs = cpu->memory->load8(cpu, IF);
			cpu->memory->write8(cpu, IF, irq_reqs | 1 << IRQ_TIMER);
			cpu->memory->write8(cpu, 0xFF05, tma);
		} else {
			cpu->memory->write8(cpu, 0xFF05, tima_next);
		}
	}
}

static void sm83_stack_push_pc(struct sm83_core *cpu, u16 *pc)
{
	cpu->sp--;
	cpu->memory->write8(cpu, cpu->sp, msb(cpu->pc));
	cpu->sp--;
	cpu->memory->write8(cpu, cpu->sp, lsb(cpu->pc));
}

void sm83_cpu_reset(struct sm83_core *cpu)
{
	cpu->sp = 0xFFFE;
	cpu->pc = 0x100;

	SET_AF(cpu, 0x01B0);
	SET_BC(cpu, 0x0013);
	SET_DE(cpu, 0x00D8);
	SET_HL(cpu, 0x014D);

	cpu->cycles = 0;
	cpu->halted = false;
	cpu->ime = false;
	cpu->ime_cycles = 0;
	cpu->ptr = 0;
	// Memory value of program counter
	cpu->bus = 0;
	// Accumulator to fetch 16 bits value
	cpu->acc = 0;
	// Beginning address of the current segment
	cpu->index = 0;
	cpu->state = SM83_CORE_FETCH;
	cpu->multiplier = 1;
}

struct sm83_core *sm83_init(void)
{
	struct sm83_core *cpu = NULL;

	cpu = sm83_cpu_alloc();
	if (!cpu)
		return NULL;
	sm83_cpu_reset(cpu);
	cpu->timer_enabled = true;
	return cpu;
}

void sm83_cpu_step(struct sm83_core *cpu)
{
	u16 irq_ack;

	cpu->cycles += cpu->multiplier;
	switch (cpu->state) {
	case SM83_CORE_FETCH:
		irq_ack = sm83_irq_ack(cpu);
		if (irq_ack) {
			cpu->ime = false;
			sm83_stack_push_pc(cpu, &cpu->pc);
			cpu->pc = irq_ack;
		}
		cpu->bus = cpu->memory->load8(cpu, cpu->pc);
		cpu->instruction = sm83_decode(cpu);
		cpu->index = cpu->pc;
		++cpu->pc;
		sm83_isa_execute(cpu);
		break;
	case SM83_CORE_PC:
		cpu->bus = cpu->memory->load8(cpu, cpu->pc);
		++cpu->pc;
		sm83_isa_execute(cpu);
		break;
	case SM83_CORE_READ_0:
	case SM83_CORE_READ_1:
		cpu->bus = cpu->memory->load8(cpu, cpu->ptr);
		sm83_isa_execute(cpu);
		break;
	case SM83_CORE_WRITE_0:
	case SM83_CORE_WRITE_1:
		cpu->memory->write8(cpu, cpu->ptr, cpu->bus);
		sm83_isa_execute(cpu);
		break;
	case SM83_CORE_HALT:
		irq_ack = sm83_irq_ack(cpu);
		if (irq_ack) {
			cpu->ime = false;
			sm83_stack_push_pc(cpu, &cpu->pc + 1);
			cpu->pc = irq_ack;
			cpu->state = SM83_CORE_FETCH;
		}
		break;
	case SM83_CORE_IDLE_0:
	case SM83_CORE_IDLE_1:
		sm83_isa_execute(cpu);
		break;
	}
	if (cpu->timer_enabled)
		sm83_update_timer_registers(cpu);
}

void sm83_destroy(struct sm83_core *cpu)
{
	zfree(cpu->memory);
	zfree(cpu);
}
