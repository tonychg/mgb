#ifndef _RENDER_H
#define _RENDER_H

#include "emu/joypad.h"
#include <raylib.h>
#include <stdbool.h>

struct keybind {
	int keyboard;
	enum joypad_button button;
};

// clang-format off
static const struct keybind keybindings[] = {
	{ KEY_Q, BUTTON_A },
	{ KEY_W, BUTTON_B },
	{ KEY_R, BUTTON_SELECT },
	{ KEY_E, BUTTON_START },
	{ KEY_RIGHT, BUTTON_RIGHT },
	{ KEY_LEFT, BUTTON_LEFT },
	{ KEY_UP, BUTTON_UP },
	{ KEY_DOWN, BUTTON_DOWN },
};
// clang-format on

void render_pixel(int x, int y, int color, int scale);
void render_init(int width, int height, int scale);
bool render_is_running(void);
void render_handle_inputs(void *ctx,
			  void(callback)(void *, enum joypad_button));
void render_debug(int dots, int frames);
void render_begin(void);
void render_end(void);
void render_release(void);

#endif
