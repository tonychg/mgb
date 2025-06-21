#include <stdlib.h>
#ifdef TEST
#include "tests.h"

int main()
{
	test_memory();
	test_cpu();
}
#else
#include "cli.h"
#include "alloc.h"

int main(int argc, char **argv)
{
	int exit_code = 0;
	Command *cmd = parse_args(argc, argv);

	if (cmd == NULL) {
		exit_code = -1;
	} else {
		exit_code = cmd->callback(cmd->args);
		zfree(cmd);
	}
	exit(exit_code);
}
#endif
