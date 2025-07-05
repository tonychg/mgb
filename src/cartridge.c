#include "cartridge.h"
#include "alloc.h"
#include "fs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Cartridge *cartridge_init()
{
	Cartridge *cartridge;

	if ((cartridge = (struct Cartridge *)malloc(
		     sizeof(struct Cartridge))) == NULL)
		return NULL;
	cartridge->buffer = NULL;
	cartridge->rom_size = 0;
	cartridge->ram_size = 0;
	return cartridge;
}

size_t cartridge_file_size(FILE *file)
{
	return fs_size(file);
}

u8 *cartridge_read_file(FILE *file, size_t numbytes)
{
	return fs_read(file, numbytes);
}

void cartridge_decode_title(Cartridge *cartridge)
{
	memcpy(cartridge->title, cartridge->buffer + 0x134, sizeof(u8) * 16);
}

u32 cartridge_ram_size(u8 code)
{
	return 32768 * (1 << code);
}

u32 cartridge_rom_size(u8 code)
{
	return CARTRIDGE_RAM_SIZES[code];
}

Cartridge *cartridge_load_from_file(char *path)
{
	Cartridge *cartridge = cartridge_init();
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

void cartridge_metadata(Cartridge *cartridge)
{
	printf("Title = %s\n", cartridge->title);
	printf("Size = %lu bytes\n", cartridge->size);
	printf("Type = 0x%02X\n", cartridge->type);
	printf("CGB = 0x%02X  SGB = 0x%02X\n", cartridge->cgb, cartridge->sgb);
	printf("ROM = %d  RAM = %d\n", cartridge->rom_size,
	       cartridge->ram_size);
}

void cartridge_release(Cartridge *cartridge)
{
	zfree(cartridge->buffer);
	zfree(cartridge);
}
