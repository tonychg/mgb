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
	COMMAND_SAVE,
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

// clang-format off
static const struct cmd_struct commands[] = {
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
	{ "save (sv)                   Save the current state\n", "save", "sv", COMMAND_SAVE },
};
// clang-format on

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
	struct gb_context *gb;

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
