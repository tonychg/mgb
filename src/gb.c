#include "gb.h"
#include "cartridge.h"
#include "cli.h"
#include "cpu.h"

int gb_boot(void *args)
{
	ArgsBoot *cargs = (ArgsBoot *)args;
	Cpu *cpu = cpu_init();
	Memory *memory = memory_init();
	Cartridge *cartridge = cartridge_load_from_file(cargs->rom_path);

	memory_bind_cartridge(memory, cartridge);
	cpu->multiplier = cargs->multiplier;
	if (cargs->debug)
		cartridge_metadata(cartridge);
	cpu_reset(cpu);
	cpu_bind_memory(cpu, memory);
	while (1) {
		if (cargs->debug)
			cpu_debug(cpu);
		cpu_tick(cpu);
		if (!cargs->debug)
			cpu_sleep_ns(CLOCK_PERIOD_NS / cpu->multiplier);
		else
			cpu_sleep_ns(50000000L);
	}
	cartridge_release(cartridge);
	cpu_release(cpu);
	return 0;
}

int gb_rom(void *args)
{
	ArgsRom *cargs = (ArgsRom *)args;
	Cartridge *cartridge = cartridge_load_from_file(cargs->rom_path);
	cartridge_metadata(cartridge);
	cartridge_release(cartridge);
	return 0;
}
