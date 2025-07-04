#include "video.h"
#include "alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>

Color convert_color(DmgPalette color)
{
	int hex_color = (color << 8) | 0xFF;
	return GetColor(hex_color | 0xFF);
}

Video *video_init(bool render)
{
	Video *video;

	if ((video = (Video *)malloc(sizeof(Video))) == NULL)
		return NULL;
	video->memory = NULL;
	video->render = render;
	if (video->render) {
		InitWindow(800, 600, "GB Emulator");
		SetTargetFPS(60);
	}
	return video;
}

void video_bind_memory(Video *video, Memory *memory)
{
	video->memory = memory;
}

void video_release(Video *video)
{
	if (video->render)
		CloseWindow();
	zfree(video);
}

void video_reset(Video *video)
{
	video->ly = 144;
	video->x = 0;
	video->enabled = false;
	video->stat = 0;
	video->dots = 0;
	video->frames = 0;
	for (int i = 0; i < (GB_HEIGHT * GB_WIDTH); i++)
		video->frame_buffer[i] = 0;
}

void video_memory_fetch(Video *video)
{
	u8 lcdc_lcd = MEM_READ(video, LCDC_LCD);

	if ((lcdc_lcd >> 7) == 1)
		video_enable(video);
	else
		video_disable(video);
	if (video->enabled)
		video->stat = MEM_READ(video, STAT_LCD);
}

void render_oam(Video *video)
{
}

void video_render(Video *video)
{
	char debug_text[256];

	sprintf(debug_text, "Frames: %d\n", video->frames);
	BeginDrawing();
	ClearBackground(convert_color(DMG_BLACK));
	DrawText(debug_text, 0, 0, 15, convert_color(DMG_WHITE));
	EndDrawing();
}

void video_tick(Video *video)
{
	video_memory_fetch(video);
	if (video->enabled) {
		if (video->dots != 0 && (video->dots % 456) == 0) {
			video->ly++;
			MEM_WRITE(video, LY_LCD, video->ly);
		}
		video->dots++;
		if (video->dots == GB_VIDEO_TOTAL_LENGTH) {
			video->frames++;
			video_reset(video);
			if (video->render) {
				video_render(video);
			}
		}
	}
}

void video_enable(Video *video)
{
	video->enabled = true;
}

void video_disable(Video *video)
{
	video->enabled = false;
}

void video_debug(Video *video)
{
	printf("  Mode = %8d | Enabled = %d\n", video->mode, video->enabled);
	printf("    ly = %8d | x = %d\n", video->ly, video->x);
	printf("LY_LCD = %8X | LYC_LY = %X\n", MEM_READ(video, LY_LCD),
	       MEM_READ(video, LYC_LY));
	printf("Frames = %8d | Dots = %d\n", video->frames, video->dots);
	printf("        Stat = %08b\n", video->stat);
}
