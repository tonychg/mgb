#ifdef TEST
#include "test.h"

int main()
{
	test_register();
	test_memory();
	test_cpu();
}
#else
int main(int argc, char **argv)
{
}
#endif
