#include "platform/types.h"

enum {
	GB_HEIGHT = 160,
	GB_WIDTH = 144,
	GB_VIDEO_VBLANK_PIXELS = 10,
	GB_VIDEO_VERTICAL_TOTAL_PIXELS = 154,
	GB_VIDEO_TOTAL_LENGTH = 70224,
};

enum ppu_mode {
	MODE_0,
	MODE_1,
	MODE_2,
	MODE_3,
};

enum dmg_colors {
	DMG_WHITE = 0x9A9E3F,
	DMG_LIGHTGRAY = 0x496B22,
	DMG_DARKGRAY = 0x0E450B,
	DMG_BLACK = 0x1B2A09,
};

static const int DMG_PALETTE[4] = {
	DMG_WHITE,
	DMG_LIGHTGRAY,
	DMG_DARKGRAY,
	DMG_BLACK,
};

struct ppu;

struct render {
	// int (*create_window)(struct ppu *gpu, int width, int height, int scale);
	void (*draw)(struct ppu *gpu, int x, int y, int color);
};

struct ppu {
	u8 ly;
	u32 x;
	u8 stat;
	bool enabled;
	bool render;
	int width;
	int height;

	u8 frame_buffer[GB_HEIGHT * GB_WIDTH];
	struct shared *memory;
	enum ppu_mode mode;

	u8 scale;
	u8 frames;
	u32 dots;
	struct render *renderer;
};

struct ppu *ppu_init(void);
void ppu_destroy(struct ppu *gpu);
void ppu_reset(struct ppu *gpu);
void draw_scanline(struct ppu *gpu);
void ppu_draw(struct ppu *gpu);
void ppu_tick(struct ppu *gpu);
