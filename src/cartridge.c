#include "cartridge.h"
#include "alloc.h"
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
	size_t numbytes;
	fseek(file, 0L, SEEK_END);
	numbytes = ftell(file);
	fseek(file, 0L, SEEK_SET);
	return numbytes;
}

u8 *cartridge_read_file(FILE *file, size_t numbytes)
{
	u8 *buffer;

	if ((buffer = (u8 *)calloc(numbytes, sizeof(u8))) == NULL)
		return NULL;
	if (fread(buffer, sizeof(u8), numbytes, file) == 0) {
		zfree(buffer);
		return NULL;
	}
	fclose(file);
	return buffer;
}

void cartridge_decode_title(Cartridge *cartridge)
{
	memcpy(cartridge->title, cartridge->buffer + 0x134, sizeof(u8) * 16);
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
	cartridge->rom_size = cartridge->buffer[0x148];
	cartridge->ram_size = cartridge->buffer[0x149];
	return cartridge;
}

void cartridge_metadata(Cartridge *cartridge)
{
	printf("Title = %s\n", cartridge->title);
	printf("Size = %lu bytes\n", cartridge->size);
	printf("Type = 0x%02X\n", cartridge->type);
	printf("CGB = 0x%02X  SGB = 0x%02X\n", cartridge->cgb, cartridge->sgb);
	printf("ROM = 0x%02X  RAM = 0x%02X\n", cartridge->rom_size,
	       cartridge->ram_size);
}

void cartridge_release(Cartridge *cartridge)
{
	zfree(cartridge->buffer);
	zfree(cartridge);
}
