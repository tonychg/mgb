#include "gb/alloc.h"
#include "gb/sm83.h"
#include "gb/types.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char *sm83_resolve_operand(struct sm83_core *cpu, const char *op,
				  u16 indice)
{
	char *buffer;

	buffer = (char *)calloc(16, sizeof(char));
	if (!buffer)
		return NULL;
	if (!strcmp(op, "a16") || !strcmp(op, "n16")) {
		sprintf(buffer, "%s[$%04X]", op,
			cpu->memory->load16(cpu, indice + 1));
	} else if (!strcmp(op, "a8") || !strcmp(op, "n8")) {
		sprintf(buffer, "%s[$%02X]", op,
			cpu->memory->load8(cpu, indice + 1));
	} else if (!strcmp(op, "e8")) {
		u8 byte = cpu->memory->load8(cpu, indice + 1);
		s8 offset = (s8)byte;
		sprintf(buffer, "%s[$%02X] [%d]", op, byte, offset);
	} else {
		sprintf(buffer, "%s", op);
	}
	return buffer;
}

void sm83_cpu_debug(struct sm83_core *cpu)
{
	char *decoded = NULL;

	printf("     Z = %d | N = %d\n", cpu_flag_is_set(cpu, FLAG_Z),
	       cpu_flag_is_set(cpu, FLAG_N));
	printf("     H = %d | C = %d\n", cpu_flag_is_set(cpu, FLAG_H),
	       cpu_flag_is_set(cpu, FLAG_C));
	printf("  A = $%02X  |  F = $%02X\n", cpu->a, cpu->f);
	printf("  %08b | %08b\n", cpu->a, cpu->f);
	printf("  B = $%02X  |  C = $%02X\n", cpu->b, cpu->c);
	printf("  %08b | %08b\n", cpu->b, cpu->c);
	printf("  D = $%02X  |  E = $%02X\n", cpu->d, cpu->e);
	printf("  %08b | %08b\n", cpu->d, cpu->e);
	printf("  H = $%02X  |  L = $%02X\n", cpu->h, cpu->l);
	printf("  %08b | %08b\n", cpu->h, cpu->l);
	printf("      SP = $%04X\n", cpu->sp);
	printf("   %016b\n", cpu->sp);
	printf("      PC = $%04X\n", cpu->pc);
	printf("   %016b\n", cpu->pc);
	printf("  IME = %d  | HALT = %d\n", cpu->ime, cpu->halted);
	printf("  DIV = %d  | TIMA = %d\n", cpu->memory->load8(cpu, 0xFF04),
	       cpu->memory->load8(cpu, 0xFF05));
	printf("  M-cycles = %lu | T-cycles = %lu\n", cpu->cycles,
	       cpu->cycles * 4);
	decoded = sm83_disassemble(cpu);
	printf("%s\n", decoded);
	zfree(decoded);
}

void sm83_memory_io_debug(struct sm83_core *cpu)
{
	for (int i = 0xFF00; i <= 0xFFFF; i++) {
		u8 byte = cpu->memory->load8(cpu, i);
		printf("%04X : %02X [%08b] %d\n", i, byte, byte, byte);
	}
}

void sm83_memory_debug(struct sm83_core *cpu, u16 start, u16 end)
{
	for (int i = start; i <= end; i++) {
		if (cpu->memory->load8(cpu, start + i) != 0)
			printf("%02X", cpu->memory->load8(cpu, start + i));
		else
			printf("..");
		if ((i + 1) % 32 == 0 && i > 0)
			printf("\n");
		else if ((i + 1) % 8 == 0 && i > 0)
			printf(" ");
	}
	printf("\n");
}

char *sm83_disassemble(struct sm83_core *cpu)
{
	char *buffer;

	buffer = (char *)calloc(256, sizeof(char));
	if (!buffer)
		return NULL;
	sprintf(buffer + strlen(buffer), "00:%04X", cpu->index);
	for (int i = 0; i < cpu->instruction.length; i++) {
		sprintf(buffer + strlen(buffer), " %02X",
			cpu->memory->load8(cpu, cpu->index + i));
	}
	sprintf(buffer + strlen(buffer), " -> ");
	if (cpu->instruction.op1 && cpu->instruction.op2) {
		char *op1 = sm83_resolve_operand(cpu, cpu->instruction.op1,
						 cpu->index);
		char *op2 = sm83_resolve_operand(cpu, cpu->instruction.op2,
						 cpu->index);
		sprintf(buffer + strlen(buffer), "%s %s %s",
			cpu->instruction.mnemonic, op1, op2);
		zfree(op1);
		zfree(op2);
	} else if (cpu->instruction.op1) {
		char *op1 = sm83_resolve_operand(cpu, cpu->instruction.op1,
						 cpu->index);
		sprintf(buffer + strlen(buffer), "%s %s",
			cpu->instruction.mnemonic, op1);
		zfree(op1);
	} else {
		sprintf(buffer + strlen(buffer), "%s",
			cpu->instruction.mnemonic);
	}
	return buffer;
}
