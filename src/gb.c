#include "gb.h"
#include "cartridge.h"
#include "cli.h"
#include "cpu.h"
#include "tests.h"
#include "unistd.h"
#include "video.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void gb_reset(Cpu *cpu, Cartridge *cartridge, Video *video)
{
	memory_reset(cpu->memory);
	memory_bind_cartridge(cpu->memory, cartridge);
	cpu_reset(cpu);
	video_reset(video);
}

u16 parse_hex_address(char *buf)
{
	char addr[4];

	for (int i = 0; i < 4; i++) {
		addr[i] = buf[i + 2];
	}
	return strtol(addr, NULL, 16);
}

void print_byte_at(Cpu *cpu, char *buf)
{
	u16 addr = parse_hex_address(buf);
	u8 byte = MEM_READ(cpu, addr);

	printf("Reading [$%04X] = $%02X\n", addr, byte);
}

void gb_tick(Cpu *cpu, Video *video)
{
	cpu_tick(cpu);
	video_tick(video);
}

void gb_goto(Cpu *cpu, Video *video, u16 address)
{
	while (cpu->pc != address) {
		gb_tick(cpu, video);
	}
}

char *gb_interactive(Cpu *cpu, Cartridge *cartridge, Video *video)
{
	char buf[256];

	printf("> ");
	if (fgets(buf, 256, stdin) == 0)
		return NULL;
	switch (buf[0]) {
	case 'w':
		memory_debug(cpu->memory, 0xC000, 0xDFFF);
		gb_interactive(cpu, cartridge, video);
		break;
	case 'v':
		memory_debug(cpu->memory, 0x8000, 0x9FFF);
		gb_interactive(cpu, cartridge, video);
		break;
	case 'q':
		memory_release(cpu->memory);
		cpu_release(cpu);
		cartridge_release(cartridge);
		exit(0);
		break;
	case 'o':
		memory_dump(cpu->memory);
		gb_interactive(cpu, cartridge, video);
		break;
	case 'r':
		gb_reset(cpu, cartridge, video);
		break;
	case 'R':
		print_byte_at(cpu, buf);
		gb_interactive(cpu, cartridge, video);
		break;
	case 'd':
		cpu_debug(cpu);
		gb_interactive(cpu, cartridge, video);
		break;
	case 'g':
		gb_goto(cpu, video, parse_hex_address(buf));
		break;
	case 'V':
		video_debug(video);
		gb_interactive(cpu, cartridge, video);
		break;
	default:
		do {
			gb_tick(cpu, video);
		} while (cpu->cycles > 0);
		break;
	}
	return NULL;
}

int gb_boot(void *args)
{
	ArgsBoot *cargs = (ArgsBoot *)args;
	Cpu *cpu = cpu_init();
	Memory *memory = memory_init();
	Cartridge *cartridge = cartridge_load_from_file(cargs->rom_path);
	Video *video = video_init();

	cpu->multiplier = cargs->multiplier;
	cpu->debug = cargs->debug;
	if (cargs->debug)
		cartridge_metadata(cartridge);
	video_bind_memory(video, memory);
	cpu_bind_memory(cpu, memory);
	gb_reset(cpu, cartridge, video);
	if (cargs->start) {
		printf("Starting at $%04X\n", cargs->start);
		gb_goto(cpu, video, cargs->start);
		cargs->start = 0;
	}
	while (1) {
		if (cargs->interactive) {
			gb_interactive(cpu, cartridge, video);
		}
		if (cargs->cpu_debug)
			cpu_debug(cpu);
		if (cargs->delay_in_sec)
			usleep(cargs->delay_in_sec * 1000000);
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
		if (cargs->memory_dump) {
			memory_dump(cpu->memory);
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

int gb_test(void *args)
{
	ArgsTest *cargs = (ArgsTest *)args;

	test_opcode(cargs->opcode, cargs->verbose, cargs->is_prefixed);
	return 0;
}
