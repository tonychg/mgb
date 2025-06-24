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
	args->wram_debug = false;
	args->vram_debug = false;
	args->interactive = false;
	args->memory_dump = false;
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
		if ((!strcmp(argv[i], "--interactive") ||
		     !strcmp(argv[i], "-i")))
			args->interactive = true;
		if ((!strcmp(argv[i], "--wram-debug") ||
		     !strcmp(argv[i], "-w")))
			args->wram_debug = true;
		if ((!strcmp(argv[i], "--vram-debug") ||
		     !strcmp(argv[i], "-v")))
			args->vram_debug = true;
		if ((!strcmp(argv[i], "--memory-dump") ||
		     !strcmp(argv[i], "-o")))
			args->memory_dump = true;
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

ArgsTest *parse_args_test(int argc, char **argv)
{
	ArgsTest *args = (ArgsTest *)malloc(sizeof(ArgsTest));

	args->opcode = 0x00;
	args->verbose = true;
	args->is_prefixed = false;
	if (argc <= 2) {
		return args;
	}
	for (int i = 2; i < argc; i++) {
		if ((!strcmp(argv[i], "--opcode") || !strcmp(argv[i], "-c")) &&
		    i + 1 < argc) {
			args->opcode = strtol(argv[i + 1], NULL, 16);
		}
		if ((!strcmp(argv[i], "--is-prefixed") ||
		     !strcmp(argv[i], "-p")))
			args->is_prefixed = true;
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
	if (!strcmp(name, "test")) {
		cmd->args = parse_args_test(argc, argv);
		cmd->callback = gb_test;
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
