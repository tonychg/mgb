#include "cpu.h"
#include "alloc.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void cpu_sleep_ns(int nanoseconds)
{
	nanosleep((const struct timespec[]){ { 0, nanoseconds } }, NULL);
}

Cpu *cpu_init(void)
{
	Cpu *cpu;

	if ((cpu = (Cpu *)malloc(sizeof(Cpu))) == NULL)
		return NULL;
	if ((cpu->af = register_create()) == NULL)
		return NULL;
	if ((cpu->bc = register_create()) == NULL)
		return NULL;
	if ((cpu->de = register_create()) == NULL)
		return NULL;
	if ((cpu->hl = register_create()) == NULL)
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

	register_set_value(cpu->af, 0x01B0);
	register_set_value(cpu->bc, 0x0013);
	register_set_value(cpu->de, 0x00D8);
	register_set_value(cpu->hl, 0x014D);

	cpu->instruction = 0;
	cpu->cycles = 0;
	cpu->halted = false;
	cpu->state = CPU_CORE_FETCH;
	cpu->multiplier = 1;
}

void cpu_release(Cpu *cpu)
{
	register_release(cpu->af);
	register_release(cpu->bc);
	register_release(cpu->de);
	register_release(cpu->hl);
	zfree(cpu);
}

unsigned cpu_get_z(Cpu *cpu)
{
	return (cpu->af->low & 0b10000000) >> 7;
}

unsigned cpu_get_n(Cpu *cpu)
{
	return (cpu->af->low & 0b01000000) >> 6;
}

unsigned cpu_get_h(Cpu *cpu)
{
	return (cpu->af->low & 0b00100000) >> 5;
}

unsigned cpu_get_c(Cpu *cpu)
{
	return (cpu->af->low & 0b00010000) >> 4;
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
	u8 byte = cpu->memory->bus[cpu->pc];
	cpu_pc_increment(cpu);
	return byte;
}

u16 cpu_read_word(Cpu *cpu)
{
	return (u16)cpu_read_pc_addr(cpu) | (u16)cpu_read_pc_addr(cpu) << 8;
}

u8 cpu_read_byte(Cpu *cpu)
{
	return cpu_read_pc_addr(cpu);
}

void cpu_jump_word(Cpu *cpu, u16 r16)
{
	cpu->pc = r16;
}

void cpu_debug(Cpu *cpu)
{
	printf("   Z = %d | N = %d\n", cpu_get_c(cpu), cpu_get_n(cpu));
	printf("   H = %d | C = %d\n", cpu_get_h(cpu), cpu_get_c(cpu));
	printf("A = 0x%02X | F = 0x%02X\n", cpu->af->high, cpu->af->low);
	printf("%08b | %08b\n", cpu->af->high, cpu->af->low);
	printf("B = 0x%02X | C = 0x%02X\n", cpu->bc->high, cpu->bc->low);
	printf("%08b | %08b\n", cpu->bc->high, cpu->bc->low);
	printf("D = 0x%02X | E = 0x%02X\n", cpu->de->high, cpu->de->low);
	printf("%08b | %08b\n", cpu->de->high, cpu->de->low);
	printf("H = 0x%02X | L = 0x%02X\n", cpu->hl->high, cpu->hl->low);
	printf("%08b | %08b\n", cpu->hl->high, cpu->hl->low);
	printf("    SP = $%04X\n", cpu->sp);
	printf(" %016b\n", cpu->sp);
	printf("    PC = $%04X\n", cpu->pc);
	printf(" %016b\n", cpu->pc);
}

void cpu_debug_instruction(Instruction instruction)
{
	printf("$%02X [%d %d] %s %s %s", instruction.opcode, instruction.length,
	       instruction.cycles, instruction.mnemonic, instruction.op_1,
	       instruction.op_2);
}

void cpu_execute(Cpu *cpu, Instruction instruction)
{
	cpu_debug_instruction(instruction);

	if (instruction.length == 3) {
		u16 imm16 = cpu_read_word(cpu);
		printf(" $%04X\n", imm16);
		if (!strcmp(instruction.mnemonic, "JP"))
			cpu->pc = imm16;
	} else if (instruction.length == 2) {
		u8 imm8 = cpu_read_byte(cpu);
		printf(" $%04X\n", imm8);
	} else {
		printf("\n");
	}
}

void cpu_tick(Cpu *cpu)
{
	u8 opcode = cpu_read_pc_addr(cpu);
	Instruction instruction = cpu_op_decode(opcode);
	cpu_execute(cpu, instruction);
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
