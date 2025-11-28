#ifndef _SM83_DEBUGGER_H
#define _SM83_DEBUGGER_H

#include "gb/sm83.h"

#define COMMAND_MAX_LENGTH 256
#define COMMAND_DELIMITERS " \n"
#define MEMORY_SIZE 0xFFFF + 1
#define MAX_BREAKPOINTS 100
#define MAX_WATCHERS 100

enum debugger_command_type {
	COMMAND_NEXT,
	COMMAND_STEP,
	COMMAND_BREAKPOINT,
	COMMAND_BREAKPOINTS,
	COMMAND_BREAKPOINT_DELETE,
	COMMAND_CONTINUE,
	COMMAND_PRINT,
	COMMAND_RANGE,
	COMMAND_LOOP,
	COMMAND_GOTO,
	COMMAND_MEM,
	COMMAND_REGISTERS,
	COMMAND_IO,
	COMMAND_FRAME,
	COMMAND_SET,
	COMMAND_RESET,
	COMMAND_QUIT,
	COMMAND_HELP,
	COMMAND_WATCH,
	COMMAND_WATCH_DELETE,
	COMMAND_WATCH_LIST,
};

enum debugger_state {
	STATE_WAIT,
	STATE_EXECUTE,
	STATE_QUIT,
};

struct cmd_struct {
	const char *description;
	const char *name;
	const char *alias;
	enum debugger_command_type type;
};

struct debugger_command_context {
	enum debugger_command_type type;
	u16 addr;
	u8 value;
	u16 end;
	u32 counter;
};

struct debugger_context {
	struct debugger_command_context command;
	enum debugger_state state;
	u16 index;
	u16 breakpoints[MAX_BREAKPOINTS];
	u16 watched_addresses[MAX_WATCHERS];
	u16 watched_values[MAX_WATCHERS];
	u8 *memory;
	struct sm83_core *cpu;
};

/* sm83_debug.c */
char *sm83_disassemble(struct sm83_core *cpu);
void sm83_cpu_debug(struct sm83_core *cpu);
void sm83_printd(const char *msg);

/* debugger.c */
int debugger_step(struct debugger_context *ctx);
int debugger_run(char *path);
int debugger_init(struct debugger_context *ctx, struct sm83_core *cpu,
		  u8 *memory);

#endif
