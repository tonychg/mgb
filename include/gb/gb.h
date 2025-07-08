#ifndef _GB_H
#define _GB_H

#include "cartridge.h"
#include "cpu.h"
#include "video.h"
#include "cli.h"

struct gb {
	struct cpu *cpu;
	struct video *video;
	struct memory *memory;
	struct cartridge *cartridge;
	struct args_boot *args;
};

struct gb *gb_create(struct args_boot *args);
int gb_boot(void *args);
int gb_test(void *args);
int gb_rom(void *args);
int gb_render(void *args);

#endif
