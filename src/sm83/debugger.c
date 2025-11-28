#include "gb/option.h"
#include "gb/debugger.h"
#include "gb/alloc.h"
#include "gb/sm83.h"
#include "gb/types.h"
#include "gb/fs.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static volatile int sigint_catcher = 0;

static u8 load8(struct sm83_core *cpu, u16 addr)
{
	struct debugger_context *ctx = (struct debugger_context *)cpu->parent;
	return ctx->memory[addr];
}

static void write8(struct sm83_core *cpu, u16 addr, u8 value)
{
	struct debugger_context *ctx = (struct debugger_context *)cpu->parent;
	ctx->memory[addr] = value;
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

static void dump_memory(u8 memory[])
{
	for (int i = 0; i <= MEMORY_SIZE; i++) {
		if (memory[i] != 0)
			printf("%02X", memory[i]);
		else
			printf("..");
		if ((i + 1) % 32 == 0 && i > 0)
			printf("\n");
		else if ((i + 1) % 8 == 0 && i > 0)
			printf(" ");
	}
}

// clang-format off
static struct cmd_struct commands[] = {
	{ "next (n)                    Next instruction\n", "next", "n", COMMAND_NEXT },
	{ "step (s)                    Step one M-cycle\n", "step", "s", COMMAND_STEP },
	{ "break (b) <addr>            Set a breakpoint\n", "break", "b", COMMAND_BREAKPOINT },
	{ "breakpoints (bs)            List breakpoints\n", "breakpoints", "bs", COMMAND_BREAKPOINTS },
	{ "del (d)                     Delete breakpoint\n", "del", "d", COMMAND_BREAKPOINT_DELETE },
	{ "continue (c)                Continue until next breakpoint\n", "continue", "c", COMMAND_CONTINUE },
	{ "print (p) <addr>            Print address value\n", "print", "p", COMMAND_PRINT },
	{ "range (r) <addr> <addr>     Dump memory range\n", "range", "r", COMMAND_RANGE },
	{ "loop (l) <counter>          Loop a number of iteration\n", "loop", "l", COMMAND_LOOP },
	{ "goto (g) <addr>             Go to addr\n", "goto", "g", COMMAND_GOTO },
	{ "mem (m)                     Dump memory\n", "mem", "m", COMMAND_MEM },
	{ "regs (R)                    Print registers\n", "regs", "R", COMMAND_REGISTERS },
	{ "io (i)                      Dump I/O ranges\n", "io", "i", COMMAND_IO },
	{ "frame (f)                   Next frame\n", "frame", "f", COMMAND_FRAME },
	{ "set (s) <addr> <value>      Set value\n", "set", "s", COMMAND_SET },
	{ "reset (r)                   Reset\n", "reset", "r", COMMAND_RESET },
	{ "quit (q)                    Quit\n", "quit", "q", COMMAND_QUIT },
	{ "help (h)                    Display this message\n", "help", "h", COMMAND_HELP },
	{ "watch (w) <addr>            Watch address\n", "watch", "w", COMMAND_WATCH },
	{ "wdelete (wd) <addr>         Delete watched\n", "wdelete", "wd", COMMAND_WATCH_DELETE },
	{ "wlist (wl)                  List watchers\n", "wlist", "wl", COMMAND_WATCH_LIST },
};
// clang-format on

static void print_help()
{
	for (int i = 0; i < ARRAY_SIZE(commands); i++) {
		struct cmd_struct cmd = commands[i];
		printf("%s", cmd.description);
	}
	printf("Address are in hexadecimal format\n");
}

static int debugger_load_rom(struct debugger_context *ctx, char *path)
{
	FILE *file;
	size_t size_in_bytes;
	file = fopen(path, "r");
	if (!file)
		return -1;
	size_in_bytes = fs_size(file);
	if (size_in_bytes > MEMORY_SIZE)
		return -1;
	for (int i = 0; i <= 0xFFFF; i++)
		ctx->memory[i] = 0;
	if (fread(ctx->memory, sizeof(u8), 0xFFFF, file) == 0)
		return -1;
	return 0;
}

static bool command_match(struct cmd_struct cmd, char *input)
{
	bool is_match = false;
	if (!strcmp(cmd.name, input)) {
		is_match = true;
	}
	if (cmd.alias && !strcmp(cmd.alias, input)) {
		is_match = true;
	}
	return is_match;
}

static int parse_hex(struct debugger_context *ctx, char **buffer)
{
	char option[COMMAND_MAX_LENGTH] = "";

	if (get_option(buffer, option, COMMAND_DELIMITERS) || !strlen(option)) {
		ctx->command.type = COMMAND_HELP;
		return 0;
	}
	return strtol(option, NULL, 16);
}

static int parse_int(struct debugger_context *ctx, char **buffer)
{
	char option[COMMAND_MAX_LENGTH] = "";

	if (get_option(buffer, option, COMMAND_DELIMITERS) || !strlen(option)) {
		ctx->command.type = COMMAND_HELP;
		return 0;
	}
	return atoi(option);
}

static int command_parse(struct debugger_context *ctx, char *buffer)
{
	char option[COMMAND_MAX_LENGTH] = "";

	get_option(&buffer, option, COMMAND_DELIMITERS);
	ctx->command.type = COMMAND_NEXT;
	for (int i = 0; i < ARRAY_SIZE(commands); i++) {
		struct cmd_struct s = commands[i];
		if (command_match(s, option)) {
			ctx->command.type = s.type;
			break;
		}
	}
	switch (ctx->command.type) {
	case COMMAND_NEXT:
	case COMMAND_STEP:
	case COMMAND_MEM:
	case COMMAND_IO:
	case COMMAND_RESET:
	case COMMAND_REGISTERS:
	case COMMAND_QUIT:
	case COMMAND_HELP:
	case COMMAND_FRAME:
	case COMMAND_CONTINUE:
	case COMMAND_BREAKPOINTS:
	case COMMAND_WATCH_LIST:
		break;
	case COMMAND_BREAKPOINT:
	case COMMAND_BREAKPOINT_DELETE:
	case COMMAND_PRINT:
	case COMMAND_WATCH:
	case COMMAND_WATCH_DELETE:
	case COMMAND_GOTO:
		ctx->command.addr = parse_hex(ctx, &buffer);
		break;
	case COMMAND_LOOP:
		ctx->command.counter = parse_int(ctx, &buffer);
		break;
	case COMMAND_SET:
		ctx->command.addr = parse_hex(ctx, &buffer);
		ctx->command.value = parse_hex(ctx, &buffer);
		break;
	case COMMAND_RANGE:
		ctx->command.addr = parse_hex(ctx, &buffer);
		ctx->command.end = parse_hex(ctx, &buffer);
		break;
	}
	return 0;
}

static int debugger_prompt(struct debugger_context *ctx)
{
	char buffer[COMMAND_MAX_LENGTH];
	printf("> ");
	if (!fgets(buffer, COMMAND_MAX_LENGTH, stdin)) {
		return -1;
	}
	if (command_parse(ctx, buffer)) {
		return -1;
	}
	return 0;
}

static void move_to_wait(struct debugger_context *ctx)
{
	sm83_cpu_debug(ctx->cpu);
	ctx->state = STATE_WAIT;
}

static int register_breakpoint(struct debugger_context *ctx, u16 addr)
{
	for (int i = 0; i < MAX_BREAKPOINTS; i++) {
		if (ctx->breakpoints[i] == 0) {
			printf("New breakpoint $%04X\n", addr);
			ctx->breakpoints[i] = addr;
			return 0;
		}
	}
	return -1;
}

static int unregister_breakpoint(struct debugger_context *ctx, u16 addr)
{
	for (int i = 0; i < MAX_BREAKPOINTS; i++) {
		if (ctx->breakpoints[i] == addr) {
			ctx->breakpoints[i] = 0;
			return 0;
		}
	}
	return -1;
}

static void check_breakpoints(struct debugger_context *ctx)
{
	for (int i = 0; i < MAX_BREAKPOINTS; i++) {
		if (ctx->breakpoints[i]) {
			if (ctx->index == ctx->breakpoints[i])
				move_to_wait(ctx);
		}
	}
}

static int register_watcher(struct debugger_context *ctx, u16 addr)
{
	for (int i = 0; i < MAX_WATCHERS; i++) {
		if (!ctx->watched_addresses[i]) {
			ctx->watched_addresses[i] = addr;
			ctx->watched_values[i] = ctx->memory[addr];
			printf("New watcher $%04X\n", addr);
			return 0;
		}
	}
	return -1;
}

static int unregister_watcher(struct debugger_context *ctx, u16 addr)
{
	for (int i = 0; i < MAX_WATCHERS; i++) {
		if (ctx->watched_addresses[i] == addr) {
			ctx->watched_addresses[i] = 0;
			ctx->watched_values[i] = 0;
			return 0;
		}
	}
	return -1;
}

static void check_watchers(struct debugger_context *ctx)
{
	for (int i = 0; i < MAX_WATCHERS; i++) {
		if (ctx->watched_addresses[i]) {
			u16 addr = ctx->watched_addresses[i];
			u8 value = ctx->watched_values[i];
			if (ctx->memory[addr] != value) {
				printf("Changed [$%04X] $%02X -> $%02X\n", addr,
				       value, ctx->memory[addr]);
				ctx->watched_values[i] = ctx->memory[addr];
			}
		}
	}
}

static int debugger_command_handle(struct debugger_context *ctx)
{
	switch (ctx->command.type) {
	case COMMAND_NEXT:
		if (ctx->state == STATE_EXECUTE &&
		    ctx->cpu->index != ctx->index)
			move_to_wait(ctx);
		else
			ctx->state = STATE_EXECUTE;
		break;
	case COMMAND_STEP:
		if (ctx->state == STATE_EXECUTE)
			move_to_wait(ctx);
		else
			ctx->state = STATE_EXECUTE;
		break;
	case COMMAND_BREAKPOINT:
		if (register_breakpoint(ctx, ctx->command.addr))
			printf("Failed to register new breakpoints, remove some");
		break;
	case COMMAND_BREAKPOINTS:
		for (int i = 0; i < MAX_BREAKPOINTS; i++) {
			if (ctx->breakpoints[i])
				printf("$%04X\n", ctx->breakpoints[i]);
		}
		break;
	case COMMAND_BREAKPOINT_DELETE:
		if (unregister_breakpoint(ctx, ctx->command.addr))
			printf("Breakpoint not found");
		else
			printf("$%04X deleted\n", ctx->command.addr);
		break;
	case COMMAND_CONTINUE:
		if (ctx->state == STATE_EXECUTE)
			check_breakpoints(ctx);
		else
			ctx->state = STATE_EXECUTE;
		break;
	case COMMAND_PRINT:
		printf("$%02X [%08b] %d\n", ctx->memory[ctx->command.addr],
		       ctx->memory[ctx->command.addr],
		       ctx->memory[ctx->command.addr]);
		break;
	case COMMAND_LOOP:
		break;
	case COMMAND_RANGE:
		break;
	case COMMAND_GOTO:
		break;
	case COMMAND_MEM:
		dump_memory(ctx->memory);
		break;
	case COMMAND_IO:
		break;
	case COMMAND_SET:
		ctx->memory[ctx->command.addr] = ctx->command.value;
		break;
	case COMMAND_RESET:
		sm83_cpu_reset(ctx->cpu);
		break;
	case COMMAND_REGISTERS:
		sm83_cpu_debug(ctx->cpu);
		break;
	case COMMAND_FRAME:
		break;
	case COMMAND_QUIT:
		ctx->state = STATE_QUIT;
		break;
	case COMMAND_HELP:
		print_help();
		break;
	case COMMAND_WATCH:
		if (register_watcher(ctx, ctx->command.addr))
			printf("Too many watchers\n");
		break;
	case COMMAND_WATCH_DELETE:
		if (unregister_watcher(ctx, ctx->command.addr))
			printf("Watcher not found\n");
		else
			printf("Remove watcher %04X\n", ctx->command.addr);
		break;
	case COMMAND_WATCH_LIST:
		for (int i = 0; i < MAX_WATCHERS; i++) {
			if (ctx->watched_addresses[i]) {
				printf("Watch %04X\n",
				       ctx->watched_addresses[i]);
			}
		}
		break;
	}
	return 0;
}

static struct sm83_core *debugger_setup_cpu()
{
	struct sm83_core *cpu;
	cpu = sm83_init();
	if (!cpu)
		return NULL;
	cpu->memory->load8 = &load8;
	cpu->memory->write8 = &write8;
	cpu->memory->read_segment = &read_segment;
	cpu->memory->load16 = &load16;
	return cpu;
}

static void sigint_handler(int dummy)
{
	sigint_catcher = 1;
}

int debugger_init(struct debugger_context *ctx, struct sm83_core *cpu,
		  u8 *memory)
{
	ctx->cpu = cpu;
	ctx->memory = memory;
	for (int i = 0; i < MAX_BREAKPOINTS; i++) {
		ctx->breakpoints[i] = 0;
	}
	for (int i = 0; i < MAX_WATCHERS; i++) {
		ctx->watched_addresses[i] = 0;
		ctx->watched_values[i] = 0;
	}
	return 0;
}

int debugger_step(struct debugger_context *ctx)
{
	while (ctx->state == STATE_WAIT) {
		if (debugger_prompt(ctx))
			return -1;
		debugger_command_handle(ctx);
	}
	ctx->index = ctx->cpu->index;
	sm83_cpu_step(ctx->cpu);
	debugger_command_handle(ctx);
	check_watchers(ctx);
	return 0;
}

int debugger_run(char *path)
{
	u8 *memory;
	struct debugger_context ctx;
	struct sm83_core *cpu;

	memory = (u8 *)calloc(sizeof(u8), MEMORY_SIZE);
	if (!memory)
		return -1;
	if (!(cpu = debugger_setup_cpu()))
		return -1;
	debugger_init(&ctx, cpu, memory);
	signal(SIGINT, sigint_handler);
	cpu->parent = &ctx;
	if (debugger_load_rom(&ctx, path))
		return -1;
	move_to_wait(&ctx);;
	while (ctx.state != STATE_QUIT) {
		if (sigint_catcher) {
			ctx.state = STATE_WAIT;
			sigint_catcher = 0;
		}
		if (debugger_step(&ctx))
			break;
	}
	sm83_destroy(ctx.cpu);
	return 0;
}
