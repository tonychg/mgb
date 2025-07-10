#include "gb/sm83.h"
#include "gb/alloc.h"
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

static u8 sm83_irq_ack(struct sm83_core *cpu)
{
	u8 irq_regs;

	if (!cpu->ime)
		return 0;
	irq_regs = cpu->memory->load8(cpu, 0xFFFF) &
		   cpu->memory->load8(cpu, 0xFF0F);
	if (irq_regs & (1 << IRQ_VBLANK)) {
		irq_regs &= ~(1 << IRQ_VBLANK);
		return VEC_VBLANK;
	}
	if (irq_regs & (1 << IRQ_LCD)) {
		irq_regs &= ~(1 << IRQ_LCD);
		return VEC_LCD;
	}
	if (irq_regs & (1 << IRQ_TIMER)) {
		irq_regs &= ~(1 << IRQ_TIMER);
		return VEC_TIMER;
	}
	if (irq_regs & (1 << IRQ_SERIAL)) {
		irq_regs &= ~(1 << IRQ_SERIAL);
		return VEC_SERIAL;
	}
	if (irq_regs & (1 << IRQ_JOYPAD)) {
		irq_regs &= ~(1 << IRQ_JOYPAD);
		return VEC_JOYPAD;
	}
	return 0;
}

static void sm83_update_timer_registers(struct sm83_core *cpu)
{
	u8 next, tima;
	u8 divider = (u16)cpu->cycles << 8;
	u8 tac = cpu->memory->load8(cpu, 0xFF07);
	u8 clock_offset = CLOCK_RATE[(tac & 0b11)];

	// Be careful to bypass the reset rule
	// https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf
	cpu->memory->write8(cpu, 0xFF04, divider);
	if ((tac & (1 << 2)) != 0) {
		next = (u16)cpu->cycles >> clock_offset;
		tima = cpu->memory->load8(cpu, 0xFF05);
		if (!next && tima == 0xFF) {
			// Load TMA into TIMA after overflow
			next = cpu->memory->load8(cpu, 0xFF06);
			// TODO: IRQ_TIMER
		}
		cpu->memory->write8(cpu, 0xFF05, next);
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
	return cpu;
}

struct sm83_instruction sm83_cpu_fetch(struct sm83_core *cpu)
{
	struct sm83_instruction instruction;

	instruction = cpu_decode(cpu);
	return instruction;
}

void sm83_cpu_step(struct sm83_core *cpu)
{
	u16 irq_ack;

	cpu->cycles++;
	switch (cpu->state) {
	case SM83_CORE_FETCH:
		irq_ack = sm83_irq_ack(cpu);
		if (irq_ack) {
			cpu->ime = false;
			sm83_stack_push_pc(cpu, &cpu->pc);
			cpu->pc = irq_ack;
		}
		cpu->bus = cpu->memory->load8(cpu, cpu->pc);
		cpu->instruction = sm83_cpu_fetch(cpu);
		cpu->index = cpu->pc;
		++cpu->pc;
		sm83_isa_execute(cpu, cpu->instruction.opcode);
		break;
	case SM83_CORE_PC:
		cpu->bus = cpu->memory->read_segment(cpu);
		++cpu->pc;
		sm83_isa_execute(cpu, cpu->instruction.opcode);
		break;
	case SM83_CORE_READ:
		cpu->bus = cpu->memory->load8(cpu, cpu->ptr);
		sm83_isa_execute(cpu, cpu->instruction.opcode);
		break;
	case SM83_CORE_WRITE_0:
		cpu->memory->write8(cpu, cpu->ptr, cpu->bus);
		sm83_isa_execute(cpu, cpu->instruction.opcode);
		break;
	case SM83_CORE_WRITE_1:
		cpu->memory->write8(cpu, cpu->ptr, cpu->bus);
		sm83_isa_execute(cpu, cpu->instruction.opcode);
		break;
	case SM83_CORE_HALT:
		irq_ack = sm83_irq_ack(cpu);
		if (irq_ack) {
			cpu->ime = false;
			sm83_stack_push_pc(cpu, &cpu->pc);
			cpu->pc = irq_ack;
		}
		break;
	case SM83_CORE_IDLE:
		sm83_isa_execute(cpu, cpu->instruction.opcode);
		break;
	}
	sm83_update_timer_registers(cpu);
}

void sm83_destroy(struct sm83_core *cpu)
{
	zfree(cpu->memory);
	zfree(cpu);
}
