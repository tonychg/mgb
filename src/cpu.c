#include "cpu.h"
#include "alloc.h"
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

	cpu->read_cache = 0;
	cpu->instruction = 0;
	cpu->cycles = 0;
	cpu->halted = false;
	cpu->branch_taken = false;
	cpu->ime = false;
	cpu->ime_cycles = 0;
	cpu->state = CPU_CORE_FETCH;
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
	return (u16)MEM_READ(cpu, cpu->pc) | (u16)MEM_READ(cpu, cpu->pc + 1)
						     << 8;
}

u8 cpu_read_byte(Cpu *cpu)
{
	return cpu_read_pc_addr(cpu);
}

u8 cpu_read_byte_no_inc(Cpu *cpu)
{
	return MEM_READ(cpu, cpu->pc);
}

void cpu_jump_word(Cpu *cpu, u16 r16)
{
	cpu->pc = r16;
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
	printf("00:%04X", cpu->pc - 1);
	printf(" %02X", instruction.opcode);
	for (int i = 0; i < 2; i++) {
		if (instruction.length - 1 > i)
			printf(" %02X", MEM_READ(cpu, cpu->pc + i));
		else
			printf("   ");
	}
	printf(" -> %s", instruction.mnemonic);
	if (instruction.op_1 != NULL) {
		printf(" %s", cpu_resolve_operand(cpu, instruction.op_1));
	}
	if (instruction.op_2 != NULL)
		printf(" %s", cpu_resolve_operand(cpu, instruction.op_2));
	printf("\n");
}

void cpu_tick(Cpu *cpu)
{
	Instruction instruction = cpu_op_decode(cpu);

	if (!instruction.prefixed)
		opcode_execute(cpu, instruction);
	else
		opcode_execute_cb(cpu, instruction);
}

#ifdef TEST
#include "tests.h"

void test_cpu()
{
	printf("# Testing cpu.c\n");
	Cpu *cpu = cpu_init();
	assert(cpu != NULL);
	cpu_reset(cpu);
	cpu_debug(cpu);
	cpu_release(cpu);
}
#endif
