#ifndef _SM83_DEBUGGER_H
#define _SM83_DEBUGGER_H

#include "emu/sm83.h"
#include "emu/memory.h"

#define COMMAND_MAX_LENGTH 256
#define COMMAND_DELIMITERS " \n"
#define MAX_BREAKPOINTS 100
#define MAX_WATCHERS 100

enum debugger_command_type {
	COMMAND_NEXT,
	COMMAND_STEP,
	COMMAND_BREAKPOINT,
	COMMAND_DELETE,
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
	COMMAND_LIST,
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
	u16 index;

	struct sm83_core *cpu;
	struct shared *memory;

	u16 breakpoints[MAX_BREAKPOINTS];
	u16 watched_addresses[MAX_WATCHERS];
	u16 watched_values[MAX_WATCHERS];

	enum debugger_state state;
	struct debugger_command_context command;
};

/* sm83_debug.c */
char *sm83_disassemble(struct sm83_core *cpu);
void sm83_cpu_debug(struct sm83_core *cpu);
void sm83_printd(const char *msg);

/* debugger.c */
int debugger_step(struct debugger_context *ctx);
int debugger_run(char *path);
int debugger_new(struct debugger_context *ctx);

#endif
