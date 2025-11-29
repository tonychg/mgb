#ifndef _GB_H
#define _GB_H

#include "platform/types.h"
#include "emu/sm83.h"
#include "emu/memory.h"
#include "emu/ppu.h"

enum gb_option_type {
	GB_OPTION_DEBUG,
	GB_OPTION_ROM,
	GB_OPTION_NO_VIDEO,
	GB_OPTION_SCALE,
};

enum gb_flags {
	GB_ON = 1,
	GB_DEBUG = 1 << 1,
	GB_VIDEO = 1 << 2,
};

#define GB_FLAG(flag) (ctx->flags & flag) != 0
#define GB_FLAG_DISABLE(flag) ctx->flags ^= flag
#define GB_FLAG_ENABLE(flag) ctx->flags |= flag
#define GB_DEFAULT_FLAGS GB_ON | GB_VIDEO

struct gb_option {
	const char *description;
	const char *l;
	const char *s;
	const int length;
	enum gb_option_type type;
};

struct gb_emulator {
	struct sm83_core *cpu;
	struct ppu *gpu;
	struct shared *memory;
};

struct gb_context {
	struct gb_emulator *gb;
	char *rom_path;
	u8 flags;
	int exit_code;
	int scale;
	u8 keys;
};

int gb_start_emulator(struct gb_context *ctx);
void gb_stop_emulator(struct gb_context *ctx);

#endif
