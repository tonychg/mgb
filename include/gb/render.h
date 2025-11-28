#ifndef _RENDER_H
#define _RENDER_H

#include "types.h"
#include <raylib.h>

enum dmg_colors {
	DMG_WHITE = 0x9A9E3F,
	DMG_LIGHTGRAY = 0x496B22,
	DMG_DARKGRAY = 0x0E450B,
	DMG_BLACK = 0x1B2A09,
};

static const int DMG_PALETTE[4] = {
	DMG_WHITE,
	DMG_LIGHTGRAY,
	DMG_DARKGRAY,
	DMG_BLACK,
};

void render_pixel(int x, int y, int scale, u8 color);
void render_init(int width, int height, int scale);
bool render_is_running(void);
void render_debug(int dots, int frames);
void render_begin(void);
void render_end(void);
void render_release(void);

#endif
