#include "gb/alloc.h"
#include "gb/memory.h"
#include "gb/sm83.h"
#include "gb/debugger.h"
#include "gb/gb.h"
#include "gb/cartridge.h"
#include "gb/render.h"
#include "gb/video.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

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
	gb->card = NULL;
	gb->cpu = NULL;
	gb->ppu = NULL;
	gb->bus = NULL;
	return gb;
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

static void gb_destroy(struct gb *gb)
{
	memory_release(gb->bus);
	sm83_destroy(gb->cpu);
	video_release(gb->ppu);
	if (gb->card)
		cartridge_release(gb->card);
}

static volatile int sigint_catcher = 0;

static void sigint_handler(int dummy)
{
	sigint_catcher = 1;
}

// clang-format off
struct gb_option gb_options[] = {
	{ "-d/--debug     Enable debugger", "--debug", "-d", 0, GB_OPTION_DEBUG },
	{ "-r/--rom       Path of the ROM", "--rom", "-r", 1, GB_OPTION_ROM },
	{ "-n/--no-video  Disable video rendering", "--no-video", "-n", 0, GB_OPTION_NO_VIDEO },
};
// clang-format on

static int gb_setup_context(struct gb_context *ctx, int argc, char **argv)
{
	if (argc < 2)
		return -1;
	ctx->video = true;
	ctx->debug = false;
	ctx->rom_path = NULL;
	ctx->running = true;
	for (int i = 1; i < argc; i++) {
		for (int j = 0; j < ARRAY_SIZE(gb_options); j++) {
			struct gb_option opt = gb_options[j];
			if (!strcmp(argv[i], opt.l)) {
				switch (opt.type) {
				case GB_OPTION_DEBUG:
					ctx->debug = true;
					break;
				case GB_OPTION_NO_VIDEO:
					ctx->video = false;
					break;
				case GB_OPTION_ROM:
					if (i + 1 < argc)
						ctx->rom_path = argv[i + 1];
					break;
				}
				break;
			}
		}
	}
	return 0;
}

static void *gb_thread_cpu(void *context)
{
	struct gb_context *ctx = context;
	signal(SIGINT, sigint_handler);
	if (ctx->debug) {
		struct debugger_context debugger_ctx;
		debugger_init(&debugger_ctx, ctx->gb->cpu, ctx->gb->bus->bus);
		while (debugger_ctx.state != STATE_QUIT) {
			if (sigint_catcher) {
				debugger_ctx.state = STATE_WAIT;
				sigint_catcher = 0;
			}
			if (debugger_step(&debugger_ctx))
				break;
			video_tick(ctx->gb->ppu);
		}
		ctx->running = false;
	} else {
		while (ctx->running) {
			if (sigint_catcher)
				ctx->running = false;
			sm83_cpu_step(ctx->gb->cpu);
			video_tick(ctx->gb->ppu);
		}
	}
	pthread_exit(NULL);
}

static void *gb_thread_gpu(void *context)
{
	struct gb_context *ctx = context;
	render_init(256 + 128, 512, ctx->gb->ppu->scale);
	while (render_is_running() && ctx->running) {
		render_begin();
		video_render(ctx->gb->ppu);
		render_debug(ctx->gb->ppu->dots, ctx->gb->ppu->frames);
		render_end();
	}
	pthread_exit(NULL);
}

int gb_boot(struct gb_context *ctx, int argc, char **argv)
{
	pthread_t thread_cpu;
	pthread_t thread_gpu;

	if (gb_setup_context(ctx, argc, argv)) {
		printf("Invalid arguments\n");
		return -1;
	}
	printf("Rom: %s\n", ctx->rom_path);
	printf("Debug: %d, Video: %d\n", ctx->debug, ctx->video);
	ctx->gb = gb_alloc();
	gb_init(ctx->gb);
	ctx->gb->card = cartridge_load_from_file(ctx->rom_path);
	if (!ctx->gb->card) {
		printf("Failed to load cartridge: %s\n", ctx->rom_path);
		return -1;
	}
	pthread_create(&thread_cpu, NULL, gb_thread_cpu, ctx);
	if (ctx->video) {
		pthread_create(&thread_gpu, NULL, gb_thread_gpu, ctx);
		pthread_join(thread_gpu, NULL);
	}
	pthread_join(thread_cpu, NULL);
	gb_destroy(ctx->gb);
	return 0;
}
