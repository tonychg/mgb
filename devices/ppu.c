#include "emu/ppu.h"
#include "emu/memory.h"
#include "emu/sm83.h"
#include "platform/mm.h"
#include <stdlib.h>

static struct ppu *allocate_ppu()
{
	struct ppu *gpu;
	gpu = (struct ppu *)malloc(sizeof(struct ppu));
	if (!gpu)
		return NULL;
	gpu->renderer = (struct render *)malloc(sizeof(struct render));
	if (!gpu->renderer)
		return NULL;
	return gpu;
}

struct ppu *ppu_init(void)
{
	struct ppu *gpu = allocate_ppu();
	if (!gpu)
		return NULL;
	gpu->frames = 0;
	gpu->dots = 0;
	ppu_reset(gpu);
	return gpu;
}

void ppu_destroy(struct ppu *gpu)
{
	zfree(gpu->renderer);
	zfree(gpu);
}

void ppu_reset(struct ppu *gpu)
{
	gpu->ly = 0;
	gpu->x = 0;
	gpu->mode = MODE_1;
}

static u8 encode_pixel_color(u8 right, u8 left, u8 bit)
{
	u8 lsb, msb;
	u8 pixel;
	u8 mask = 1 << bit;
	msb = ((left & mask) >> bit) << 1;
	lsb = ((right & mask) >> bit);
	pixel = msb | lsb;
	return pixel;
}

static void draw_tile(struct ppu *gpu, u8 *vram, int tile_index, int x, int y)
{
	int j = 0;
	int indice = tile_index * 16;

	for (int b = 0; b < 16; b = b + 2) {
		int i = 0;
		u8 right = vram[indice + b];
		u8 left = vram[indice + b + 1];
		for (int bit = 7; bit >= 0; bit--) {
			u8 color = encode_pixel_color(right, left, bit);
			gpu->renderer->draw(gpu, x * 8 + i, y * 8 + j,
					    DMG_PALETTE[color]);
			i++;
		}
		j++;
	}
}

static void draw_tiledata(struct ppu *gpu, u8 *vram)
{
	for (int j = 0; j <= 23; j++) {
		for (int i = 0; i < 16; i++) {
			int tile_id = (j * 16) + i;
			draw_tile(gpu, vram, tile_id, i, j);
		}
	}
}

static void draw_oam(struct ppu *gpu, int x, int y)
{
	// u8 obj_mode = LCD_CONTROL(LCD_OBJ_SIZE);

	for (int i = 0xFE00; i <= 0xFE9F; i = i + 4) {
		u8 oam_y = gpu->memory->array->bytes[i];
		u8 oam_x = gpu->memory->array->bytes[i + 1];
		u8 oam_tile_index = gpu->memory->array->bytes[i + 2];
		u8 oam_flags = gpu->memory->array->bytes[i + 3];
		// bool is_y_flip = FLAG_ENABLE(oam_flags, OAM_Y_FLIP);
		// bool is_x_flip = FLAG_ENABLE(oam_flags, OAM_X_FLIP);
		draw_tile(gpu, gpu->memory->array->bytes + 0x8000,
			  oam_tile_index, oam_x + x, oam_y + y);
	}
}

static void draw_viewport(struct ppu *gpu, int x, int y)
{
	draw_oam(gpu, x, y);
	for (int j = 0; j < GB_HEIGHT; j++) {
		for (int i = 0; i < GB_WIDTH; i++) {
			u8 color = gpu->frame_buffer[j * GB_WIDTH + i];
			gpu->renderer->draw(gpu, x + i, y + j,
					    DMG_PALETTE[color]);
		}
	}
}

static struct vram_area_range current_vram_area_range(struct ppu *gpu,
						      enum vram_area area)
{
	struct vram_area_range range;
	switch (area) {
	case VRAM_AREA_WINDOW_TILEMAP:
		range = vram_areas[VRAM_AREA_WINDOW_TILEMAP]
				  [LCD_CONTROL(LCD_BG_TILEMAP_AREA)];
		break;
	case VRAM_AREA_BG_WINDOW_TILE:
		range = vram_areas[VRAM_AREA_BG_WINDOW_TILE]
				  [LCD_CONTROL(LCD_BG_WINDOW_TILE_AREA)];
		break;
	case VRAM_AREA_BG_TILEMAP:
		range = vram_areas[VRAM_AREA_BG_TILEMAP]
				  [LCD_CONTROL(LCD_BG_TILEMAP_AREA)];
		break;
	}
	return range;
}

static void update_lyc_ly(struct ppu *gpu)
{
	u8 lcd_status = load_u8(gpu->memory, STAT_LCD);
	u8 lyc = load_u8(gpu->memory, LYC_LY);
	if (lyc == gpu->ly)
		lcd_status |= 1 << STAT_LYC_LY;
	else
		lcd_status ^= 1 << STAT_LYC_LY;
}

static void request_vlank_interrupt(struct ppu *gpu)
{
	if (gpu->ly == 144)
		request_interrupt(gpu->memory, IRQ_VBLANK);
}

static void request_stat_interrupt(struct ppu *gpu)
{
	if (LCD_STATUS(STAT_LYC_INT_SELECT) && LCD_STATUS(STAT_LYC_LY)) {
		request_interrupt(gpu->memory, IRQ_LCD);
	}
}

static void draw_window(struct ppu *gpu, int x, int y)
{
	struct vram_area_range win_area =
		current_vram_area_range(gpu, VRAM_AREA_WINDOW_TILEMAP);
	u8 *tiledata = gpu->memory->array->bytes;
	u8 *tilemap = gpu->memory->array->bytes + win_area.begin;
	for (int j = 0; j < 32; j++) {
		for (int i = 0; i < 32; i++) {
			int tile_index = tilemap[j * 32 + i];
			u16 begin = 0x8000;
			// Simulate signed indexing
			// https://gbdev.io/pandocs/Tile_Data.html#vram-tile-data
			if (!(LCD_CONTROL(LCD_BG_WINDOW_TILE_AREA))) {
				if (tile_index > 127)
					tile_index -= 128;
				else
					begin = 0x9000;
			}
			draw_tile(gpu, tiledata + begin, tile_index,
				  i + (x / 8), j + (y / 8));
		}
	}
}

static void draw_background(struct ppu *gpu, int x, int y)
{
	struct vram_area_range bg_area =
		current_vram_area_range(gpu, VRAM_AREA_BG_TILEMAP);
	u8 *tiledata = gpu->memory->array->bytes;
	u8 *tilemap = gpu->memory->array->bytes + bg_area.begin;
	for (int j = 0; j < 32; j++) {
		for (int i = 0; i < 32; i++) {
			int tile_index = tilemap[j * 32 + i];
			u16 begin = 0x8000;
			// Simulate signed indexing
			// https://gbdev.io/pandocs/Tile_Data.html#vram-tile-data
			if (!(LCD_CONTROL(LCD_BG_WINDOW_TILE_AREA))) {
				if (tile_index > 127)
					tile_index -= 128;
				else
					begin = 0x9000;
			}
			draw_tile(gpu, tiledata + begin, tile_index,
				  i + (x / 8), j + (y / 8));
		}
	}
}

void ppu_draw(struct ppu *gpu)
{
	if (LCD_CONTROL(LCD_ENABLE))
		draw_viewport(gpu, 0, 8 * 23);
	draw_tiledata(gpu, gpu->memory->array->bytes + 0x8000);
	draw_window(gpu, GB_WIDTH, 0);
	draw_background(gpu, GB_WIDTH, 256);
}

static void increment_scanline(struct ppu *gpu)
{
	if (gpu->ly == 153) {
		gpu->frames++;
		ppu_reset(gpu);
	}
	if (gpu->dots >= GB_VIDEO_SCANLINE_PERIOD) {
		gpu->dots -= GB_VIDEO_SCANLINE_PERIOD;
		u8 ly = load_u8(gpu->memory, LY_LCD);
		gpu->ly++;
		if (gpu->ly == 144 && ly != gpu->ly) {
			request_vlank_interrupt(gpu);
		}
		gpu->memory->array->bytes[LY_LCD] = gpu->ly;
		update_lyc_ly(gpu);
		request_stat_interrupt(gpu);
	}
}

void ppu_tick(struct ppu *gpu, struct sm83_core *cpu)
{
	if (LCD_CONTROL(LCD_ENABLE)) {
		gpu->dots += cpu->multiplier;
		increment_scanline(gpu);
	}
}
