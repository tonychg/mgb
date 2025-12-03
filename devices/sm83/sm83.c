#include "emu/sm83.h"
#include "emu/memory.h"
#include "emu/timer.h"
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
	cpu->pc = 0x0100;

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
	cpu->previous = SM83_CORE_FETCH;
	cpu->multiplier = 1;

	// Timers
	cpu->internal_divider = 0;
	cpu->internal_timer = 0;
	cpu->dma.scheduled = false;
}

struct sm83_core *sm83_init(void)
{
	struct sm83_core *cpu = NULL;

	cpu = sm83_cpu_alloc();
	if (!cpu)
		return NULL;
	sm83_cpu_reset(cpu);
	cpu->timer_enabled = true;
	// DMA
	cpu->dma.start_addr = 0;
	cpu->dma.scheduled = false;
	cpu->dma.cursor = SM83_DMA_TRANSFER_CYCLES;
	return cpu;
}

void sm83_halt(struct sm83_core *cpu)
{
	u8 reg_ie = cpu->memory->load8(cpu, IE);
	u8 reg_if = cpu->memory->load8(cpu, IF);
	if (!(reg_ie & reg_if & 0x1F)) {
		cpu->state = SM83_CORE_HALT;
	} else if (!cpu->ime) {
		cpu->state = SM83_CORE_HALT_BUG;
	}
}

void sm83_schedule_dma_transfer(struct sm83_core *cpu, u16 start_addr)
{
	if (start_addr >= 0xE000) {
		start_addr &= 0xDFFF;
	}
	cpu->dma.start_addr = start_addr;
	cpu->dma.scheduled = true;
	cpu->dma.cursor = SM83_DMA_TRANSFER_CYCLES;
	cpu->previous = cpu->state;
	cpu->state = SM83_CORE_DMA_TRANSFER;
}

void sm83_cpu_step(struct sm83_core *cpu)
{
	u16 irq_ack;

	cpu->cycles += cpu->multiplier;
	switch (cpu->state) {
	case SM83_CORE_DMA_TRANSFER: {
		if (cpu->dma.scheduled && cpu->dma.cursor > 0) {
			// printf("[%lu] [$%04X] Cursor %d, Start address %04X Scheduled: %d\n",
			//        cpu->cycles, cpu->index, cpu->dma.cursor,
			//        cpu->dma.start_addr + cpu->dma.cursor,
			//        cpu->dma.scheduled);
			u8 value = cpu->memory->load8(
				cpu, cpu->dma.start_addr + cpu->dma.cursor);
			cpu->memory->write8(cpu, 0xFE00 + cpu->dma.cursor,
					    value);
			cpu->dma.cursor--;
		} else {
			cpu->dma.scheduled = false;
			cpu->pc++;
			cpu->state = SM83_CORE_FETCH;
			sm83_isa_execute(cpu);
		}
		break;
	}
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
	case SM83_CORE_IDLE_0:
	case SM83_CORE_IDLE_1:
		sm83_isa_execute(cpu);
		break;
	case SM83_CORE_HALT: {
		if (cpu->ime) {
			irq_ack = sm83_irq_ack(cpu);
			if (irq_ack) {
				cpu->ime = false;
				cpu->state = cpu->previous;
				cpu->halted = false;
				sm83_stack_push_pc(cpu, &cpu->pc);
				cpu->pc = irq_ack;
			}
		} else {
			u8 reg_ie = cpu->memory->load8(cpu, IE);
			u8 reg_if = cpu->memory->load8(cpu, IF);
			if ((reg_ie & reg_if & 0x1F)) {
				cpu->memory->write8(cpu, IF, 0);
				cpu->halted = false;
				cpu->state = SM83_CORE_FETCH;
				cpu->pc++;
			}
		}
	} break;
	case SM83_CORE_HALT_BUG: {
		// FIX ME
		u8 irq_regs;
		u8 irq_reqs;
		// printf("Halt bug is triggered\n");
		irq_reqs = cpu->memory->load8(cpu, IF);
		irq_regs = cpu->memory->load8(cpu, IE) & irq_reqs;
		if (irq_regs != 0) {
			cpu->index = cpu->sp;
			cpu->ime = false;
			cpu->state = SM83_CORE_FETCH;
			cpu->memory->write8(cpu, IF, 0);
		}
		cpu->state = SM83_CORE_FETCH;
		cpu->halted = false;
		break;
	}
	}
	if (cpu->timer_enabled)
		sm83_update_timer_registers(cpu);
}

void sm83_destroy(struct sm83_core *cpu)
{
	zfree(cpu->memory);
	zfree(cpu);
}
