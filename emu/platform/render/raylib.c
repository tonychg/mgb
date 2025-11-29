#include "emu/gb.h"
#include "platform/render.h"
#include <raylib.h>
#include <stdio.h>

static Color convert_color(int color)
{
	int hex_color = (color << 8) | 0xFF;
	return GetColor(hex_color | 0xFF);
}

void render_pixel(int x, int y, int color, int scale)
{
	DrawRectangle(x * scale, y * scale, scale, scale, convert_color(color));
}

void log_callback(int msgType, const char *text, va_list args)
{
	return;
}

void render_init(int width, int height, int scale)
{
	SetTraceLogLevel(LOG_ERROR);
	SetTraceLogCallback(log_callback);
	SetTargetFPS(60);
	InitWindow(width * scale, height * scale, "GB");
}

void render_debug(int dots, int frames)
{
	char str_dots[256];
	char str_frames[256];
	sprintf(str_dots, "Dots: %d", dots);
	sprintf(str_frames, "Frames: %d", frames);
	DrawText(str_dots, 10, 40, 20, RED);
	DrawText(str_frames, 10, 60, 20, RED);
}

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

void render_handle_inputs(u8 *joypad)
{
	// https://gbdev.io/pandocs/Joypad_Input.html#ff00--p1joyp-joypad
	int start = 0;
	u8 prev = *joypad;
	if ((*joypad & 0x30) == 0) {
		*joypad |= 0xF;
		return;
	}
	if ((*joypad & 0x10) == 0)
		start = 4;
	for (int i = 0; i < 4; i++) {
		struct keybind key = keybindings[start + i];
		if (IsKeyDown(key.keyboard)) {
			*joypad &= ~(1 << key.button);
			printf("Keypressed %d JOYPAD: %06b -> %06b\n",
			       key.keyboard, prev, *joypad);
		}
	}
}

bool render_is_running(void)
{
	return !WindowShouldClose();
}

void render_begin(void)
{
	BeginDrawing();
}

void render_end(void)
{
	EndDrawing();
}

void render_release(void)
{
	CloseWindow();
}
