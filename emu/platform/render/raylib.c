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

void render_debug(char *label, int value, int x, int y, int height)
{
	char str[256];
	sprintf(str, label, value);
	DrawText(str, x, y, height, RED);
}

void render_handle_inputs(u8 *keys)
{
	for (int i = 0; i < 8; i++) {
		struct keybind key = keybindings[0 + i];
		if (IsKeyDown(key.keyboard)) {
			*keys |= (1 << key.button);
			return;
		}
	}
	*keys = 0;
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
