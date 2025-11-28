#include "gb/video.h"
#include "gb/alloc.h"
#include "gb/render.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

struct video *video_init(bool render)
{
	struct video *video;

	video = (struct video *)malloc(sizeof(struct video));
	if (!video)
		return NULL;
	video->memory = NULL;
	video->render = render;
	video->scale = 1;
	return video;
}

void video_bind_memory(struct video *video, struct memory *memory)
{
	video->memory = memory;
}

void video_release(struct video *video)
{
	zfree(video);
}

void video_reset(struct video *video)
{
	video->ly = 144;
	video->x = 0;
	video->stat = 1;
	video->dots = 0;
	video->mode = MODE_1;
}

void video_memory_fetch(struct video *video)
{
	u8 lcdc_lcd = MEM_READ(video, LCDC_LCD);

	if ((lcdc_lcd >> 7) == 1)
		video->enabled = true;
	else
		video->enabled = false;
	if (video->enabled)
		video->stat = MEM_READ(video, STAT_LCD);
}

void video_render(struct video *video)
{
	video_render_tiles(video->memory->bus + 0x8000, video->scale);
	video_render_tilemap(video->memory->bus, 0, 128, 0, video->scale);
	video_render_tilemap(video->memory->bus, 1, 128, 256, video->scale);
}

static void gui_inputs(struct video *video)
{
	u8 joyp = video->memory->bus[0xFF00];

	if (!(joyp >> 4) && !(joyp >> 5)) {
		if (IsKeyDown(KEY_A) || IsKeyDown(KEY_Z) || IsKeyDown(KEY_E) ||
		    IsKeyDown(KEY_R) || IsKeyDown(KEY_RIGHT) ||
		    IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_UP) ||
		    IsKeyDown(KEY_DOWN)) {
			joyp |= 0xF;
		}
	} else if (!(joyp >> 4)) {
		if (IsKeyDown(KEY_A)) {
			printf("A is pressed\n");
			joyp |= 1 << 0;
		}
		if (IsKeyDown(KEY_Z)) {
			printf("B is pressed\n");
			joyp |= 1 << 1;
		}
		if (IsKeyDown(KEY_E)) {
			printf("Select is pressed\n");
			joyp |= 1 << 2;
		}
		if (IsKeyDown(KEY_R)) {
			printf("Start is pressed\n");
			joyp |= 1 << 3;
		}
	} else if (!(joyp >> 5)) {
		if (IsKeyDown(KEY_RIGHT)) {
			printf("Right is pressed\n");
			joyp |= 1 << 0;
		}
		if (IsKeyDown(KEY_LEFT)) {
			printf("Left is pressed\n");
			joyp |= 1 << 1;
		}
		if (IsKeyDown(KEY_UP)) {
			printf("Up is pressed\n");
			joyp |= 1 << 2;
		}
		if (IsKeyDown(KEY_DOWN)) {
			printf("Down is pressed\n");
			joyp |= 1 << 3;
		}
	}
	video->memory->bus[0xFF] = joyp;
}

void video_tick(struct video *video)
{
	video_memory_fetch(video);
	gui_inputs(video);
	if (video->enabled) {
		if (video->dots != 0 && (video->dots % 456) == 0) {
			// video_debug(video);
			video->ly++;
			MEM_WRITE(video, LY_LCD, video->ly);
		}
		video->dots++;
	}
	if (video->dots == GB_VIDEO_TOTAL_LENGTH) {
		video->frames++;
		video_reset(video);
	}
}

void video_debug(struct video *video)
{
	printf("  Mode = %d | Enabled = %d\n", video->mode, video->enabled);
	printf("    ly = %d | x = %d\n", video->ly, video->x);
	printf("LY_LCD = %X | LYC_LY = %X\n", MEM_READ(video, LY_LCD),
	       MEM_READ(video, LYC_LY));
	printf("Frames = %d | Dots = %d\n", video->frames, video->dots);
	printf("        Stat = %08b\n", video->stat);
}

void video_render_tile(u8 *vram, int n, int x, int y, int scale)
{
	int j = 0;
	int indice = n * 16;

	for (int b = 0; b < 16; b = b + 2) {
		int i = 0;
		u8 right = vram[indice + b];
		u8 left = vram[indice + b + 1];
		for (int bit = 7; bit >= 0; bit--) {
			int color = video_pixel_color(right, left, bit);
			render_pixel(x * 8 + i, y * 8 + j, scale, color);
			i++;
		}
		j++;
	}
}

u8 video_pixel_color(u8 right, u8 left, u8 bit)
{
	u8 lsb, msb;
	u8 pixel;
	u8 mask = 1 << bit;

	msb = ((left & mask) >> bit) << 1;
	lsb = ((right & mask) >> bit);
	pixel = msb | lsb;
	return pixel;
}

void video_render_tiles(u8 *vram, int scale)
{
	for (int y = 0; y <= 23; y++) {
		for (int x = 0; x < 16; x++) {
			int i = (y * 16) + x;
			video_render_tile(vram, i, x, y, scale);
		}
	}
}

void video_render_tilemap(u8 *vram, u8 area, int x, int y, int scale)
{
	u8 *tilemap;

	tilemap = vram + (area == 0 ? 0x9800 : 0x9C00);
	for (int j = 0; j < 32; j++) {
		for (int i = 0; i < 32; i++) {
			int indice = tilemap[j * 32 + i];
			video_render_tile(vram + 0x8000, indice, i + (x / 8),
					  j + (y / 8), scale);
		}
	}
}
