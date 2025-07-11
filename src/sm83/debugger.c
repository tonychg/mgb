#include "gb/alloc.h"
#include "gb/sm83.h"
#include "gb/types.h"
#include "gb/fs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void sm83_printd(const char *msg)
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

static u8 load8(struct sm83_core *cpu, u16 addr)
{
	u8 value;
	struct sm83_debugger *debugger = (struct sm83_debugger *)cpu->parent;

	if (debugger->rom && addr < 0x8000)
		value = debugger->rom[addr];
	else
		value = debugger->bus[addr];
	return value;
}

static void write8(struct sm83_core *cpu, u16 addr, u8 value)
{
	struct sm83_debugger *debugger = (struct sm83_debugger *)cpu->parent;

	if (debugger->rom && addr >= 0x8000)
		debugger->bus[addr] = value;
	else
		debugger->bus[addr] = value;
}

static u16 load16(struct sm83_core *cpu, u16 addr)
{
	return unsigned_16(cpu->memory->load8(cpu, addr),
			   cpu->memory->load8(cpu, addr + 1));
}

static u8 read_segment(struct sm83_core *cpu)
{
	return cpu->memory->load8(cpu, cpu->pc);
}

struct sm83_debugger *sm83_debugger_alloc()
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
	debugger->breakpoint = 0;
	return debugger;

free_cpu:
	zfree(debugger);
free_debugger:
	sm83_destroy(debugger->cpu);
error:
	return NULL;
}

struct sm83_debugger *sm83_debugger_init(u8 *rom)
{
	struct sm83_debugger *debugger;

	debugger = sm83_debugger_alloc();
	if (!debugger)
		return NULL;
	debugger->rom = rom;
	debugger->cpu->parent = debugger;
	debugger->cpu->memory->load8 = &load8;
	debugger->cpu->memory->write8 = &write8;
	debugger->cpu->memory->load16 = &load16;
	debugger->cpu->memory->read_segment = &read_segment;
	return debugger;
}

void sm83_debugger_destroy(struct sm83_debugger *debugger)
{
	sm83_destroy(debugger->cpu);
	zfree(debugger->bus);
	zfree(debugger);
}

#define COMMAND_MAX_LENGTH 256

enum debugger_command_type {
	DEBUG_NEXT,
	DEBUG_STEP,
	DEBUG_CONTINUE,
	DEBUG_LOOP,
	DEBUG_BREAKPOINT,
	DEBUG_PRINT,
	DEBUG_RESET,
	DEBUG_QUIT,
};

struct debugger_command {
	enum debugger_command_type type;
	u16 addr;
	u32 counter;
};

static bool match_command(const char *name, const char *alias,
			  const char *input)
{
	bool match = false;

	if (alias) {
		match = !strcmp(name, input) || !strcmp(alias, input);
	} else {
		match = !strcmp(name, input);
	}
	return match;
}

static struct debugger_command *parse_command(char *buffer)
{
	int i;
	char command[COMMAND_MAX_LENGTH] = { 0 };
	struct debugger_command *cmd;

	cmd = (struct debugger_command *)malloc(
		sizeof(struct debugger_command));
	if (!cmd)
		return NULL;
	for (i = 0;
	     i < COMMAND_MAX_LENGTH && buffer[i] != ' ' && buffer[i] != '\n';
	     i++) {
		command[i] = buffer[i];
	}
	if (match_command("next", "n", command)) {
		cmd->type = DEBUG_NEXT;
	} else if (match_command("step", "s", command)) {
		cmd->type = DEBUG_STEP;
	} else if (match_command("continue", "c", command)) {
		cmd->type = DEBUG_CONTINUE;
	} else if (match_command("breakpoint", "b", command)) {
		cmd->type = DEBUG_BREAKPOINT;
		cmd->addr = strtol(buffer + i + 1, NULL, 16);
	} else if (match_command("print", "p", command)) {
		cmd->type = DEBUG_PRINT;
		cmd->addr = strtol(buffer + i + 1, NULL, 16);
	} else if (match_command("loop", "l", command)) {
		cmd->type = DEBUG_LOOP;
		cmd->counter = atoi(buffer + i + 1);
	} else if (match_command("reset", "r", command)) {
		cmd->type = DEBUG_RESET;
	} else if (match_command("quit", "q", command)) {
		cmd->type = DEBUG_QUIT;
	} else {
		cmd->type = DEBUG_NEXT;
	}
	return cmd;
}

static struct debugger_command *debugger_readline(void)
{
	char *buffer;
	struct debugger_command *cmd;

	buffer = (char *)calloc(COMMAND_MAX_LENGTH, sizeof(char));
	if (!buffer)
		return NULL;
	printf("> ");
	if (!fgets(buffer, COMMAND_MAX_LENGTH, stdin))
		return NULL;
	cmd = parse_command(buffer);
	zfree(buffer);
	return cmd;
}

void debugger_event_loop(struct sm83_core *cpu)
{
	bool debugger_running = true;
	struct debugger_command *cmd;
	u16 index;
	u16 breakpoint = 0;

	while (debugger_running) {
		cmd = debugger_readline();
		if (!cmd) {
			sm83_printd("Failed to parse command");
			continue;
		}
		index = cpu->index;
		switch (cmd->type) {
		case DEBUG_STEP:
			sm83_cpu_step(cpu);
			sm83_cpu_debug(cpu);
			break;
		case DEBUG_NEXT:
			do {
				sm83_cpu_step(cpu);
			} while (index == cpu->index);
			sm83_cpu_debug(cpu);
			break;
		case DEBUG_LOOP:
			for (int i = 0; i < cmd->counter; i++) {
				u16 idx = cpu->index;
				do {
					sm83_cpu_step(cpu);
				} while (cpu->index == idx);
			}
			sm83_cpu_debug(cpu);
			break;
		case DEBUG_CONTINUE:
			do {
				sm83_cpu_step(cpu);
			} while (cpu->index != breakpoint);
			sm83_cpu_debug(cpu);
			break;
		case DEBUG_BREAKPOINT:
			breakpoint = cmd->addr;
			printf("Breakpoint $%04X\n", cmd->addr);
			break;
		case DEBUG_PRINT:
			printf("$%04X -> %02X\n", cmd->addr,
			       cpu->memory->load8(cpu, cmd->addr));
			break;
		case DEBUG_RESET:
			sm83_cpu_reset(cpu);
			sm83_cpu_debug(cpu);
			break;
		case DEBUG_QUIT:
			debugger_running = false;
			break;
		}
		zfree(cmd);
	}
}

void debugger_start(char *rom_path)
{
	u8 *rom;
	struct sm83_debugger *debugger;

	rom = readfile(rom_path);
	if (!rom) {
		sm83_printd("Failed to open");
		goto exit;
	}
	debugger = sm83_debugger_init(rom);
	if (!debugger) {
		sm83_printd("Failed init debugger");
		goto free_rom;
	}
	sm83_cpu_step(debugger->cpu);
	sm83_cpu_debug(debugger->cpu);
	debugger_event_loop(debugger->cpu);
	sm83_debugger_destroy(debugger);

free_rom:
	zfree(rom);
exit:
	return;
}
