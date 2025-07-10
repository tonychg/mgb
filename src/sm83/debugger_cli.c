#include "gb/sm83.h"
#include "gb/fs.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc > 1) {
		u8 *rom;

		rom = readfile(argv[1]);
		if (!rom)
			printf("Failed to open %s\n", argv[1]);
		else
			sm83_debugger_start(rom);
	} else {
		printf("Invalid ROM path\n");
	}
}
