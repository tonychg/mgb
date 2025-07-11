#include "gb/sm83.h"

int main(int argc, char **argv)
{
	if (argc > 1)
		debugger_start(argv[1]);
	else
		sm83_printd("Invalid ROM path");
}
