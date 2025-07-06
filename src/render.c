#include "render.h"

Color convert_color(DmgColors color)
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
}

bool render_is_running(void)
{
	return !WindowShouldClose();
}

void render_begin(void)
{
	BeginDrawing();
	// ClearBackground(convert_color(DMG_BLACK));
}

void render_end(void)
{
	EndDrawing();
}

void render_release(void)
{
	CloseWindow();
}
