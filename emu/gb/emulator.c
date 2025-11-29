#include "emu/gb.h"
#include "emu/memory.h"
#include "emu/sm83.h"
#include "platform/mm.h"
#include "platform/render.h"
#include "debugger.h"
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

static volatile int sigint_catcher = 0;

static void sigint_handler(int dummy)
{
	sigint_catcher = 1;
}

static u8 gb_load(struct sm83_core *cpu, u16 addr)
{
	struct gb_emulator *gb = (struct gb_emulator *)cpu->parent;
	return load_u8(gb->memory, addr);
}

static void gb_write(struct sm83_core *cpu, u16 addr, u8 value)
{
	struct gb_emulator *gb = (struct gb_emulator *)cpu->parent;
	if (addr == P1_JOYP) {
		u8 joyp = load_u8(gb->memory, P1_JOYP);
		value = (joyp & 0xF) | (value & 0x30);
	}
	write_u8(gb->memory, addr, value);
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
	gb->gpu->width = 256 + 128;
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
		cycles++;
		if (GB_FLAG(GB_VIDEO))
			ppu_tick(ctx->gb->gpu);
		// Throttling
		if (cycles >= 17476) {
			cycles = 0;
			wait_ms_after(&start_time, 16670);
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
			if (GB_FLAG(GB_VIDEO))
				ppu_tick(ctx->gb->gpu);
		}
	}
	pthread_exit(NULL);
}

static void draw(struct ppu *gpu, int x, int y, int color)
{
	render_pixel(x, y, color, gpu->scale);
}

static void handle_joypad(struct gb_context *ctx)
{
	struct ppu *gpu = ctx->gb->gpu;
	u8 joypad = load_u8(gpu->memory, P1_JOYP);
	u8 joypad_prev = joypad;
	render_handle_inputs(&joypad);
	if (joypad != joypad_prev) {
		request_interrupt(ctx->gb->memory, IRQ_JOYPAD);
		write_u8(gpu->memory, P1_JOYP, joypad);
	}
}

static void *run_emulator_gpu_thread(void *arg)
{
	struct gb_context *ctx = arg;
	struct ppu *gpu = ctx->gb->gpu;
	render_init(gpu->width, gpu->height, gpu->scale);
	gpu->renderer->draw = draw;
	while (render_is_running() && GB_FLAG(GB_ON)) {
		handle_joypad(ctx);
		render_begin();
		ppu_draw(ctx->gb->gpu);
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
