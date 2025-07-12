#ifndef _VIDEO_H
#define _VIDEO_H

#include "types.h"
#include "memory.h"

enum {
	GB_HEIGHT = 160,
	GB_WIDTH = 144,
	GB_VIDEO_VBLANK_PIXELS = 10,
	GB_VIDEO_VERTICAL_TOTAL_PIXELS = 154,
	GB_VIDEO_TOTAL_LENGTH = 70224,
};

enum video_mode {
	MODE_0,
	MODE_1,
	MODE_2,
	MODE_3,
};

struct video {
	u8 ly;
	u32 x;
	enum video_mode mode;
	u8 stat;
	bool enabled;
	bool render;

	u8 frame_buffer[GB_HEIGHT * GB_WIDTH];
	struct memory *memory;

	u8 scale;
	u8 frames;
	u32 dots;
};

struct video *video_init(bool render);
void video_release(struct video *video);
void video_reset(struct video *video);
void video_bind_memory(struct video *video, struct memory *memory);
void video_tick(struct video *video);
void video_debug(struct video *video);
void video_render(struct video *video);
void video_render_tiles(u8 *vram, int scale);
void video_render_frame(struct video *video);
void video_render_tilemap(u8 *vram, u8 area, int x, int y, int scale);
u8 video_pixel_color(u8 right, u8 left, u8 bit);

#endif
