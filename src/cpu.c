#include "cpu.h"
#include "memory.h"
#include "alloc.h"
#include "timer.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "opcodes.h"

void cpu_sleep_ns(int nanoseconds)
{
	nanosleep((const struct timespec[]){ { 0, nanoseconds } }, NULL);
}

Cpu *cpu_init(void)
{
	Cpu *cpu;

	if ((cpu = (Cpu *)malloc(sizeof(Cpu))) == NULL)
		return NULL;
	cpu->state = CPU_CORE_RUNNING;
	cpu->opcode = 0;
	cpu->cycles = 0;
	cpu->ticks = 0;
	cpu->irq = list_create();
	return cpu;
}

void cpu_bind_memory(Cpu *cpu, Memory *memory)
{
	cpu->memory = memory;
}

void cpu_reset(Cpu *cpu)
{
	cpu->sp = 0xFFFE;
	cpu->pc = 0x100;

	SET_AF(cpu, 0x01B0);
	SET_BC(cpu, 0x0013);
	SET_DE(cpu, 0x00D8);
	SET_HL(cpu, 0x014D);

	cpu->opcode = 0;
	cpu->cycles = 0;
	cpu->halted = false;
	cpu->ime = false;
	cpu->ime_cycles = 0;
	list_empty(cpu->irq);
	cpu->state = CPU_CORE_RUNNING;
	cpu->multiplier = 1;
}

void cpu_release(Cpu *cpu)
{
	zfree(cpu);
}

void cpu_flag_set(Cpu *cpu, int flag)
{
	cpu->f = flag;
}

void cpu_flag_toggle(Cpu *cpu, int flag)
{
	cpu->f |= flag;
}

void cpu_flag_untoggle(Cpu *cpu, int flag)
{
	cpu->f &= (~flag);
}

void cpu_flag_clear(Cpu *cpu)
{
	cpu_flag_set(cpu, FLAG_NONE);
}

bool cpu_flag_is_set(Cpu *cpu, int flag)
{
	return (cpu->f & flag) != 0;
}

void cpu_flag_flip(Cpu *cpu, int flag)
{
	cpu->f ^= flag;
}

void cpu_flag_set_or_clear(Cpu *cpu, int flag)
{
	if (cpu_flag_is_set(cpu, flag))
		cpu_flag_set(cpu, flag);
	else
		cpu_flag_clear(cpu);
}

void cpu_pc_decrement(Cpu *cpu)
{
	cpu->pc--;
}

void cpu_pc_increment(Cpu *cpu)
{
	cpu->pc++;
}

u8 cpu_read_pc_addr(Cpu *cpu)
{
	u8 byte = MEM_READ(cpu, cpu->pc);
	cpu_pc_increment(cpu);
	return byte;
}

u16 cpu_read_word(Cpu *cpu)
{
	return (u16)cpu_read_pc_addr(cpu) | (u16)cpu_read_pc_addr(cpu) << 8;
}

u16 cpu_read_word_no_inc(Cpu *cpu)
{
	return (u16)MEM_READ(cpu, cpu->pc + 1) | (u16)MEM_READ(cpu, cpu->pc + 2)
							 << 8;
}

u8 cpu_read_byte(Cpu *cpu)
{
	return cpu_read_pc_addr(cpu);
}

u8 cpu_read_byte_no_inc(Cpu *cpu)
{
	return MEM_READ(cpu, cpu->pc + 1);
}

void cpu_jump_word(Cpu *cpu, u16 r16)
{
	cpu->pc = r16;
}

void cpu_enable_display(Cpu *cpu)
{
	MEM_WRITE(cpu, LY_LCD, 0x94);
}

void cpu_debug(Cpu *cpu)
{
	printf("   Z = %d | N = %d\n", cpu_flag_is_set(cpu, FLAG_ZERO),
	       cpu_flag_is_set(cpu, FLAG_SUBS));
	printf("   H = %d | C = %d\n", cpu_flag_is_set(cpu, FLAG_HALF),
	       cpu_flag_is_set(cpu, FLAG_CARRY));
	printf("A = $%02X  |  F = $%02X\n", cpu->a, cpu->f);
	printf("%08b | %08b\n", cpu->a, cpu->f);
	printf("B = $%02X  |  C = $%02X\n", cpu->b, cpu->c);
	printf("%08b | %08b\n", cpu->b, cpu->c);
	printf("D = $%02X  |  E = $%02X\n", cpu->d, cpu->e);
	printf("%08b | %08b\n", cpu->d, cpu->e);
	printf("H = $%02X  |  L = $%02X\n", cpu->h, cpu->l);
	printf("%08b | %08b\n", cpu->h, cpu->l);
	printf("    SP = $%04X\n", cpu->sp);
	printf(" %016b\n", cpu->sp);
	printf("    PC = $%04X\n", cpu->pc);
	printf(" %016b\n", cpu->pc);
	printf("IME = %d  | HALT = %d\n", cpu->ime, cpu->halted);
}

char *cpu_resolve_operand(Cpu *cpu, const char *op)
{
	static char buffer[20];

	if (!strcmp(op, "a16") || !strcmp(op, "n16")) {
		sprintf(buffer, "$%04X", cpu_read_word_no_inc(cpu));
	} else if (!strcmp(op, "a8") || !strcmp(op, "n8")) {
		sprintf(buffer, "$%02X", cpu_read_byte_no_inc(cpu));
	} else if (!strcmp(op, "e8")) {
		u8 byte = cpu_read_byte_no_inc(cpu);
		s8 offset = (s8)byte;
		sprintf(buffer, "$%02X [%d]", byte, offset);
	} else {
		sprintf(buffer, "%s", op);
	}
	return buffer;
}

void cpu_debug_instruction(Cpu *cpu, Instruction instruction)
{
	printf("00:%04X", cpu->pc);
	for (int i = 0; i < instruction.length; i++) {
		printf(" %02X", MEM_READ(cpu, cpu->pc + i));
	}
	printf(" -> %s", instruction.mnemonic);
	if (instruction.op_1 != NULL) {
		printf(" %s", cpu_resolve_operand(cpu, instruction.op_1));
	}
	if (instruction.op_2 != NULL)
		printf(" %s", cpu_resolve_operand(cpu, instruction.op_2));
	printf("\n");
}

Instruction cpu_prefetch(Cpu *cpu)
{
	u8 opcode;
	Instruction instruction;

	opcode = MEM_READ(cpu, cpu->pc);
	if (opcode == 0xCB) {
		opcode = MEM_READ(cpu, cpu->pc + 1);
		instruction = cpu_op_decode_cb(opcode);
	} else {
		instruction = cpu_op_decode(opcode);
	}
	if (cpu->debug) {
		cpu_debug_instruction(cpu, instruction);
	}
	return instruction;
}

void cpu_execute(Cpu *cpu, Instruction instruction)
{
	cpu_pc_increment(cpu);
	if (!instruction.prefixed) {
		opcode_execute(cpu, instruction.opcode);
	} else {
		cpu_pc_increment(cpu);
		opcode_execute_cb(cpu, instruction.opcode);
	}
}

void cpu_tick(Cpu *cpu)
{
	if (MEM_READ(cpu, LY_LCD) == 144) {
		MEM_WRITE(cpu, IF, MEM_READ(cpu, IF) | IR_VBLANK);
	}
	cpu_cycle(cpu);
	cpu->ticks++;
}

bool cpu_has_interrupt(Cpu *cpu)
{
	return cpu->ime && (MEM_READ(cpu, IF) & MEM_READ(cpu, IE));
}

bool cpu_has_pending_interrupt(Cpu *cpu)
{
	return cpu->irq->len > 0;
}

void cpu_handler_vblank(Cpu *cpu)
{
	if (cpu->debug)
		printf("IR_VBLANK handle\n");
	opcode_stack_push_pc(cpu, &cpu->pc);
	cpu->pc = 0x40;
}

void cpu_handler_lcd(Cpu *cpu)
{
	if (cpu->debug)
		printf("IR_LCD handle\n");
	opcode_stack_push_pc(cpu, &cpu->pc);
	cpu->pc = 0x48;
}

void cpu_handler_timer(Cpu *cpu)
{
	if (cpu->debug)
		printf("IR_TIMER handle\n");
	opcode_stack_push_pc(cpu, &cpu->pc);
	cpu->pc = 0x50;
}

void cpu_handler_serial(Cpu *cpu)
{
	if (cpu->debug)
		printf("IR_SERIAL handle\n");
	opcode_stack_push_pc(cpu, &cpu->pc);
	cpu->pc = 0x58;
}

void cpu_handler_joypad(Cpu *cpu)
{
	if (cpu->debug)
		printf("IR_JOYPAD handle\n");
	opcode_stack_push_pc(cpu, &cpu->pc);
	cpu->pc = 0x60;
}

void cpu_ir_handle(Cpu *cpu, void (*ir_handler)(Cpu *cpu))
{
	(*ir_handler)(cpu);
}

void cpu_ack_interrupt(Cpu *cpu)
{
	if (IR_REQUESTED(cpu, IR_VBLANK)) {
		list_add_node_head(cpu->irq, &cpu_handler_vblank);
		MEM_WRITE(cpu, IF, MEM_READ(cpu, IF) & ~IR_VBLANK);
	}
	if (IR_REQUESTED(cpu, IR_LCD)) {
		list_add_node_head(cpu->irq, &cpu_handler_lcd);
		MEM_WRITE(cpu, IF, MEM_READ(cpu, IF) & ~IR_LCD);
	}
	if (IR_REQUESTED(cpu, IR_TIMER)) {
		list_add_node_head(cpu->irq, &cpu_handler_timer);
		MEM_WRITE(cpu, IF, MEM_READ(cpu, IF) & ~IR_TIMER);
	}
	if (IR_REQUESTED(cpu, IR_SERIAL)) {
		list_add_node_head(cpu->irq, &cpu_handler_serial);
		MEM_WRITE(cpu, IF, MEM_READ(cpu, IF) & ~IR_SERIAL);
	}
	if (IR_REQUESTED(cpu, IR_JOYPAD)) {
		list_add_node_head(cpu->irq, &cpu_handler_joypad);
		MEM_WRITE(cpu, IF, MEM_READ(cpu, IF) & ~IR_JOYPAD);
	}
	cpu->ime = false;
	cpu->ime_cycles = 5;
	cpu->state = CPU_CORE_INTERRUPT_DISPATCH;
}

void cpu_req_interrupt(Cpu *cpu, enum Interrupt ir)
{
	cpu->ime = true;
	MEM_WRITE(cpu, IF, MEM_READ(cpu, IF) | ir);
}

void cpu_handle_timer(Cpu *cpu)
{
	TimerControl tc = timer_tac(cpu);

	if (cpu->ticks != 0 && cpu->ticks % 256 == 0)
		++cpu->memory->bus[DIV];
	if (tc.tima_enabled && cpu->ticks % tc.clock_rate == 0) {
		++cpu->memory->bus[TIMA];
		// TIMA Overflow
		if (cpu->memory->bus[TIMA] == 0) {
			cpu->memory->bus[TIMA] = cpu->memory->bus[TMA];
			cpu_req_interrupt(cpu, IR_TIMER);
		}
	}
}

void cpu_cycle(Cpu *cpu)
{
	Instruction instruction;

	if (cpu_has_interrupt(cpu)) {
		cpu_ack_interrupt(cpu);
	}
	switch (cpu->state) {
	case CPU_CORE_RUNNING:
		if (cpu_has_pending_interrupt(cpu)) {
			ListNode *ir = list_pop_node_head(cpu->irq);
			cpu_ir_handle(cpu, ir->data);
		}
		instruction = cpu_prefetch(cpu);
		cpu->opcode = instruction.opcode;
		cpu->cycles = instruction.cycles;
		cpu_execute(cpu, instruction);
		cpu->state = CPU_CORE_EXECUTE;
		break;
	case CPU_CORE_EXECUTE:
		if (cpu->cycles == 0) {
			cpu->state = CPU_CORE_RUNNING;
		}
		cpu->cycles--;
		break;
	case CPU_CORE_INTERRUPT_DISPATCH:
		if (cpu->ime_cycles == 0)
			cpu->state = CPU_CORE_RUNNING;
		cpu->ime_cycles--;
		break;
	case CPU_CORE_HALT:
		cpu->cycles--;
		break;
	}
	cpu_handle_timer(cpu);
}
