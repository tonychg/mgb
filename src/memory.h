#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "cartridge.h"
#include "types.h"

typedef enum HardwareRegister {
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
} HardwareRegister;

typedef struct Memory {
	u8 *bus;
	u8 wram_bank;
	u8 vram_bank;
} Memory;

Memory *memory_init(void);
void memory_reset(Memory *memory);
u8 memory_read(Memory *memory, u16 addr);
void memory_write(Memory *memory, u16 addr, u8 byte);
void memory_write_word(Memory *memory, u16 addr, u16 word, bool big_endian);
void memory_bind_cartridge(Memory *memory, Cartridge *cartridge);
void memory_release(Memory *memory);
u8 memory_hardware_register(Memory *memory, HardwareRegister reg);
void memory_debug(Memory *memory, u16 start, u16 end);

#endif
