#include "gb/gb.h"
#include "gb/cartridge.h"
#include "gb/fs.h"
#include "gb/render.h"
#include "gb/tests.h"
#include "gb/video.h"
#include "gb/thread.h"
#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Gb *gb_create(ArgsBoot *args)
{
	Gb *gb;

	if ((gb = (Gb *)malloc(sizeof(Gb))) == NULL)
		return NULL;
	gb->cpu = NULL;
	gb->memory = NULL;
	gb->cartridge = NULL;
	gb->video = NULL;
	gb->args = args;
	return gb;
}

void gb_init(Gb *gb)
{
	gb->cpu = cpu_init();
	gb->memory = memory_init();
	gb->cartridge = cartridge_load_from_file(gb->args->rom_path);
	gb->video = video_init(gb->args->render);
	gb->cpu->multiplier = gb->args->multiplier;
	gb->cpu->debug = gb->args->debug;
	video_bind_memory(gb->video, gb->memory);
	cpu_bind_memory(gb->cpu, gb->memory);
	gb_reset(gb);
}

void gb_reset(Gb *gb)
{
	memory_reset(gb->cpu->memory);
	memory_bind_cartridge(gb->cpu->memory, gb->cartridge);
	video_reset(gb->video);
	cpu_reset(gb->cpu);
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

void gb_tick(Gb *gb)
{
	cpu_tick(gb->cpu);
	video_tick(gb->video);
}

void gb_goto(Gb *gb, u16 address)
{
	while (gb->cpu->pc != address) {
		gb_tick(gb);
	}
}

void gb_start_at(Gb *gb)
{
	printf("Starting at $%04X\n", gb->args->start);
	gb_goto(gb, gb->args->start);
	gb->args->start = 0;
}

void gb_release(Gb *gb)
{
	memory_release(gb->memory);
	cpu_release(gb->cpu);
	video_release(gb->video);
	cartridge_release(gb->cartridge);
}

char *gb_interactive(Gb *gb)
{
	char buf[256];

	printf("> ");
	if (fgets(buf, 256, stdin) == 0)
		return NULL;
	switch (buf[0]) {
	case 'w':
		memory_debug(gb->memory, 0xC000, 0xDFFF);
		gb_interactive(gb);
		break;
	case 'v':
		memory_debug(gb->memory, 0x8000, 0x9FFF);
		gb_interactive(gb);
		break;
	case 'q':
		gb_release(gb);
		exit(0);
		break;
	case 'o':
		memory_dump(gb->memory);
		gb_interactive(gb);
		break;
	case 'r':
		gb_reset(gb);
		break;
	case 'R':
		print_byte_at(gb->cpu, buf);
		gb_interactive(gb);
		break;
	case 'd':
		cpu_debug(gb->cpu);
		gb_interactive(gb);
		break;
	case 'g':
		gb_goto(gb, parse_hex_address(buf));
		break;
	case 'V':
		video_debug(gb->video);
		gb_interactive(gb);
		break;
	default:
		do {
			gb_tick(gb);
		} while (gb->cpu->cycles > 0);
		break;
	}
	return NULL;
}

void gb_debug(Gb *gb)
{
	if (gb->args->cpu_debug)
		cpu_debug(gb->cpu);
	if (gb->args->delay_in_sec)
		usleep(gb->args->delay_in_sec * 1000000);
	if (gb->args->wram_debug) {
		// Work RAM
		memory_debug(gb->memory, 0xC000, 0xCFFF);
		// Work RAM Bank
		memory_debug(gb->memory, 0xD000, 0xDFFF);
	}
	if (gb->args->vram_debug) {
		// Video RAM
		memory_debug(gb->memory, 0x8000, 0x9FFF);
	}
	if (gb->args->memory_dump) {
		memory_dump(gb->memory);
	}
}

int gb_boot(void *args)
{
	thread_boot((ArgsBoot *)args);
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

int gb_render(void *args)
{
	u8 *buffer;
	size_t size_in_bytes;
	ArgsRender *cargs = (ArgsRender *)args;
	FILE *dump = fopen(cargs->dump, "r");

	if (dump == NULL)
		return -1;
	size_in_bytes = fs_size(dump);
	buffer = fs_read(dump, size_in_bytes);
	if (buffer != NULL) {
		Video *video = video_init(true);
		Memory *memory = memory_init();
		int i = 0;
		memcpy(memory->bus, buffer, size_in_bytes);
		video_bind_memory(video, memory);
		while (render_is_running()) {
			printf("Rendering %d ...\n", i);
			video_render(video);
			i++;
		}
		video_release(video);
		memory_release(memory);
	}
	return 0;
}
