#include "emu/gb.h"
#include "emu/memory.h"
#include "platform/mm.h"
#include "platform/types.h"
#include "platform/debugger.h"
#include <string.h>
#include <stdlib.h>

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

static int parse_hex(struct debugger *dbg, char **buffer)
{
	char option[COMMAND_MAX_LENGTH] = "";

	if (get_option(buffer, option, COMMAND_DELIMITERS) || !strlen(option)) {
		dbg->command.type = COMMAND_HELP;
		return 0;
	}
	return strtol(option, NULL, 16);
}

static void move_to_wait(struct debugger *dbg)
{
	sm83_info(dbg->gb->cpu);
	dbg->state = STATE_WAIT;
}

static int register_breakpoint(struct debugger *dbg, u16 addr)
{
	for (int i = 0; i < MAX_BREAKPOINTS; i++) {
		if (dbg->breakpoints[i] == 0) {
			printf("New breakpoint $%04X\n", addr);
			dbg->breakpoints[i] = addr;
			return 0;
		}
	}
	return -1;
}

static int unregister_breakpoint(struct debugger *dbg, u16 addr)
{
	for (int i = 0; i < MAX_BREAKPOINTS; i++) {
		if (dbg->breakpoints[i] == addr) {
			dbg->breakpoints[i] = 0;
			return 0;
		}
	}
	return -1;
}

static void check_breakpoints(struct debugger *dbg)
{
	for (int i = 0; i < MAX_BREAKPOINTS; i++) {
		if (dbg->breakpoints[i]) {
			if (dbg->index == dbg->breakpoints[i])
				move_to_wait(dbg);
		}
	}
}

static int register_watcher(struct debugger *dbg, u16 addr)
{
	for (int i = 0; i < MAX_WATCHERS; i++) {
		if (!dbg->watched_addresses[i]) {
			dbg->watched_addresses[i] = addr;
			dbg->watched_values[i] = load_u8(dbg->memory, addr);
			printf("New watcher $%04X\n", addr);
			return 0;
		}
	}
	return -1;
}

static int unregister_watcher(struct debugger *dbg, u16 addr)
{
	for (int i = 0; i < MAX_WATCHERS; i++) {
		if (dbg->watched_addresses[i] == addr) {
			dbg->watched_addresses[i] = 0;
			dbg->watched_values[i] = 0;
			return 0;
		}
	}
	return -1;
}

static void check_watchers(struct debugger *dbg)
{
	const char *format =
		"$%1$04X $%2$02X (0b%2$08b) -> $%03$2X (0b%3$08b)\n";
	for (int i = 0; i < MAX_WATCHERS; i++) {
		if (dbg->watched_addresses[i]) {
			u16 addr = dbg->watched_addresses[i];
			u8 prev = dbg->watched_values[i];
			u8 curr = load_u8(dbg->memory, addr);
			if (curr == prev)
				continue;
			printf("Hit watchpoint at %04X\n", addr);
			printf(format, dbg->cpu->index, prev, curr);
			dbg->watched_values[i] = curr;
			move_to_wait(dbg);
			break;
		}
	}
}

static int command_parse(struct debugger *dbg, char *buffer)
{
	char option[COMMAND_MAX_LENGTH] = "";
	get_option(&buffer, option, COMMAND_DELIMITERS);
	dbg->command.type = COMMAND_NEXT;
	for (int i = 0; i < ARRAY_SIZE(commands); i++) {
		struct cmd_struct s = commands[i];
		if (command_match(s, option)) {
			dbg->command.type = i;
			break;
		}
	}
	switch (dbg->command.type) {
	case COMMAND_NEXT:
	case COMMAND_STEP:
	case COMMAND_MEM:
	case COMMAND_IO:
	case COMMAND_RESET:
	case COMMAND_INFO:
	case COMMAND_QUIT:
	case COMMAND_HELP:
	case COMMAND_FRAME:
	case COMMAND_CONTINUE:
	case COMMAND_LIST:
	case COMMAND_SAVE:
	case COMMAND_CLEAR:
		break;
	case COMMAND_BREAKPOINT:
	case COMMAND_DELETE:
	case COMMAND_PRINT:
	case COMMAND_WATCH:
	case COMMAND_SET:
		dbg->command.addr = parse_hex(dbg, &buffer);
		dbg->command.value = parse_hex(dbg, &buffer);
		break;
	case COMMAND_RANGE:
		dbg->command.addr = parse_hex(dbg, &buffer);
		dbg->command.end = parse_hex(dbg, &buffer);
		break;
	}
	return 0;
}

static int debugger_prompt(struct debugger *dbg)
{
	char buffer[COMMAND_MAX_LENGTH];
	printf("> ");
	if (!fgets(buffer, COMMAND_MAX_LENGTH, stdin)) {
		return -1;
	}
	if (command_parse(dbg, buffer)) {
		return -1;
	}
	return 0;
}

static int debugger_command_handle(struct debugger *dbg)
{
	switch (dbg->command.type) {
	case COMMAND_NEXT:
		if (dbg->state == STATE_EXECUTE &&
		    dbg->cpu->index != dbg->index) {
			move_to_wait(dbg);
		} else {
			dbg->state = STATE_EXECUTE;
		}
		break;
	case COMMAND_STEP:
		if (dbg->state == STATE_EXECUTE)
			move_to_wait(dbg);
		else
			dbg->state = STATE_EXECUTE;
		break;
	case COMMAND_BREAKPOINT:
		if (register_breakpoint(dbg, dbg->command.addr))
			printf("Failed to register new breakpoints, remove some");
		break;
	case COMMAND_DELETE:
		if (!unregister_breakpoint(dbg, dbg->command.addr))
			printf("Remove breakpoint $%04X\n", dbg->command.addr);
		if (!unregister_watcher(dbg, dbg->command.addr))
			printf("Remove watcher %04X\n", dbg->command.addr);
		break;
	case COMMAND_CONTINUE:
		if (dbg->state == STATE_EXECUTE) {
			check_breakpoints(dbg);
			check_watchers(dbg);
		} else {
			dbg->state = STATE_EXECUTE;
		}
		break;
	case COMMAND_PRINT:
		print_addr(dbg->memory, dbg->command.addr);
		break;
	case COMMAND_RANGE:
		break;
	case COMMAND_MEM:
		dump_memory(dbg->memory);
		break;
	case COMMAND_IO:
		print_hardware_registers(dbg->memory);
		break;
	case COMMAND_SET:
		write_u8(dbg->memory, dbg->command.addr, dbg->command.value);
		break;
	case COMMAND_RESET:
		sm83_cpu_reset(dbg->cpu);
		break;
	case COMMAND_INFO:
		sm83_info(dbg->cpu);
		ppu_info(dbg->gpu);
		break;
	case COMMAND_FRAME: {
		if (dbg->state == STATE_EXECUTE) {
			dbg->until -= dbg->cpu->multiplier;
			if (dbg->until == 0) {
				dbg->state = STATE_WAIT;
			}
		} else {
			dbg->state = STATE_EXECUTE;
			dbg->until = GB_VIDEO_FRAME_PERIOD;
		}
		break;
	}
	case COMMAND_QUIT:
		dbg->state = STATE_QUIT;
		break;
	case COMMAND_HELP:
		print_help();
		break;
	case COMMAND_WATCH:
		if (register_watcher(dbg, dbg->command.addr))
			printf("Too many watchers\n");
		break;
	case COMMAND_LIST:
		for (int i = 0; i < MAX_BREAKPOINTS; i++) {
			if (dbg->breakpoints[i])
				printf("Breakpoint $%04X\n",
				       dbg->breakpoints[i]);
		}
		for (int i = 0; i < MAX_WATCHERS; i++) {
			if (dbg->watched_addresses[i]) {
				printf("Watch %04X\n",
				       dbg->watched_addresses[i]);
			}
		}
		break;
	case COMMAND_SAVE:
		printf("Dumping memory to dump.sav\n");
		dump_memory_to_file(dbg->memory, "dump.sav");
		break;
	case COMMAND_CLEAR:
		for (int i = 0; i < MAX_BREAKPOINTS; i++) {
			dbg->breakpoints[i] = 0;
		}
		for (int i = 0; i < MAX_WATCHERS; i++) {
			dbg->watched_addresses[i] = 0;
			dbg->watched_values[i] = 0;
		}
		break;
	}
	return 0;
}

int debugger_new(struct debugger *dbg)
{
	dbg->state = STATE_WAIT;
	for (int i = 0; i < MAX_BREAKPOINTS; i++)
		dbg->breakpoints[i] = 0;
	for (int i = 0; i < MAX_WATCHERS; i++) {
		dbg->watched_addresses[i] = 0;
		dbg->watched_values[i] = 0;
	}
	return 0;
}

int debugger_step(struct debugger *dbg)
{
	struct cmd_struct cmd = commands[dbg->command.type];
	while (dbg->state == STATE_WAIT) {
		if (debugger_prompt(dbg))
			return -1;
		debugger_command_handle(dbg);
		printf("Command: %s State: %d\n", cmd.name, dbg->state);
	}
	debugger_command_handle(dbg);
	dbg->index = dbg->cpu->index;
	if (dbg->state == STATE_EXECUTE)
		sm83_cpu_step(dbg->cpu);
	return 0;
}
