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

void render_handle_inputs(u8 *joypad)
{
	// https://gbdev.io/pandocs/Joypad_Input.html#ff00--p1joyp-joypad
	*joypad |= 0x0;
	if (*joypad == 0x30)
		return;
	if ((*joypad & 0x20) == 0) {
		if (IsKeyDown(KEY_Q))
			*joypad |= 1 << 0;
		if (IsKeyDown(KEY_W))
			*joypad |= 1 << 1;
		if (IsKeyDown(KEY_E))
			*joypad |= 1 << 2;
		if (IsKeyDown(KEY_R))
			*joypad |= 1 << 3;
	}
	if ((*joypad & 0x10) == 0) {
		if (IsKeyDown(KEY_RIGHT))
			*joypad |= 1 << 0;
		if (IsKeyDown(KEY_LEFT))
			*joypad |= 1 << 1;
		if (IsKeyDown(KEY_UP))
			*joypad |= 1 << 2;
		if (IsKeyDown(KEY_DOWN))
			*joypad |= 1 << 3;
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
