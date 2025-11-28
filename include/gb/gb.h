#ifndef _GB_H
#define _GB_H

#include "cartridge.h"
#include "video.h"

struct gb {
	struct sm83_core *cpu;
	struct video *ppu;
	struct memory *bus;
	struct cartridge *card;
};

enum gb_option_types {
	GB_OPTION_DEBUG,
	GB_OPTION_ROM,
	GB_OPTION_NO_VIDEO,
};

struct gb_option {
	const char *description;
	const char *l;
	const char *s;
	const int length;
	enum gb_option_types type;
};

struct gb_context {
	struct gb *gb;
	char *rom_path;
	bool debug;
	bool video;
	bool running;
};

int gb_boot(struct gb_context *ctx, int argc, char **argv);

#endif
