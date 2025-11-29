#include "emu/memory.h"
#include "platform/mm.h"
#include "platform/types.h"
#include "debugger.h"
#include <signal.h>
#include <string.h>
#include <stdlib.h>

static volatile int sigint_catcher = 0;

// clang-format off
static struct cmd_struct commands[] = {
	{ "next (n)                    Next instruction\n", "next", "n", COMMAND_NEXT },
	{ "step (s)                    Step one M-cycle\n", "step", "s", COMMAND_STEP },
	{ "break (b) <addr>            Set a breakpoint\n", "break", "b", COMMAND_BREAKPOINT },
	{ "del (d)                     Delete breakpoint or wacher\n", "del", "d", COMMAND_DELETE },
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
	{ "list (ll)                   List breakpoints and watchers\n", "list", "ll", COMMAND_LIST },
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

static int is_delimiter(char c, char *delims)
{
	for (int i = 0; i < strlen(delims); i++)
		if (c == delims[i])
			return 1;
	return 0;
}

static int get_option(char **buffer, char *option, char *delims)
{
	int start, end, length;
	char *curr = *buffer;

	for (start = 0;
	     is_delimiter(curr[start], delims) && start < strlen(curr); start++)
		;
	for (end = start;
	     !is_delimiter(curr[end], delims) && end < strlen(curr); end++)
		;
	length = end - start + 1;
	option[length] = '\0';
	strncpy(option, curr + start, end - start);
	(*buffer) += end;
	return 0;
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
			case COMMAND_LIST:
				break;
			case COMMAND_BREAKPOINT:
			case COMMAND_DELETE:
			case COMMAND_PRINT:
			case COMMAND_WATCH:
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
			break;
		}
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
			ctx->watched_values[i] = load_u8(ctx->memory, addr);
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
	const char *format =
		"$%1$04X $%2$02X (0b%2$08b) -> $%03$2X (0b%3$08b)\n";
	for (int i = 0; i < MAX_WATCHERS; i++) {
		if (ctx->watched_addresses[i]) {
			u16 addr = ctx->watched_addresses[i];
			u8 prev = ctx->watched_values[i];
			u8 curr = load_u8(ctx->memory, addr);
			if (curr == prev)
				continue;
			printf("Hit watchpoint at %04X\n", addr);
			printf(format, ctx->cpu->index, prev, curr);
			ctx->watched_values[i] = curr;
			move_to_wait(ctx);
			break;
		}
	}
}

static int debugger_command_handle(struct debugger_context *ctx)
{
	switch (ctx->command.type) {
	case COMMAND_NEXT:
		if (ctx->state == STATE_EXECUTE &&
		    ctx->cpu->index != ctx->index) {
			move_to_wait(ctx);
		} else {
			ctx->state = STATE_EXECUTE;
		}
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
	case COMMAND_DELETE:
		if (!unregister_breakpoint(ctx, ctx->command.addr))
			printf("Remove breakpoint $%04X", ctx->command.addr);
		if (unregister_watcher(ctx, ctx->command.addr))
			printf("Remove watcher %04X\n", ctx->command.addr);
		break;
	case COMMAND_CONTINUE:
		if (ctx->state == STATE_EXECUTE) {
			check_breakpoints(ctx);
			check_watchers(ctx);
		} else {
			ctx->state = STATE_EXECUTE;
		}
		break;
	case COMMAND_PRINT:
		print_addr(ctx->memory, ctx->command.addr);
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
		print_hardware_registers(ctx->memory);
		break;
	case COMMAND_SET:
		write_u8(ctx->memory, ctx->command.addr, ctx->command.value);
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
	case COMMAND_LIST:
		for (int i = 0; i < MAX_BREAKPOINTS; i++) {
			if (ctx->breakpoints[i])
				printf("Breakpoint $%04X\n",
				       ctx->breakpoints[i]);
		}
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

static u8 load8(struct sm83_core *cpu, u16 addr)
{
	struct debugger_context *ctx = (struct debugger_context *)cpu->parent;
	return load_u8(ctx->memory, addr);
}

static void write8(struct sm83_core *cpu, u16 addr, u8 value)
{
	struct debugger_context *ctx = (struct debugger_context *)cpu->parent;
	write_u8(ctx->memory, addr, value);
}

static struct sm83_core *debugger_setup_cpu()
{
	struct sm83_core *cpu;
	cpu = sm83_init();
	if (!cpu)
		return NULL;
	cpu->memory->load8 = &load8;
	cpu->memory->write8 = &write8;
	return cpu;
}

static void sigint_handler(int dummy)
{
	sigint_catcher = 1;
}

int debugger_new(struct debugger_context *ctx)
{
	ctx->state = STATE_WAIT;
	for (int i = 0; i < MAX_BREAKPOINTS; i++)
		ctx->breakpoints[i] = 0;
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
		printf("Command: %d State: %d\n", ctx->command.type, ctx->state);
	}
	debugger_command_handle(ctx);
	ctx->index = ctx->cpu->index;
	if (ctx->state == STATE_EXECUTE)
		sm83_cpu_step(ctx->cpu);
	return 0;
}

int debugger_run(char *path)
{
	struct debugger_context ctx;
	debugger_new(&ctx);
	signal(SIGINT, sigint_handler);
	if (!(ctx.cpu = debugger_setup_cpu())) {
		printf("failed to allocate new cpu\n");
		return -1;
	}
	if (!(ctx.memory = allocate_memory())) {
		printf("failed to allocate new memory\n");
		goto err;
	}
	ctx.cpu->parent = &ctx;
	if (load_rom(ctx.memory, path))
		return -1;
	move_to_wait(&ctx);
	while (ctx.state != STATE_QUIT) {
		if (sigint_catcher) {
			ctx.state = STATE_WAIT;
			sigint_catcher = 0;
		}
		if (debugger_step(&ctx))
			break;
	}
	destroy_memory(ctx.memory);
	sm83_destroy(ctx.cpu);
	return 0;
err:
	sm83_destroy(ctx.cpu);
	return -1;
}
