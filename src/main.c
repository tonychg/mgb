#include <stdio.h>
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
#include "gb/gb.h"

int main(int argc, char **argv)
{
	struct gb_context ctx;

	if (gb_boot(&ctx, argc, argv))
		printf("Emulator terminate with errors\n");
}
#endif
