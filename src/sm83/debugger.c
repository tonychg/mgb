#include "gb/alloc.h"
#include "gb/list.h"
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

#define COMMAND_MAX_LENGTH 256

enum debugger_command_type {
	DEBUG_NEXT,
	DEBUG_STEP,
	DEBUG_BREAKPOINT,
	DEBUG_CONTINUE,
	DEBUG_PRINT,
	DEBUG_LOOP,
	DEBUG_RANGE,
	DEBUG_GOTO,
	DEBUG_DUMP,
	DEBUG_IO,
	DEBUG_SET,
	DEBUG_RESET,
	DEBUG_REGISTERS,
	DEBUG_FRAME,
	DEBUG_QUIT,
	DEBUG_HELP,
	DEBUG_WATCH,
};

struct cmd_struct {
	const char *description;
	const char *cmd;
	const char *alias;
	enum debugger_command_type type;
};

// clang-format off
static struct cmd_struct commands[] = {
	{ "next (n)                Next instruction\n", "next", "n", DEBUG_NEXT },
	{ "step (s)                Step one M-cycle\n", "step", "s", DEBUG_STEP },
	{ "break (b) <addr>        Set a breakpoint\n", "break", "b", DEBUG_BREAKPOINT },
	{ "continue (c)            Continue until next breakpoint\n", "continue", "c", DEBUG_CONTINUE },
	{ "print (p) <addr>        Print address value\n", "print", "p", DEBUG_PRINT },
	{ "range (r) <addr> <addr> Dump memory range\n", "range", "r", DEBUG_RANGE },
	{ "loop (l) <counter>      Loop a number of iteration\n", "loop", "l", DEBUG_LOOP },
	{ "goto (g) <addr>         Go to addr\n", "goto", "g", DEBUG_GOTO },
	{ "dump (d)                Dump memory\n", "dump", "d", DEBUG_DUMP },
	{ "regs (R)                Print registers\n", "regs", "R", DEBUG_REGISTERS },
	{ "io (i)                  Dump I/O ranges\n", "io", "i", DEBUG_IO },
	{ "frame (f)               Next frame\n", "frame", "f", DEBUG_FRAME },
	{ "set (s) <addr> <value>  Set value\n", "set", "s", DEBUG_SET },
	{ "reset (r)               Reset\n", "reset", "r", DEBUG_RESET },
	{ "quit (q)                Quit\n", "quit", "q", DEBUG_QUIT },
	{ "help (h)                Display this message\n", "help", "h", DEBUG_HELP },
	{ "watch (w) <addr>        Watch address\n", "watch", "w", DEBUG_WATCH },
};
// clang-format on

struct debugger_command {
	enum debugger_command_type type;
	u16 addr;
	u8 value;
	u16 start;
	u16 end;
	u32 counter;
};

void parse_command_args(char *buffer, struct debugger_command *cmd)
{
	switch (cmd->type) {
	case DEBUG_NEXT:
	case DEBUG_STEP:
	case DEBUG_DUMP:
	case DEBUG_IO:
	case DEBUG_RESET:
	case DEBUG_REGISTERS:
	case DEBUG_QUIT:
	case DEBUG_HELP:
	case DEBUG_FRAME:
	case DEBUG_CONTINUE:
		break;
	case DEBUG_LOOP:
		cmd->counter = atoi(strtok(NULL, " "));
		break;
	case DEBUG_BREAKPOINT:
	case DEBUG_PRINT:
	case DEBUG_WATCH:
	case DEBUG_GOTO:
		cmd->addr = strtol(strtok(NULL, " "), NULL, 16);
		break;
	case DEBUG_SET:
		cmd->addr = strtol(strtok(NULL, " "), NULL, 16);
		cmd->value = strtol(strtok(NULL, " "), NULL, 16);
		break;
	case DEBUG_RANGE:
		cmd->start = strtol(strtok(NULL, " "), NULL, 16);
		cmd->end = strtol(strtok(NULL, " "), NULL, 16);
		break;
	default:
		cmd->type = DEBUG_NEXT;
		break;
	}
}

void command_match(char *buffer, struct debugger_command *cmd)
{
	for (int i = 0; i < ARRAY_SIZE(commands); i++) {
		struct cmd_struct s = commands[i];
		if (match_command(s.cmd, s.alias, buffer)) {
			cmd->type = s.type;
		}
	}
}

static struct debugger_command *parse_command(char *buffer)
{
	char *command;
	struct debugger_command *cmd;

	cmd = (struct debugger_command *)malloc(
		sizeof(struct debugger_command));
	if (!cmd)
		return NULL;
	if (strlen(buffer) == 1)
		goto command_default;
	command = strtok(buffer, " \n");
	command_match(command, cmd);
	parse_command_args(command, cmd);
	return cmd;
command_default:
	cmd->type = DEBUG_NEXT;
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

void debugger_help()
{
	for (int i = 0; i < ARRAY_SIZE(commands); i++) {
		struct cmd_struct cmd = commands[i];
		printf("%s", cmd.description);
	}
	printf("Address are in hexadecimal format\n");
}

void watch_value(struct sm83_core *cpu, u16 addr, u8 *previous)
{
	u8 value = cpu->memory->load8(cpu, addr);
	if (value != *previous)
		printf("CHANGED! [%04X] %02X -> %02X\n", addr, *previous,
		       value);
	*previous = value;
}

void debugger_step(struct sm83_core *cpu, u16 *watcher, u8 *watcher_value)
{
	sm83_cpu_step(cpu);
	if (*watcher)
		watch_value(cpu, *watcher, watcher_value);
}

enum debugger_state {
	DEBUGGER_STATE_STOP,
	DEBUGGER_STATE_COMMAND,
	DEBUGGER_STATE_EXECUTE,
};

struct debugger_watcher {
	u16 addr;
	u16 previous;
};

struct debugger_breakpoint {
	u16 addr;
};

void register_breakpoint(struct list *breakpoints, u16 addr)
{
	struct debugger_breakpoint *breakpoint;

	breakpoint = (struct debugger_breakpoint*)malloc(sizeof(struct debugger_breakpoint));
	if (!breakpoint) {
		printf("Failed to register breakpoint %04X\n", addr);
	} else {
		breakpoint->addr = addr;
		list_add_node_head(breakpoints, breakpoint);
	}
}

void register_watcher(struct list *watchers, u16 addr, u8 value)
{
	struct debugger_watcher *watcher;

	watcher = (struct debugger_watcher*)malloc(sizeof(struct debugger_watcher));
	if (!watcher) {
		printf("Failed to register watcher %04X\n", addr);
	} else {
		watcher->addr = addr;
		watcher->previous = value;
		list_add_node_head(watchers, watcher);
	}
}

struct debugger_context {
	u16 index;
	u64 until;
	u16 next;
	struct list *breakpoints;
	struct list *watchers;
	struct debugger_command *cmd;
	enum debugger_state state;
};

struct debugger_context *debugger_context_init()
{
	struct debugger_context *ctx;

	ctx = (struct debugger_context *)malloc(
		sizeof(struct debugger_context));
	if (!ctx)
		return NULL;
	ctx->breakpoints = list_create();
	ctx->breakpoints->free = zfree;
	if (!ctx->breakpoints)
		goto err;
	ctx->watchers = list_create();
	ctx->watchers->free = zfree;
	if (!ctx->watchers)
		goto err;
	ctx->index = 0;
	ctx->until = 0;
	ctx->next = 0;
	ctx->state = DEBUGGER_STATE_COMMAND;
	return ctx;
err:
	zfree(ctx->watchers);
	zfree(ctx->breakpoints);
	zfree(ctx);
	return NULL;
}

void debugger_context_destroy(struct debugger_context *ctx)
{
	list_release(ctx->breakpoints);
	list_release(ctx->watchers);
	zfree(ctx);
}

void debugger_command_hook(struct debugger_context *ctx, struct sm83_core *cpu)
{
	ctx->cmd = debugger_readline();
	if (!ctx->cmd) {
		sm83_printd("Failed to parse command");
	}
	switch (ctx->cmd->type) {
	case DEBUG_HELP:
		debugger_help();
		break;
	case DEBUG_FRAME:
		ctx->until = (u64)(SM83_FREQ / 59.73);
		ctx->state = DEBUGGER_STATE_EXECUTE;
		break;
	case DEBUG_REGISTERS:
		sm83_cpu_debug(cpu);
		break;
	case DEBUG_STEP:
		ctx->until = 1;
		ctx->state = DEBUGGER_STATE_EXECUTE;
		break;
	case DEBUG_NEXT:
		ctx->state = DEBUGGER_STATE_EXECUTE;
		break;
	case DEBUG_LOOP:
		ctx->until = ctx->cmd->counter;
		ctx->state = DEBUGGER_STATE_EXECUTE;
		break;
	case DEBUG_GOTO:
		ctx->next = ctx->cmd->addr;
		ctx->state = DEBUGGER_STATE_EXECUTE;
		break;
	case DEBUG_DUMP:
		sm83_memory_debug(cpu, 0x0000, 0xFFFF);
		break;
	case DEBUG_IO:
		sm83_memory_io_debug(cpu);
		break;
	case DEBUG_SET:
		cpu->memory->write8(cpu, ctx->cmd->addr, ctx->cmd->value);
		break;
	case DEBUG_CONTINUE:
		ctx->state = DEBUGGER_STATE_EXECUTE;
		break;
	case DEBUG_BREAKPOINT:
		register_breakpoint(ctx->breakpoints, ctx->cmd->addr);
		printf("Breakpoint $%04X\n", ctx->cmd->addr);
		break;
	case DEBUG_PRINT:
		printf("$%04X -> %02X\n", ctx->cmd->addr,
		       cpu->memory->load8(cpu, ctx->cmd->addr));
		break;
	case DEBUG_RESET:
		sm83_cpu_reset(cpu);
		sm83_cpu_debug(cpu);
		break;
	case DEBUG_RANGE:
		sm83_memory_debug(cpu, ctx->cmd->start, ctx->cmd->end);
		break;
	case DEBUG_QUIT:
		ctx->state = DEBUGGER_STATE_STOP;
		break;
	case DEBUG_WATCH:
		list_add_node_head(ctx->watchers, &(ctx->cmd->addr));
		break;
	}
	zfree(ctx->cmd);
}

void debugger_cpu_step(struct debugger_context *ctx, struct sm83_core *cpu)
{
	while (ctx->state == DEBUGGER_STATE_COMMAND)
		debugger_command_hook(ctx, cpu);
	ctx->index = cpu->index;
	sm83_cpu_step(cpu);
	for (struct list_node *node = ctx->breakpoints->head; node != NULL;
	     node = node->next) {
		u16 addr = *(u16*)node->data;
		printf("Check %04X\n", addr);
		if (ctx->index == addr)
			ctx->state = DEBUGGER_STATE_COMMAND;
	}
	if (ctx->next == cpu->index)
		ctx->state = DEBUGGER_STATE_COMMAND;
	if (ctx->until > 0)
		ctx->until--;
	// Handle watch points
	// if (*watcher)
	// 	watch_value(cpu, *watcher, watcher_value);
}

void debugger_mainloop(struct debugger_context *ctx, struct sm83_core *cpu)
{
	sm83_cpu_step(cpu);
	sm83_cpu_debug(cpu);
	while (ctx->state != DEBUGGER_STATE_STOP) {
		printf("Current state: %d, Index: %04X\n", ctx->state, ctx->index);
		debugger_cpu_step(ctx, cpu);
	}
}

void debugger_event_loop(struct sm83_core *cpu)
{
	bool debugger_running = true;
	struct debugger_command *cmd;
	u16 index;
	u16 breakpoint = 0;
	u16 watcher = 0;
	u8 watcher_value = 0;

	while (debugger_running) {
		cmd = debugger_readline();
		if (!cmd) {
			sm83_printd("Failed to parse command");
			continue;
		}
		index = cpu->index;
		switch (cmd->type) {
		case DEBUG_HELP:
			debugger_help();
			break;
		case DEBUG_FRAME:
			for (u64 i = 0; i < (u64)(SM83_FREQ / 59.73); i++) {
				debugger_step(cpu, &watcher, &watcher_value);
			}
			sm83_cpu_debug(cpu);
			break;
		case DEBUG_REGISTERS:
			sm83_cpu_debug(cpu);
			break;
		case DEBUG_STEP:
			debugger_step(cpu, &watcher, &watcher_value);
			break;
		case DEBUG_NEXT:
			do {
				debugger_step(cpu, &watcher, &watcher_value);
			} while (index == cpu->index);
			sm83_cpu_debug(cpu);
			break;
		case DEBUG_LOOP:
			for (int i = 0; i < cmd->counter; i++) {
				u16 idx = cpu->index;
				do {
					debugger_step(cpu, &watcher,
						      &watcher_value);
				} while (cpu->index == idx);
			}
			break;
		case DEBUG_GOTO:
			do {
				debugger_step(cpu, &watcher, &watcher_value);
			} while (cpu->index != cmd->addr);
			sm83_cpu_debug(cpu);
			break;
		case DEBUG_DUMP:
			sm83_memory_debug(cpu, 0x0000, 0xFFFF);
			break;
		case DEBUG_IO:
			sm83_memory_io_debug(cpu);
			break;
		case DEBUG_SET:
			cpu->memory->write8(cpu, cmd->addr, cmd->value);
			break;
		case DEBUG_CONTINUE:
			do {
				debugger_step(cpu, &watcher, &watcher_value);
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
		case DEBUG_RANGE:
			sm83_memory_debug(cpu, cmd->start, cmd->end);
			break;
		case DEBUG_QUIT:
			debugger_running = false;
			break;
		case DEBUG_WATCH:
			watcher = cmd->addr;
			break;
		}
		zfree(cmd);
	}
}

void debugger_start(char *rom_path)
{
	u8 *rom;
	struct sm83_debugger *debugger;
	struct debugger_context *debugger_context;

	rom = readfile(rom_path);
	if (!rom) {
		sm83_printd("Failed to open");
		goto exit;
	}
	debugger = sm83_debugger_init(rom);
	debugger_context = debugger_context_init();
	if (!debugger_context) {
		sm83_printd("Failed to init debugger context");
		goto free_rom;
	}
	if (!debugger) {
		sm83_printd("Failed init debugger");
		goto free_rom;
	}
	debugger_mainloop(debugger_context, debugger->cpu);
	sm83_debugger_destroy(debugger);
	debugger_context_destroy(debugger_context);
free_rom:
	zfree(rom);
exit:
	return;
}
