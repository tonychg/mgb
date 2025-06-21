#ifdef TEST
#include "tests.h"

int main()
{
	test_register();
	test_memory();
	test_cpu();
}
#else
#include "cpu.h"
#include "cartridge.h"
#include <unistd.h>

void sleep_ms(int milliseconds)
{
	usleep(milliseconds * 1000);
}

int main(int argc, char **argv)
{
	Cpu *cpu = cpu_init();
	Cartridge *gb_cartridge = cartridge_load_from_file(argv[1]);
	cartridge_metadata(gb_cartridge);
	cpu_reset(cpu);
	while (1) {
		cpu_debug(cpu);
		cpu_instruction(cpu, gb_cartridge);
		sleep_ms(100);
	}
	cartridge_release(gb_cartridge);
	cpu_release(cpu);
}
#endif
