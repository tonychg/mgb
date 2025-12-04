#include "mgb/sm83.h"
#include "platform/types.h"

enum {
	GB_WIDTH = 160,
	GB_HEIGHT = 144,
	GB_VIDEO_VBLANK_PIXELS = 10,
	GB_VIDEO_VERTICAL_TOTAL_PIXELS = 154,
	GB_VIDEO_FRAME_PERIOD = 70224,
	GB_VIDEO_SCANLINE_PERIOD = 456,
	GB_BG_MAP_WIDTH = 256,
	GB_BG_MAP_HEIGHT = 256,
	GB_TILE_SIZE = 8,
	GB_TILE_MEMORY_SIZE = 16,
	GB_TILEMAP_SIZE = 32
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

// https://gbdev.io/pandocs/LCDC.html#ff40--lcdc-lcd-control
enum lcd_control_flag {
	LCD_BG_WINDOW_ENABLE,
	LCD_OBJ_ENABLE,
	LCD_OBJ_SIZE,
	LCD_BG_TILEMAP_AREA,
	LCD_BG_WINDOW_TILE_AREA,
	LCD_WINDOW_ENABLE,
	LCD_WINDOW_TILEMAP_AREA,
	LCD_ENABLE,
};

// https://gbdev.io/pandocs/STAT.html#ff41--stat-lcd-status
enum lcd_status_flag {
	STAT_PPU_MODE_LOW, // read-only
	STAT_PPU_MODE_HIGH, // read-only
	STAT_LYC_LY, // read-only
	STAT_MODE_0,
	STAT_MODE_1,
	STAT_MODE_2,
	STAT_LYC_INT_SELECT,
};

enum oam_flag_attribute {
	OAM_PRIORITY = 7,
	OAM_Y_FLIP = 6,
	OAM_X_FLIP = 5,
	OAM_DMG_PALETTE = 4,
};

#define FLAG_ENABLE(byte, flag) (byte & (1 << flag)) != 0
#define FLAG_MEM_ENABLE(addr, flag) FLAG_ENABLE(gpu->ram.load(gpu, addr), flag)
#define LCD_CONTROL(flag) FLAG_MEM_ENABLE(LCDC_LCD, flag)
#define LCD_STATUS(flag) FLAG_MEM_ENABLE(STAT_LCD, flag)

static const int DMG_PALETTE[4] = {
	DMG_WHITE,
	DMG_LIGHTGRAY,
	DMG_DARKGRAY,
	DMG_BLACK,
};

enum vram_area {
	VRAM_AREA_WINDOW_TILEMAP,
	VRAM_AREA_BG_WINDOW_TILE,
	VRAM_AREA_BG_TILEMAP,
};

struct vram_area_range {
	u16 begin;
	u16 end;
};

// clang-format off
static const struct vram_area_range vram_areas[][2] = {
	{
		{ 0x9800, 0x9BFF },
		{ 0x9C00, 0x9FFF },
	},
	{
		{ 0x8800, 0x97FF },
		{ 0x8000, 0x8FFF },
	},
	{
		{ 0x9800, 0x9BFF },
		{ 0x9C00, 0x9FFF },
	},
};
// clang-format on

struct ppu;
struct ppu_memory;

struct render {
	// int (*create_window)(struct ppu *gpu, int width, int height, int scale);
	void (*draw)(struct ppu *gpu, int x, int y, int color);
};

struct ppu_memory {
	u8 (*load)(struct ppu *gpu, u16 addr);
	void (*write)(struct ppu *gpu, u16 addr, u8 value);
	u8 *(*offset)(struct ppu *gpu, u16 offset);
};

struct ppu {
	bool render;

	u8 ly;
	u32 x;
	int width;
	int height;

	u8 frame_buffer[GB_HEIGHT * GB_WIDTH];

	enum ppu_mode mode;

	u8 scale;
	u64 frames;
	u64 dots;
	struct render renderer;
	struct ppu_memory ram;
	void *parent;
};

void ppu_init(struct ppu *gpu);
void ppu_reset(struct ppu *gpu);
void draw_scanline(struct ppu *gpu);
void ppu_draw(struct ppu *gpu);
void ppu_tick(struct ppu *gpu, struct sm83_core *cpu);
void ppu_info(struct ppu *gpu);
