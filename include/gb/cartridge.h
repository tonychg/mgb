#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__

#include "types.h"
#include <stddef.h>

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

struct cartridge {
	u8 *buffer;
	char title[16];
	u8 cgb;
	u8 sgb;
	u8 type;
	size_t size;
	u32 rom_size;
	u32 ram_size;
};

struct cartridge *cartridge_init(void);
struct cartridge *cartridge_load_from_file(char *path);
void cartridge_metadata(struct cartridge *cartridge);
void cartridge_release(struct cartridge *cartridge);

#endif
