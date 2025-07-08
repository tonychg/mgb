#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "gb/gb.h"
#include "gb/cartridge.h"
#include "gb/fs.h"
#include "gb/render.h"
#include "gb/tests.h"
#include "gb/video.h"
#include "gb/timer.h"

struct gb *gb_create(struct args_boot *args)
{
	struct gb *gb;

	if ((gb = (struct gb *)malloc(sizeof(struct gb))) == NULL)
		return NULL;
	gb->cpu = NULL;
	gb->memory = NULL;
	gb->cartridge = NULL;
	gb->video = NULL;
	gb->args = args;
	return gb;
}

static void gb_reset(struct gb *gb)
{
	memory_reset(gb->cpu->memory);
	memory_bind_cartridge(gb->cpu->memory, gb->cartridge);
	video_reset(gb->video);
	cpu_reset(gb->cpu);
}

static void gb_init(struct gb *gb)
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

static u16 parse_hex_address(char *buf)
{
	char addr[4];

	for (int i = 0; i < 4; i++) {
		addr[i] = buf[i + 2];
	}
	return strtol(addr, NULL, 16);
}

static void print_byte_at(struct cpu *cpu, char *buf)
{
	u16 addr = parse_hex_address(buf);
	u8 byte = MEM_READ(cpu, addr);

	printf("Reading [$%04X] = $%02X\n", addr, byte);
}

static void gb_tick(struct gb *gb)
{
	cpu_tick(gb->cpu);
	video_tick(gb->video);
}

static void gb_goto(struct gb *gb, u16 address)
{
	while (gb->cpu->pc != address) {
		gb_tick(gb);
	}
}

static void gb_start_at(struct gb *gb)
{
	printf("Starting at $%04X\n", gb->args->start);
	gb_goto(gb, gb->args->start);
	gb->args->start = 0;
}

static void gb_release(struct gb *gb)
{
	memory_release(gb->memory);
	cpu_release(gb->cpu);
	video_release(gb->video);
	cartridge_release(gb->cartridge);
}

static char *gb_interactive(struct gb *gb)
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

static void gb_debug(struct gb *gb)
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
		// struct video RAM
		memory_debug(gb->memory, 0x8000, 0x9FFF);
	}
	if (gb->args->memory_dump) {
		memory_dump(gb->memory);
	}
}

static void *gb_thread_cpu(void *arg)
{
	struct gb *gb = (struct gb *)arg;

	if (gb->args->debug)
		printf("Create CPU thread\n");
	while (1) {
		if (gb->args->interactive) {
			gb_interactive(gb);
			cpu_debug(gb->cpu);
			timer_debug(gb->cpu);
		}
		gb_tick(gb);
		gb_debug(gb);
	}
	if (gb->args->debug)
		printf("Exit CPU thread\n");
	pthread_exit(NULL);
}

static void *gb_thread_gui(void *arg)
{
	struct gb *gb = (struct gb *)arg;

	if (gb->args->debug)
		printf("Create GUI thread\n");
	render_init(256 + 128, 512, gb->video->scale);
	while (render_is_running()) {
		render_begin();
		video_render(gb->video);
		render_end();
	}
	if (gb->args->debug)
		printf("Exit GUI thread\n");
	render_release();
	pthread_exit(NULL);
}

int gb_boot(void *args)
{
	pthread_t cpu;
	pthread_t gui;
	struct gb *gb;

	gb = gb_create(args);
	gb_init(gb);
	if (gb->args->debug)
		cartridge_metadata(gb->cartridge);
	if (gb->args->start != 0)
		gb_start_at(gb);
	pthread_create(&cpu, NULL, gb_thread_cpu, gb);
	if (gb->video->render) {
		pthread_create(&gui, NULL, gb_thread_gui, gb);
		pthread_join(gui, NULL);
	}
	pthread_join(cpu, NULL);
	gb_release(gb);
	return 0;
}

int gb_rom(void *args)
{
	struct args_rom *cargs = (struct args_rom *)args;
	struct cartridge *cartridge = cartridge_load_from_file(cargs->rom_path);

	cartridge_metadata(cartridge);
	cartridge_release(cartridge);
	return 0;
}

int gb_test(void *args)
{
	struct args_test *cargs = (struct args_test *)args;

	test_opcode(cargs->opcode, cargs->verbose, cargs->is_prefixed);
	return 0;
}

int gb_render(void *args)
{
	u8 *buffer;
	size_t size_in_bytes;
	struct args_render *cargs = (struct args_render *)args;
	FILE *dump = fopen(cargs->dump, "r");

	if (dump == NULL)
		return -1;
	size_in_bytes = fs_size(dump);
	buffer = fs_read(dump, size_in_bytes);
	if (buffer != NULL) {
		struct video *video = video_init(true);
		struct memory *memory = memory_init();
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
