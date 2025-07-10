#include "gb/alloc.h"
#include "gb/sm83.h"
#include "gb/types.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void printd(const char *msg)
{
	printf("[sm83] msg=%s\n", msg);
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

char *sm83_disassemble(struct sm83_core *cpu)
{
	char *buffer;

	buffer = (char *)calloc(256, sizeof(char));
	if (!buffer)
		return NULL;
	sprintf(buffer + strlen(buffer), "00:%04X", cpu->pc);
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

struct sm83_debugger {
	struct sm83_core *cpu;
	u8 *bus;
	u8 *rom;
};

static u8 load8(struct sm83_core *cpu, u16 addr)
{
	u8 value;
	struct sm83_debugger *debugger = (struct sm83_debugger *)cpu->parent;

	if (addr < 0x8000)
		value = debugger->rom[addr];
	else
		value = debugger->bus[addr];
	return value;
}

static void write8(struct sm83_core *cpu, u16 addr, u8 value)
{
	struct sm83_debugger *debugger = (struct sm83_debugger *)cpu->parent;

	if (addr >= 0x8000)
		debugger->bus[addr] = value;
}

static u16 load16(struct sm83_core *cpu, u16 addr)
{
	return unsigned_16(cpu->memory->load8(cpu, addr),
			   cpu->memory->load8(cpu, addr + 1));
}

static s8 loads8(struct sm83_core *cpu)
{
	return (cpu->pc + 1) + (s8)cpu->memory->load8(cpu, cpu->pc);
}

static u8 read_segment(struct sm83_core *cpu)
{
	return cpu->memory->load8(cpu, cpu->pc);
}

static struct sm83_debugger *sm83_debugger_alloc()
{
	struct sm83_debugger *debugger;

	debugger = (struct sm83_debugger *)malloc(sizeof(struct sm83_debugger));
	if (!debugger)
		goto error;
	debugger->cpu = sm83_init();
	if (!debugger->cpu)
		goto free_debugger;
	debugger->bus = (u8 *)calloc(0xFFFF, sizeof(u8));
	if (!debugger->bus) {
		goto free_cpu;
	}
	debugger->rom = NULL;
	return debugger;

free_cpu:
	zfree(debugger);
free_debugger:
	sm83_destroy(debugger->cpu);
error:
	return NULL;
}

static struct sm83_debugger *sm83_debugger_init(u8 *rom)
{
	struct sm83_debugger *debugger;

	debugger = sm83_debugger_alloc();
	if (!debugger)
		return NULL;
	debugger->rom = rom;
	debugger->cpu->parent = debugger;
	debugger->cpu->memory->load8 = &load8;
	debugger->cpu->memory->loads8 = &loads8;
	debugger->cpu->memory->write8 = &write8;
	debugger->cpu->memory->load16 = &load16;
	debugger->cpu->memory->read_segment = &read_segment;
	return debugger;
}

void sm83_debugger_start(u8 *rom)
{
	struct sm83_debugger *debugger = NULL;

	debugger = sm83_debugger_init(rom);
	if (!debugger)
		printd("Failed to init debugger");
	for (int cycle = 0; cycle < 50; cycle++) {
		sm83_cpu_step(debugger->cpu);
		sm83_cpu_debug(debugger->cpu);
	}
}
