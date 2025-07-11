#ifndef _GB_H
#define _GB_H

#include "cartridge.h"
#include "video.h"
#include "cli.h"

struct gb {
	struct sm83_core *cpu;
	struct video *ppu;
	struct memory *bus;
	struct cartridge *card;
	struct args_boot *args;
};

struct gb *gb_create(struct args_boot *args);
int gb_boot(void *args);
int gb_test(void *args);
int gb_rom(void *args);
int gb_render(void *args);

#endif
