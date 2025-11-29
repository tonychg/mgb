#include "debugger.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Invalid ROM path\n");
	}
	if (debugger_run(argv[1])) {
		printf("Debugger terminate with errors\n");
	}
}
