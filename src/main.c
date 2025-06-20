#ifdef TEST
#include "tests.h"

int main()
{
	test_register();
	test_memory();
	test_cpu();
}
#else
#include "cartridge.h"

int main(int argc, char **argv)
{
	cartridge *gb_cartridge = cartridge_load_from_file(argv[1]);
	cartridge_metadata(gb_cartridge);
	cartridge_release(gb_cartridge);
}
#endif
