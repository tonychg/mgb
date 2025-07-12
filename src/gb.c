#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "gb/alloc.h"
#include "gb/memory.h"
#include "gb/sm83.h"
#include "gb/gb.h"
#include "gb/cartridge.h"
#include "gb/fs.h"
#include "gb/render.h"
#include "gb/video.h"

static u8 gb_load8(struct sm83_core *cpu, u16 addr)
{
	struct gb *gb = (struct gb *)cpu->parent;

	if (gb->card && addr < 0x8000) {
		return gb->card->buffer[addr];
	}
	return memory_read(gb->bus, addr);
}

static void gb_write8(struct sm83_core *cpu, u16 addr, u8 value)
{
	struct gb *gb = (struct gb *)cpu->parent;

	memory_write(gb->bus, addr, value);
}

static u16 gb_load16(struct sm83_core *cpu, u16 addr)
{
	return unsigned_16(cpu->memory->load8(cpu, addr),
			   cpu->memory->load8(cpu, addr + 1));
}

static u8 gb_read_segment(struct sm83_core *cpu)
{
	return cpu->memory->load8(cpu, cpu->pc);
}

struct gb *gb_alloc()
{
	struct gb *gb;

	gb = (struct gb *)malloc(sizeof(struct gb));
	if (!gb)
		return NULL;
	gb->args = NULL;
	gb->card = NULL;
	gb->cpu = NULL;
	gb->ppu = NULL;
	gb->bus = NULL;
	return gb;
}

void gb_configure(struct gb *gb, struct args_boot *args)
{
	gb->args = args;
}

void gb_init(struct gb *gb)
{
	gb->cpu = sm83_init();
	gb->ppu = video_init(false);
	gb->bus = memory_init();
	gb->cpu->parent = gb;
	gb->cpu->memory->load8 = &gb_load8;
	gb->cpu->memory->load16 = &gb_load16;
	gb->cpu->memory->write8 = &gb_write8;
	gb->cpu->memory->read_segment = &gb_read_segment;
	gb->ppu->memory = gb->bus;
	sm83_cpu_reset(gb->cpu);
	memory_reset(gb->bus);
}

static void gb_tick(struct gb *gb)
{
	sm83_cpu_step(gb->cpu);
	video_tick(gb->ppu);
}

static void gb_destroy(struct gb *gb)
{
	memory_release(gb->bus);
	sm83_destroy(gb->cpu);
	video_release(gb->ppu);
	if (gb->card)
		cartridge_release(gb->card);
}

static void *gb_thread_cpu(void *arg)
{
	struct gb *gb = (struct gb *)arg;
	char *decoded;

	if (gb->args->debug)
		printf("Create CPU thread\n");
	if (gb->args->interactive) {
		debugger_event_loop(gb->cpu);
	} else {
		while (1) {
			gb_tick(gb);
			if (gb->args->debug) {
				decoded = sm83_disassemble(gb->cpu);
				printf("%s\n", decoded);
				zfree(decoded);
			}
		}
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
	render_init(256 + 128, 512, gb->ppu->scale);
	while (render_is_running()) {
		render_begin();
		video_render(gb->ppu);
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

	gb = gb_alloc();
	gb_init(gb);
	gb_configure(gb, args);
	gb->card = cartridge_load_from_file(gb->args->rom_path);
	if (gb->args->debug)
		cartridge_metadata(gb->card);
	pthread_create(&cpu, NULL, gb_thread_cpu, gb);
	if (gb->args->render) {
		pthread_create(&gui, NULL, gb_thread_gui, gb);
		pthread_join(gui, NULL);
	}
	pthread_join(cpu, NULL);
	gb_destroy(gb);
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
