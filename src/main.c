#include <stdlib.h>
#ifdef TEST
#include "gb/tests.h"

int main()
{
	test_memory();
#ifdef VERBOSE
	test_cpu(true);
#else
	test_cpu(false);
#endif
}
#else
#include "gb/cli.h"
#include "gb/alloc.h"

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
