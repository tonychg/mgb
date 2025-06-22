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
	cpu->debug = cargs->debug;
	if (cargs->debug)
		cartridge_metadata(cartridge);
	cpu_reset(cpu);
	cpu_bind_memory(cpu, memory);
	while (1) {
		if (cargs->cpu_debug)
			cpu_debug(cpu);
		cpu_tick(cpu);
		if (cargs->delay_in_sec)
			cpu_sleep_ns(cargs->delay_in_sec * 1000000000);
		cpu_sleep_ns(CLOCK_PERIOD_NS / cpu->multiplier);
		// Work RAM
		// memory_debug(cpu->memory, 0xC000, 0xCFFF);
		// Video RAM
		// memory_debug(cpu->memory, 0x8000, 0x9FFF);
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
