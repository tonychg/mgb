#include "gb/alloc.h"
#include "gb/sm83.h"
#include "gb/fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void debugger_event_loop(struct sm83_debugger *debugger)
{
	bool debugger_running = true;
	struct debugger_command *cmd;
	u16 index;

	while (debugger_running) {
		cmd = debugger_readline();
		if (!cmd) {
			sm83_printd("Failed to parse command");
			continue;
		}
		index = debugger->cpu->index;
		switch (cmd->type) {
		case DEBUG_STEP:
			sm83_cpu_step(debugger->cpu);
			sm83_cpu_debug(debugger->cpu);
			break;
		case DEBUG_NEXT:
			do {
				sm83_cpu_step(debugger->cpu);
			} while (index == debugger->cpu->index);
			sm83_cpu_debug(debugger->cpu);
			break;
		case DEBUG_LOOP:
			for (int i = 0; i < cmd->counter; i++) {
				u16 idx = debugger->cpu->index;
				do {
					sm83_cpu_step(debugger->cpu);
				} while (debugger->cpu->index == idx);
			}
			sm83_cpu_debug(debugger->cpu);
			break;
		case DEBUG_CONTINUE:
			do {
				sm83_cpu_step(debugger->cpu);
			} while (debugger->cpu->index != debugger->breakpoint);
			sm83_cpu_debug(debugger->cpu);
			break;
		case DEBUG_BREAKPOINT:
			debugger->breakpoint = cmd->addr;
			printf("Breakpoint $%04X\n", cmd->addr);
			break;
		case DEBUG_PRINT:
			printf("$%04X -> %02X\n", cmd->addr,
			       debugger->cpu->memory->load8(debugger->cpu,
							    cmd->addr));
			break;
		case DEBUG_RESET:
			sm83_cpu_reset(debugger->cpu);
			sm83_cpu_debug(debugger->cpu);
			break;
		case DEBUG_QUIT:
			debugger_running = false;
			break;
		}
		zfree(cmd);
	}
}

static void debugger_start(char *rom_path)
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
	debugger_event_loop(debugger);
	sm83_debugger_destroy(debugger);

free_rom:
	zfree(rom);
exit:
	return;
}

int main(int argc, char **argv)
{
	if (argc > 1) {
		debugger_start(argv[1]);
	} else {
		sm83_printd("Invalid ROM path");
	}
}
