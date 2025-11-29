#include "emu/ppu.h"
#include "emu/memory.h"
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
	gpu->ly = 144;
	gpu->x = 0;
	gpu->stat = 1;
	gpu->dots = 0;
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

static void draw_tile(struct ppu *gpu, u8 *vram, int tile_id, int x, int y)
{
	int j = 0;
	int indice = tile_id * 16;

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

static void draw_tiles(struct ppu *gpu, u8 *vram)
{
	for (int y = 0; y <= 23; y++) {
		for (int x = 0; x < 16; x++) {
			int tile_id = (y * 16) + x;
			draw_tile(gpu, vram, tile_id, x, y);
		}
	}
}

static void draw_tilemap(struct ppu *gpu, u8 *vram, u8 area, int x, int y)
{
	u8 *tilemap;
	tilemap = vram + (area == 0 ? 0x9800 : 0x9C00);
	for (int j = 0; j < 32; j++) {
		for (int i = 0; i < 32; i++) {
			int indice = tilemap[j * 32 + i];
			draw_tile(gpu, vram + 0x8000, indice, i + (x / 8), j + (y / 8));
		}
	}
}

void ppu_draw(struct ppu *gpu)
{
	draw_tiles(gpu, gpu->memory->array->bytes + 0x8000);
	draw_tilemap(gpu, gpu->memory->array->bytes, 0, 128, 0);
	draw_tilemap(gpu, gpu->memory->array->bytes, 1, 128, 256);
}

static void lcd_irq_ack(struct ppu *gpu)
{
	u8 lcdc_lcd = load_u8(gpu->memory, LCDC_LCD);
	gpu->enabled = ((lcdc_lcd >> 7) == 1);
	if (gpu->enabled)
		gpu->stat = load_u8(gpu->memory, STAT_LCD);
}

void ppu_tick(struct ppu *gpu)
{
	lcd_irq_ack(gpu);
	if (gpu->enabled) {
		if (gpu->dots != 0 && (gpu->dots % 456) == 0) {
			gpu->ly++;
			write_u8(gpu->memory, LY_LCD, gpu->ly);
		}
		gpu->dots++;
	}
	if (gpu->dots == GB_VIDEO_TOTAL_LENGTH) {
		gpu->frames++;
		ppu_reset(gpu);
	}
}
