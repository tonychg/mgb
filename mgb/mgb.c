#include "platform/mm.h"
#include "platform/render.h"
#include "mgb/mgb.h"
#include "mgb/debugger.h"
#include <pthread.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

static volatile int sigint_catcher = 0;

static void sigint_handler(int dummy)
{
	sigint_catcher = 1;
}

static u8 gb_cpu_load(struct sm83_core *cpu, u16 addr)
{
	struct gb_emulator *gb = (struct gb_emulator *)cpu->parent;
	switch (addr) {
	case P1_JOYP:
		return update_joypad(gb);
	// case 0xC000 ... 0xDE00:
	// 	addr += 0x2000;
	}
	return gb->memory.ram[addr];
}

static void gb_cpu_write(struct sm83_core *cpu, u16 addr, u8 value)
{
	struct gb_emulator *gb = (struct gb_emulator *)cpu->parent;
	switch (addr) {
	case P1_JOYP: {
		gb->memory.ram[P1_JOYP] = value | 0x0f;
		update_joypad(gb);
		break;
	}
	case LY_LCD:
		return;
	case STAT_LCD: {
		u8 stat = gb->memory.ram[STAT_LCD];
		gb->memory.ram[addr] = (stat & 0x3) | (value & 0xfc);
		break;
	}
	case DMA_OAM_DMA: {
		// Starting DMA transfer
		sm83_schedule_dma_transfer(cpu, value * 0x100);
		return;
	}
	// case 0xC000 ... 0xDE00:
	// 	gb->memory.ram[addr] = value;
	// 	gb->memory.ram[addr + 0x2000] = value;
	// case 0xE000 ... 0xFE00:
	// 	gb->memory.ram[addr] = value;
	// 	gb->memory.ram[addr - 0x2000] = value;
	default:
		gb->memory.ram[addr] = value;
	}
}

static u8 *gb_load_offset(struct ppu *gpu, u16 offset)
{
	return ((struct gb_emulator*)gpu->parent)->memory.ram + offset;
}

static u8 gb_gpu_read(struct ppu *gpu, u16 addr)
{
	return ((struct gb_emulator*)gpu->parent)->memory.ram[addr];
}

static void gb_gpu_write(struct ppu *gpu, u16 addr, u8 value)
{
	((struct gb_emulator*)gpu->parent)->memory.ram[addr] = value;
}

static void init_devices(struct gb_emulator *gb)
{
	sm83_cpu_reset(&gb->cpu);
	gb->cpu.parent = gb;
	gb->cpu.memory.load8 = gb_cpu_load;
	gb->cpu.memory.write8 = gb_cpu_write;
	ppu_init(&gb->gpu);
	gb->gpu.parent = gb;
	gb->gpu.ram.load = gb_gpu_read;
	gb->gpu.ram.write = gb_gpu_write;
	gb->gpu.ram.offset = gb_load_offset;
	gb->gpu.width = 256 + GB_WIDTH;
	gb->gpu.height = 512;
}

static struct gb_emulator *init_gb_emulator()
{
	struct gb_emulator *gb;
	gb = (struct gb_emulator *)malloc(sizeof(struct gb_emulator));
	if (!gb)
		return NULL;
	init_devices(gb);
	return gb;
}

static void destroy_gb_emulator(struct gb_emulator *gb)
{
	zfree(gb);
}

static void gb_log_error(struct gb_context *ctx, char *msg)
{
	printf("[emulator] %s ", msg);
	ctx->exit_code = -1;
}

static void throttling(struct timeval *start_time, int milliseconds)
{
	unsigned long elapsed = 0;
	struct timeval now, diff;
	while (elapsed < milliseconds) {
		gettimeofday(&now, NULL);
		timersub(&now, start_time, &diff);
		elapsed = (unsigned long)diff.tv_usec * 1000;
	}
}

static void bind_debugger(struct debugger *dbg, struct gb_context *ctx)
{
	debugger_new(dbg);
	dbg->gb = ctx->gb;
	dbg->gb->cpu = ctx->gb->cpu;
	dbg->gb->gpu = ctx->gb->gpu;
	dbg->gb->memory = ctx->gb->memory;
}

static void run_cpu_debugger(struct gb_context *ctx)
{
	struct debugger dbg;
	struct timeval start_time;
	int cycles = 0;
	bind_debugger(&dbg, ctx);
	while (dbg.state != STATE_QUIT) {
		gettimeofday(&start_time, NULL);
		if (sigint_catcher) {
			dbg.state = STATE_WAIT;
			sigint_catcher = 0;
		}
		if (debugger_step(&dbg))
			break;
		ppu_tick(&ctx->gb->gpu, &ctx->gb->cpu);
		if (!ctx->gb->cpu.halted && GB_FLAG(GB_THROTTLING)) {
			cycles++;
			// Throttling
			if (cycles >= 17476) {
				cycles -= 17476;
				throttling(&start_time, 16670);
			}
		}
	}
}

static void *run_emulator_cpu_thread(void *arg)
{
	struct gb_context *ctx = arg;
	signal(SIGINT, sigint_handler);
	if (GB_FLAG(GB_DEBUG)) {
		run_cpu_debugger(ctx);
		GB_FLAG_DISABLE(GB_ON);
	} else {
		while (GB_FLAG(GB_ON)) {
			if (sigint_catcher)
				GB_FLAG_DISABLE(GB_ON);
			sm83_cpu_step(&ctx->gb->cpu);
			ppu_tick(&ctx->gb->gpu, &ctx->gb->cpu);
		}
	}
	pthread_exit(NULL);
}

static void draw(struct ppu *gpu, int x, int y, int color)
{
	render_pixel(x, y, color, gpu->scale);
}

static void draw_debug_gui(struct ppu *gpu, struct gb_context *ctx)
{
	render_debug("Dots: %d", gpu->dots, 20, gpu->scale * 384, 20);
	render_debug("Frames: %d", gpu->frames, 20, gpu->scale * 404, 20);
	render_debug("LY: %d", gpu->ly, 20, gpu->scale * 424, 20);
	render_debug("Cycles: %d", ctx->gb->cpu.cycles, 20, ctx->scale * 444,
		     20);
}

static void *run_emulator_gpu_thread(void *arg)
{
	struct gb_context *ctx = arg;

	ctx->gb->gpu.renderer.draw = draw;

	render_init(ctx->gb->gpu.width, ctx->gb->gpu.height,
		    ctx->gb->gpu.scale);
	while (render_is_running() && GB_FLAG(GB_ON)) {
		render_handle_inputs(&ctx->gb->keys);
		render_begin();
		ClearBackground(BLACK);
		ppu_draw(&ctx->gb->gpu);
		draw_debug_gui(&ctx->gb->gpu, ctx);
		render_end();
	}
	render_release();
	pthread_exit(NULL);
}

void gb_stop_emulator(struct gb_context *ctx)
{
	destroy_gb_emulator(ctx->gb);
}

int gb_start_emulator(struct gb_context *ctx)
{
	pthread_t thread_cpu;
	pthread_t thread_gpu;

	if (!(ctx->gb = init_gb_emulator()))
		gb_log_error(ctx, "failed to initialize emulator");
	if (load_rom(&ctx->gb->memory, ctx->rom_path))
		gb_log_error(ctx, "failed to load ROM into emulator");
	pthread_create(&thread_cpu, NULL, run_emulator_cpu_thread, ctx);
	if (GB_FLAG(GB_VIDEO)) {
		ctx->gb->gpu.scale = ctx->scale;
		printf("Resolution: %dx%d Scale: %d\n", ctx->gb->gpu.width,
		       ctx->gb->gpu.height, ctx->gb->gpu.scale);
		pthread_create(&thread_gpu, NULL, run_emulator_gpu_thread, ctx);
		pthread_join(thread_gpu, NULL);
	}
	pthread_join(thread_cpu, NULL);
	return ctx->exit_code;
}

// clang-format off
struct gb_option options[] = {
	{ "-d/--debug         Enable debugger", "--debug", "-d", 0, GB_OPTION_DEBUG },
	{ "-r/--rom <path>    Path of the ROM", "--rom", "-r", 1, GB_OPTION_ROM },
	{ "-n/--no-video      Disable video rendering", "--no-video", "-n", 0, GB_OPTION_NO_VIDEO },
	{ "-s/--scale <int>   Scale viewport", "--scale", "-s", 1, GB_OPTION_SCALE },
	{ "-t/--throttling    Enable throttling", "--throttling", "-t", 0, GB_OPTION_THROTTLING },
};
// clang-format on

static void print_help()
{
	printf("usage: gb [ARGS]\n");
	for (int i = 0; i < ARRAY_SIZE(options); i++)
		printf("   %s\n", options[i].description);
}

static void gb_context_init(struct gb_context *ctx)
{
	ctx->flags = 0;
	ctx->rom_path = NULL;
	ctx->scale = 1;
	GB_FLAG_ENABLE(GB_VIDEO);
	GB_FLAG_ENABLE(GB_ON);
}

static void parse_option(struct gb_context *ctx, int i, int argc, char **argv)
{
	for (int j = 0; j < ARRAY_SIZE(options); j++) {
		if (strcmp(argv[i], options[j].l) &&
		    strcmp(argv[i], options[j].s)) {
			continue;
		}
		switch (options[j].type) {
		case GB_OPTION_DEBUG:
			GB_FLAG_ENABLE(GB_DEBUG);
			break;
		case GB_OPTION_NO_VIDEO:
			GB_FLAG_DISABLE(GB_VIDEO);
			break;
		case GB_OPTION_THROTTLING:
			GB_FLAG_DISABLE(GB_THROTTLING);
			break;
		case GB_OPTION_ROM:
			if (i + 1 < argc)
				ctx->rom_path = argv[i + 1];
			break;
		case GB_OPTION_SCALE:
			if (i + 1 < argc)
				ctx->scale = atoi(argv[i + 1]);
			break;
		}
		break;
	}
}

static void print_header(struct gb_context *ctx)
{
	printf("Starting up ...\n");
	printf("Debug: %s ", GB_FLAG(GB_DEBUG) ? "On" : "Off");
	printf("Video: %s ", GB_FLAG(GB_VIDEO) ? "On" : "Off");
	printf("\n");
	printf("Throttling: %s\n", GB_FLAG(GB_THROTTLING) ? "On" : "Off");
	printf("Rom: %s\n", ctx->rom_path ? ctx->rom_path : "Not loaded");
	printf("Scale: %d\n", ctx->scale);
}

static int context_create(struct gb_context *ctx, int argc, char **argv)
{
	gb_context_init(ctx);
	for (int i = 1; i < argc; i++)
		parse_option(ctx, i, argc, argv);
	if (!ctx->rom_path)
		return -1;
	if (ctx->scale < 1)
		return -1;
	return 0;
}

static int emulator_run(int argc, char **argv)
{
	struct gb_context ctx;
	if (context_create(&ctx, argc, argv)) {
		print_help();
		return 0;
	} else {
		print_header(&ctx);
	}
	gb_start_emulator(&ctx);
	gb_stop_emulator(&ctx);
	return ctx.exit_code;
}

int main(int argc, char **argv)
{
	if (emulator_run(argc, argv))
		printf("emulator terminate with errors\n");
	return 0;
}
