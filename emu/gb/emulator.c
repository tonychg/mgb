#include "emu/gb.h"
#include "emu/memory.h"
#include "emu/sm83.h"
#include "platform/mm.h"
#include "platform/render.h"
#include "debugger.h"
#include <pthread.h>
#include <raylib.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

static volatile int sigint_catcher = 0;

static void sigint_handler(int dummy)
{
	sigint_catcher = 1;
}

static void gb_dma_transfer(struct gb_emulator *gb, u16 start_addr)
{
	for (int i = 0; i < 160; i++) {
		// printf("DMA transfer triggered with start address %04X %04X\n", 0xFE00 + i, start_addr + i);
		gb->memory->array->bytes[0xFE00 + i] =
			gb->memory->array->bytes[start_addr + i];
	}
}

static u8 gb_load(struct sm83_core *cpu, u16 addr)
{
	struct gb_emulator *gb = (struct gb_emulator *)cpu->parent;
	switch (addr) {
	case P1_JOYP:
		return update_joypad(gb);
	}
	return load_u8(gb->memory, addr);
}

static void gb_write(struct sm83_core *cpu, u16 addr, u8 value)
{
	struct gb_emulator *gb = (struct gb_emulator *)cpu->parent;
	switch (addr) {
	case P1_JOYP: {
		gb->memory->array->bytes[P1_JOYP] = value | 0x0f;
		update_joypad(gb);
		break;
	}
	case LY_LCD:
		return;
	case STAT_LCD: {
		u8 stat = load_u8(gb->memory, STAT_LCD);
		gb->memory->array->bytes[addr] = (stat & 0x3) | (value & 0xfc);
		break;
	}
	case DMA_OAM_DMA: {
		// Starting DMA transfer
		gb->memory->array->bytes[addr] = value;
		gb_dma_transfer(gb, value * 0x100);
		break;
	}
	default:
		write_u8(gb->memory, addr, value);
	}
}

static int init_devices(struct gb_emulator *gb)
{
	gb->memory = allocate_memory();
	if (!gb->memory)
		goto err;
	gb->cpu = sm83_init();
	if (!gb->cpu)
		goto free_memory;
	gb->cpu->parent = gb;
	gb->cpu->memory->load8 = gb_load;
	gb->cpu->memory->write8 = gb_write;
	gb->gpu = ppu_init();
	gb->gpu->memory = gb->memory;
	gb->gpu->width = 256 + GB_WIDTH;
	gb->gpu->height = 512;
	if (!gb->gpu)
		goto free_cpu;
	return 0;
free_cpu:
	sm83_destroy(gb->cpu);
free_memory:
	destroy_memory(gb->memory);
err:
	zfree(gb);
	return -1;
}

static struct gb_emulator *init_gb_emulator()
{
	struct gb_emulator *gb;
	gb = (struct gb_emulator *)malloc(sizeof(struct gb_emulator));
	if (!gb || init_devices(gb)) {
		zfree(gb);
		return NULL;
	}
	return gb;
}

static void destroy_gb_emulator(struct gb_emulator *gb)
{
	ppu_destroy(gb->gpu);
	sm83_destroy(gb->cpu);
	destroy_memory(gb->memory);
	zfree(gb);
}

static void gb_log_error(struct gb_context *ctx, char *msg)
{
	printf("[emulator] %s ", msg);
	ctx->exit_code = -1;
}

static void wait_ms_after(struct timeval *start_time, int milliseconds)
{
	unsigned long elapsed = 0;
	struct timeval now, diff;
	while (elapsed < milliseconds) {
		gettimeofday(&now, NULL);
		timersub(&now, start_time, &diff);
		elapsed = (unsigned long)diff.tv_usec * 1000;
	}
}

static void run_cpu_debugger(struct gb_context *ctx)
{
	struct debugger_context debugger_ctx;
	struct timeval start_time;
	int cycles = 0;
	debugger_new(&debugger_ctx);
	debugger_ctx.memory = ctx->gb->memory;
	debugger_ctx.cpu = ctx->gb->cpu;
	while (debugger_ctx.state != STATE_QUIT) {
		gettimeofday(&start_time, NULL);
		if (sigint_catcher) {
			debugger_ctx.state = STATE_WAIT;
			sigint_catcher = 0;
		}
		if (debugger_step(&debugger_ctx))
			break;
		ppu_tick(ctx->gb->gpu, ctx->gb->cpu);
		if (!ctx->gb->cpu->halted && GB_FLAG(GB_THROTTLING)) {
			cycles++;
			// Throttling
			if (cycles >= 17476) {
				cycles -= 17476;
				wait_ms_after(&start_time, 16670);
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
			sm83_cpu_step(ctx->gb->cpu);
			ppu_tick(ctx->gb->gpu, ctx->gb->cpu);
		}
	}
	pthread_exit(NULL);
}

static void draw(struct ppu *gpu, int x, int y, int color)
{
	render_pixel(x, y, color, gpu->scale);
}

static void *run_emulator_gpu_thread(void *arg)
{
	struct gb_context *ctx = arg;
	struct ppu *gpu = ctx->gb->gpu;
	render_init(gpu->width, gpu->height, gpu->scale);
	gpu->renderer->draw = draw;
	while (render_is_running() && GB_FLAG(GB_ON)) {
		render_handle_inputs(&ctx->gb->keys);
		render_begin();
		ClearBackground(BLACK);
		ppu_draw(ctx->gb->gpu);
		render_debug("Dots: %d", gpu->dots, 20, ctx->scale * 384, 20);
		render_debug("Frames: %d", gpu->frames, 20, ctx->scale * 404,
			     20);
		render_debug("LY: %d", gpu->ly, 20, ctx->scale * 424, 20);
		render_debug("Cycles: %d", ctx->gb->cpu->cycles, 20,
			     ctx->scale * 444, 20);
		render_end();
	}
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
	if (load_rom(ctx->gb->memory, ctx->rom_path))
		gb_log_error(ctx, "failed to load ROM into emulator");
	pthread_create(&thread_cpu, NULL, run_emulator_cpu_thread, ctx);
	if (GB_FLAG(GB_VIDEO)) {
		ctx->gb->gpu->scale = ctx->scale;
		printf("Resolution: %dx%d Scale: %d\n", ctx->gb->gpu->width,
		       ctx->gb->gpu->height, ctx->gb->gpu->scale);
		pthread_create(&thread_gpu, NULL, run_emulator_gpu_thread, ctx);
		pthread_join(thread_gpu, NULL);
	}
	pthread_join(thread_cpu, NULL);
	return ctx->exit_code;
}
