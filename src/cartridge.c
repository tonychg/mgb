#include "gb/alloc.h"
#include "gb/fs.h"
#include "gb/cartridge.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static struct cartridge *cartridge_init(void)
{
	struct cartridge *cartridge;

	cartridge = (struct cartridge *)malloc(sizeof(struct cartridge));
	if (!cartridge)
		return NULL;
	cartridge->buffer = NULL;
	cartridge->rom_size = 0;
	cartridge->ram_size = 0;
	return cartridge;
}

static size_t cartridge_file_size(FILE *file)
{
	return fs_size(file);
}

static u8 *cartridge_read_file(FILE *file, size_t numbytes)
{
	return fs_read(file, numbytes);
}

static void cartridge_decode_title(struct cartridge *cartridge)
{
	memcpy(cartridge->title, cartridge->buffer + 0x134, sizeof(u8) * 16);
}

static u32 cartridge_ram_size(u8 code)
{
	return 32768 * (1 << code);
}

static u32 cartridge_rom_size(u8 code)
{
	return CARTRIDGE_RAM_SIZES[code];
}

struct cartridge *cartridge_load_from_file(char *path)
{
	struct cartridge *cartridge = cartridge_init();
	FILE *file = fopen(path, "r");
	size_t numbytes;

	if (file == NULL)
		return NULL;
	if (cartridge == NULL)
		return NULL;
	numbytes = cartridge_file_size(file);
	if ((cartridge->buffer = cartridge_read_file(file, numbytes)) == NULL)
		return NULL;
	cartridge_decode_title(cartridge);
	cartridge->size = numbytes;
	cartridge->cgb = cartridge->buffer[0x143];
	cartridge->sgb = cartridge->buffer[0x146];
	cartridge->type = cartridge->buffer[0x147];
	cartridge->rom_size = cartridge_rom_size(cartridge->buffer[0x148]);
	cartridge->ram_size = cartridge_ram_size(cartridge->buffer[0x149]);
	return cartridge;
}

void cartridge_metadata(struct cartridge *cartridge)
{
	printf("Title = %s\n", cartridge->title);
	printf("Size = %lu bytes\n", cartridge->size);
	printf("Type = 0x%02X\n", cartridge->type);
	printf("CGB = 0x%02X  SGB = 0x%02X\n", cartridge->cgb, cartridge->sgb);
	printf("ROM = %d  RAM = %d\n", cartridge->rom_size,
	       cartridge->ram_size);
}

void cartridge_release(struct cartridge *cartridge)
{
	zfree(cartridge->buffer);
	zfree(cartridge);
}
