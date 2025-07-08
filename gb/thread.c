#include "gb/thread.h"
#include "gb/cartridge.h"
#include "gb/gb.h"
#include "gb/render.h"
#include "gb/timer.h"
#include <stdio.h>

void *thread_cpu(void *arg)
{
	Gb *gb = (Gb *)arg;

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

void *thread_gui(void *arg)
{
	Gb *gb = (Gb *)arg;

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

void thread_boot(ArgsBoot *args)
{
	pthread_t cpu;
	pthread_t gui;
	Gb *gb;

	gb = gb_create(args);
	gb_init(gb);
	if (gb->args->debug)
		cartridge_metadata(gb->cartridge);
	if (gb->args->start != 0)
		gb_start_at(gb);
	pthread_create(&cpu, NULL, thread_cpu, gb);
	if (gb->video->render) {
		pthread_create(&gui, NULL, thread_gui, gb);
		pthread_join(gui, NULL);
	}
	pthread_join(cpu, NULL);
	gb_release(gb);
}
