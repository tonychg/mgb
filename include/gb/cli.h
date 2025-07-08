#ifndef __CLI_H__
#define __CLI_H__

#include <stdbool.h>

struct command {
	int (*callback)(void *args);
	void *args;
};

struct args_boot {
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
};

struct args_rom {
	char *rom_path;
};

struct args_test {
	int opcode;
	bool verbose;
	bool is_prefixed;
};

struct args_render {
	int scale;
	char *dump;
};

#define CLI_HELP                                                  \
	"Usage: gameboy [rom,boot,test] [-p/--path] [-d/--debug]" \
	"[-m/--multiplier] [INT]"                                 \
	"\n"

struct command *parse_args(int argc, char **argv);

#endif
