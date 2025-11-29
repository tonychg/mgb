#ifndef _MEMORY_H
#define _MEMORY_H

#include "platform/mm.h"
#include "platform/types.h"

#define MEMORY_SIZE 0x10000

enum hardware_register {
	P1_JOYP = 0xFF00,
	SB = 0xFF01,
	SC = 0xFF02,
	DIV = 0xFF04,
	TIMA = 0xFF05,
	TMA = 0xFF06,
	TAC = 0xFF07,
	IF = 0xFF0F,
	NR10 = 0xFF10,
	NR11 = 0xFF11,
	NR12 = 0xFF12,
	NR13 = 0xFF13,
	NR14 = 0xFF14,
	NR21 = 0xFF16,
	NR22 = 0xFF17,
	NR23 = 0xFF18,
	NR24 = 0xFF19,
	NR30 = 0xFF1A,
	NR31 = 0xFF1B,
	NR32 = 0xFF1C,
	NR33 = 0xFF1D,
	NR34 = 0xFF1E,
	NR41 = 0xFF20,
	NR42 = 0xFF21,
	NR43 = 0xFF22,
	NR44 = 0xFF23,
	NR50 = 0xFF24,
	NR51 = 0xFF25,
	NR52 = 0xFF26,
	// WAVE_RAM       = 0xFF30,
	LCDC_LCD = 0xFF40,
	STAT_LCD = 0xFF41,
	SCY = 0xFF42,
	SCX = 0xFF43,
	LY_LCD = 0xFF44,
	LYC_LY = 0xFF45,
	DMA_OAM_DMA = 0xFF46,
	BGP_BG = 0xFF47,
	OBP0_OBJ = 0xFF48,
	OBP1_OBJ = 0xFF49,
	WY = 0xFF4A,
	WX = 0xFF4B,
	KEY1 = 0xFF4D,
	VBK_VRAM = 0xFF4F,
	HDMA1_VRAM_DMA = 0xFF51,
	HDMA2_VRAM_DMA = 0xFF52,
	HDMA3_VRAM_DMA = 0xFF53,
	HDMA4_VRAM_DMA = 0xFF54,
	HDMA5_VRAM_DMA = 0xFF55,
	RP = 0xFF56,
	BCPS_BGPI = 0xFF68,
	BCPD_BGPD = 0xFF69,
	OCPS_OBPI = 0xFF6A,
	OCPD_OBPD = 0xFF6B,
	OPRI = 0xFF6C,
	SVBK = 0xFF70,
	PCM12 = 0xFF76,
	PCM34 = 0xFF77,
	IE = 0xFFFF,
};

struct shared {
	struct byte_array *array;
};

struct shared *allocate_memory();
void destroy_memory(struct shared *memory);
u8 load_u8(struct shared *memory, u16 addr);
void write_u8(struct shared *memory, u16 addr, u8 value);
int load_rom(struct shared *memory, char *path);
void dump_memory(struct shared *memory);
void print_addr(struct shared *memory, u16 addr);

enum cartridge_type {
	ROM_ONLY = 0x00,
	MBC1,
	MBC1_RAM,
	MBC1_RAM_BATTERY,
	MBC2,
	MBC2_BATTERY,
	ROM_RAM_9,
	ROM_RAM_BATTERY_9,
	MMM01,
	MMM01_RAM,
	MMM01_RAM_BATTERY,
	MBC3_TIMER_BATTERY,
	MBC3_TIMER_RAM_BATTERY_10,
	MBC3,
	MBC3_RAM_10,
	MBC3_RAM_BATTERY_10,
	MBC5,
	MBC5_RAM,
	MBC5_RAM_BATTERY,
	MBC5_RUMBLE,
	MBC5_RUMBLE_RAM,
	MBC5_RUMBLE_RAM_BATTERY,
	MBC6,
	MBC7_SENSOR_RUMBLE_RAM_BATTERY,
	POCKET_CAMERA,
	BANDAI_TAMA5,
	HUC3,
	HUC1_RAM_BATTERY,
};

static const u32 CARTRIDGE_RAM_SIZES[6] = {
	0, // No RAM
	0, // Unused
	8192, // 1 Bank
	32768, // 4 Banks of 8KiB each
	131072, // 16 Banks of 8KiB each
	65536 // 8 Banks of 8KiB each
};

#endif
