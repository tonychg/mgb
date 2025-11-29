#include "platform/render.h"
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

void render_handle_inputs(void *ctx, void (callback)(void*, enum joypad_button))
{
	for (int i = 0; i < 8; i++) {
		struct keybind key = keybindings[0 + i];
		if (IsKeyDown(key.keyboard)) {
			callback(ctx, key.button);
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
