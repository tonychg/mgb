#ifndef _RENDER_H
#define _RENDER_H

#include "emu/joypad.h"
#include "platform/types.h"
#include <raylib.h>
#include <stdbool.h>

struct keybind {
	int keyboard;
	enum joypad_button button;
	const char *label;
};

// clang-format off
static const struct keybind keybindings[] = {
	{ KEY_Q, BUTTON_A, "A", },
	{ KEY_W, BUTTON_B, "B", },
	{ KEY_R, BUTTON_SELECT, "SELECT", },
	{ KEY_E, BUTTON_START, "START" },
	{ KEY_RIGHT, BUTTON_RIGHT, "RIGHT" },
	{ KEY_LEFT, BUTTON_LEFT, "LEFT" },
	{ KEY_UP, BUTTON_UP, "UP" },
	{ KEY_DOWN, BUTTON_DOWN, "DOWN" },
};
// clang-format on

void render_pixel(int x, int y, int color, int scale);
void render_init(int width, int height, int scale);
bool render_is_running(void);
void render_handle_inputs(u8 *keys);
void render_debug(char *label, int value, int x, int y, int height);
void render_begin(void);
void render_end(void);
void render_release(void);

#endif
