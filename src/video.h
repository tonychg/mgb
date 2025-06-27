#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "types.h"
#include "memory.h"

enum {
	GB_HEIGHT = 160,
	GB_WIDTH = 144,
	GB_VIDEO_VBLANK_PIXELS = 10,
	GB_VIDEO_VERTICAL_TOTAL_PIXELS = 154,
	GB_VIDEO_TOTAL_LENGTH = 70224,
};

enum VideoMode {
	MODE_0,
	MODE_1,
	MODE_2,
	MODE_3,
};

typedef struct Video {
	u32 ly;
	u32 x;
	enum VideoMode mode;
	u8 stat;
	bool enabled;

	u8 frame_buffer[GB_HEIGHT * GB_WIDTH];
	Memory *memory;

	u8 frames;
	u32 dots;
} Video;

Video *video_init(void);
void video_release(Video *video);
void video_reset(Video *video);
void video_bind_memory(Video *video, Memory *memory);
void video_tick(Video *video);
void video_enable(Video *video);
void video_disable(Video *video);
void video_debug(Video *video);

#endif
