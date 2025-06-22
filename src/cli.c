#include "cli.h"
#include "gb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ArgsBoot *parse_args_boot(int argc, char **argv)
{
	ArgsBoot *args = (ArgsBoot *)malloc(sizeof(ArgsBoot));

	if (args == NULL)
		return NULL;
	args->rom_path = NULL;
	args->debug = false;
	args->cpu_debug = false;
	args->delay_in_sec = 0.0;
	args->multiplier = 1;
	if (argc <= 2) {
		return args;
	}

	for (int i = 2; i < argc; i++) {
		if ((!strcmp(argv[i], "--path") || !strcmp(argv[i], "-p")) &&
		    i + 1 < argc) {
			args->rom_path = argv[i + 1];
		}
		if ((!strcmp(argv[i], "--debug") || !strcmp(argv[i], "-d")))
			args->debug = true;
		if ((!strcmp(argv[i], "--multiplier") ||
		     !strcmp(argv[i], "-m")) &&
		    i + 1 < argc) {
			args->multiplier = atoi(argv[i + 1]);
		}
		if ((!strcmp(argv[i], "--cpu-debug") || !strcmp(argv[i], "-c")))
			args->cpu_debug = true;
		if ((!strcmp(argv[i], "--delay")) && i + 1 < argc)
			args->delay_in_sec = atof(argv[i + 1]);
	}
	return args;
}

ArgsRom *parse_args_rom(int argc, char **argv)
{
	ArgsRom *args = (ArgsRom *)malloc(sizeof(ArgsRom));

	if (args == NULL)
		return NULL;
	args->rom_path = NULL;
	if (argc <= 2) {
		return args;
	}
	for (int i = 2; i < argc; i++) {
		if ((!strcmp(argv[i], "--path") || !strcmp(argv[i], "-p")) &&
		    i + 1 < argc) {
			args->rom_path = argv[i + 1];
		}
	}
	return args;
}

Command *parse_command(int argc, char **argv)
{
	Command *cmd;
	char *name = argv[1];

	if ((cmd = (Command *)malloc(sizeof(Command))) == NULL)
		return NULL;
	if (!strcmp(name, "boot")) {
		cmd->args = parse_args_boot(argc, argv);
		cmd->callback = gb_boot;
	}
	if (!strcmp(name, "rom")) {
		cmd->args = parse_args_rom(argc, argv);
		cmd->callback = gb_rom;
	}
	return cmd;
}

Command *parse_args(int argc, char **argv)
{
	if (argc < 2) {
		printf(CLI_HELP);
		return NULL;
	} else {
		return parse_command(argc, argv);
	}
}
