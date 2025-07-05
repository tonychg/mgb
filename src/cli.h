#ifndef __CLI_H__
#define __CLI_H__

#include <stdbool.h>

typedef struct Command {
	int (*callback)(void *args);
	void *args;
} Command;

typedef struct ArgsBoot {
	char *rom_path;
	bool debug;
	bool render;
	bool cpu_debug;
	bool wram_debug;
	bool vram_debug;
	bool memory_dump;
	bool interactive;
	double delay_in_sec;
	int multiplier;
	int start;
} ArgsBoot;

typedef struct ArgsRom {
	char *rom_path;
} ArgsRom;

typedef struct ArgsTest {
	int opcode;
	bool verbose;
	bool is_prefixed;
} ArgsTest;

typedef struct ArgsRender {
	int scale;
	char *dump;
	char *type;
} ArgsRender;

#define CLI_HELP                                                  \
	"Usage: gameboy [rom,boot,test] [-p/--path] [-d/--debug]" \
	"[-m/--multiplier] [INT]"                                 \
	"\n"

Command *parse_args(int argc, char **argv);

#endif
