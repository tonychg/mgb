#ifndef __GB_H__
#define __GB_H__

#include "cartridge.h"
#include "cpu.h"
#include "video.h"
#include "cli.h"

typedef struct Gb {
	Cpu *cpu;
	Video *video;

	Memory *memory;
	Cartridge *cartridge;

	ArgsBoot *args;
} Gb;

Gb *gb_create(ArgsBoot *args);
void gb_init(Gb *gb);
void gb_reset(Gb *gb);
int gb_boot(void *args);
int gb_test(void *args);
int gb_rom(void *args);
int gb_render(void *args);

#endif
