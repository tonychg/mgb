#include "gb/render.h"
#include <raylib.h>
#include <stdio.h>

Color convert_color(enum dmg_colors color)
{
	int hex_color = (color << 8) | 0xFF;
	return GetColor(hex_color | 0xFF);
}

void render_pixel(int x, int y, int scale, u8 color)
{
	DrawRectangle(x * scale, y * scale, scale, scale,
		      convert_color(DMG_PALETTE[color]));
}

void render_init(int width, int height, int scale)
{
	InitWindow(width * scale, height * scale, "GB");
	SetTargetFPS(60);
	SetTraceLogLevel(LOG_ERROR);
	SetExitKey(KEY_Q);
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
