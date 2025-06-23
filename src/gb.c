#include "gb.h"
#include "cartridge.h"
#include "cli.h"
#include "cpu.h"
#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void gb_reset(Cpu *cpu, Cartridge *cartridge)
{
	memory_reset(cpu->memory);
	memory_bind_cartridge(cpu->memory, cartridge);
	cpu_reset(cpu);
}

char *gb_interactive(Cpu *cpu, Cartridge *cartridge)
{
	static char buf[256];

	printf("> ");
	if (scanf("%255s", buf) >= 0) {
		if (!strcmp(buf, "c")) {
			return NULL;
		} else if (!strcmp(buf, "p")) {
			memory_debug(cpu->memory, 0xC000, 0xDFFF);
			gb_interactive(cpu, cartridge);
		} else if (!strcmp(buf, "d")) {
			cpu_debug(cpu);
			gb_interactive(cpu, cartridge);
		} else if (!strcmp(buf, "q")) {
			exit(0);
		} else if (!strcmp(buf, "r")) {
			gb_reset(cpu, cartridge);
		}
	}
	return NULL;
}

int gb_boot(void *args)
{
	ArgsBoot *cargs = (ArgsBoot *)args;
	Cpu *cpu = cpu_init();
	Memory *memory = memory_init();
	Cartridge *cartridge = cartridge_load_from_file(cargs->rom_path);

	cpu->multiplier = cargs->multiplier;
	cpu->debug = cargs->debug;
	if (cargs->debug)
		cartridge_metadata(cartridge);
	cpu_bind_memory(cpu, memory);
	gb_reset(cpu, cartridge);
	cpu_enable_display(cpu);
	while (1) {
		if (cargs->interactive) {
			gb_interactive(cpu, cartridge);
		}
		if (cargs->cpu_debug)
			cpu_debug(cpu);
		cpu_tick(cpu);
		if (cargs->delay_in_sec)
			usleep(cargs->delay_in_sec * 1000000);
		cpu_sleep_ns(CLOCK_PERIOD_NS / cpu->multiplier);
		if (cargs->wram_debug) {
			// Work RAM
			memory_debug(cpu->memory, 0xC000, 0xCFFF);
			// Work RAM Bank
			memory_debug(cpu->memory, 0xD000, 0xDFFF);
		}
		if (cargs->vram_debug) {
			// Video RAM
			memory_debug(cpu->memory, 0x8000, 0x9FFF);
		}
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
