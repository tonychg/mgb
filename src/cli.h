#ifndef __CLI_H__
#define __CLI_H__

#include <stdbool.h>

typedef struct Command {
	int (*callback)(void *args);
	void *args;
} Command;

typedef struct ArgsBoot {
	char *rom_path;
} ArgsBoot;

typedef struct ArgsRom {
	char *rom_path;
} ArgsRom;

#define CLI_HELP "Usage: gameboy [rom,boot] [-p/--path]\n"

Command *parse_args(int argc, char **argv);

#endif
