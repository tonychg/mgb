#ifndef _SM83_DEBUGGER_H
#define _SM83_DEBUGGER_H

#include "mgb/mgb.h"

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
	COMMAND_MEM,
	COMMAND_INFO,
	COMMAND_IO,
	COMMAND_FRAME,
	COMMAND_SET,
	COMMAND_RESET,
	COMMAND_QUIT,
	COMMAND_HELP,
	COMMAND_WATCH,
	COMMAND_LIST,
	COMMAND_SAVE,
	COMMAND_CLEAR,
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
	[COMMAND_NEXT]       = { "next (n)                Next instruction\n", "next", "n" },
	[COMMAND_STEP]       = { "step (s)                Step one M-cycle\n", "step", "s" },
	[COMMAND_BREAKPOINT] = { "break (b) <addr>        Set a breakpoint\n", "break", "b" },
	[COMMAND_DELETE]     = { "del (d)                 Delete breakpoint or wacher\n", "del", "d" },
	[COMMAND_CONTINUE]   = { "continue (c)            Continue until next breakpoint\n", "continue", "c" },
	[COMMAND_PRINT]      = { "print (p) <addr>        Print address value\n", "print", "p" },
	[COMMAND_RANGE]      = { "range (r) <addr> <addr> Dump memory range\n", "range", "r" },
	[COMMAND_MEM]        = { "mem (m)                 Dump memory\n", "mem", "m" },
	[COMMAND_INFO]       = { "info (in)               Print information\n", "info", "i" },
	[COMMAND_IO]         = { "io (io)                 Dump I/O ranges\n", "io", "io" },
	[COMMAND_FRAME]      = { "frame (f)               Next frame\n", "frame", "f" },
	[COMMAND_SET]        = { "set (s) <addr> <value>  Set value\n", "set", "s" },
	[COMMAND_RESET]      = { "reset (r)               Reset\n", "reset", "r" },
	[COMMAND_QUIT]       = { "quit (q)                Quit\n", "quit", "q" },
	[COMMAND_HELP]       = { "help (h)                Display this message\n", "help", "h" },
	[COMMAND_WATCH]      = { "watch (w) <addr>        Watch address\n", "watch", "w" },
	[COMMAND_LIST]       = { "list (ll)               List breakpoints and watchers\n", "list", "ll" },
	[COMMAND_SAVE]       = { "save (sv)               Save the current state\n", "save", "sv" },
	[COMMAND_CLEAR]      = { "clear (cl)              Clear all watch and break points\n", "clear", "cl" },
};
// clang-format on

struct debugger_command_context {
	u16 addr;
	u8 value;
	u16 end;
	u32 counter;
	enum debugger_command_type type;
};

struct debugger {
	u16 index;
	u32 until;

	struct gb_emulator *gb;

	u16 breakpoints[MAX_BREAKPOINTS];
	u16 watched_addresses[MAX_WATCHERS];
	u16 watched_values[MAX_WATCHERS];

	u8 break_counter;
	u8 watch_counter;

	enum debugger_state state;
	struct debugger_command_context command;
};

/* debugger.c */
int debugger_step(struct debugger *dbg);
int debugger_new(struct debugger *dbg);

#endif
