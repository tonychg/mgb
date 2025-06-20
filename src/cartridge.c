#include "cartridge.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

cartridge *cartridge_init()
{
	cartridge *cartridge;

	if ((cartridge = (struct cartridge *)malloc(
		     sizeof(struct cartridge))) == NULL)
		return NULL;
	cartridge->buffer = NULL;
	cartridge->rom_size = 0;
	cartridge->ram_size = 0;
	return cartridge;
}

int cartridge_read_file(cartridge *cartridge, char *path)
{
	u8 *buffer;
	FILE *file = fopen(path, "r");
	size_t numbytes;

	fseek(file, 0L, SEEK_END);
	numbytes = ftell(file);
	fseek(file, 0L, SEEK_SET);
	if ((buffer = (u8 *)calloc(numbytes, sizeof(u8))) == NULL)
		return -1;
	if (fread(buffer, sizeof(u8), numbytes, file) == 0) {
		free(buffer);
		buffer = NULL;
		return -1;
	}
	fclose(file);
	cartridge->buffer = buffer;
	cartridge->size = numbytes;
	return 0;
}

void cartridge_decode_title(cartridge *cartridge)
{
	memcpy(cartridge->title, cartridge->buffer + 0x134, sizeof(u8) * 16);
}

cartridge *cartridge_load_from_file(char *path)
{
	cartridge *cartridge = cartridge_init();

	if (cartridge == NULL)
		return NULL;
	if (cartridge_read_file(cartridge, path) != 0)
		return NULL;
	cartridge_decode_title(cartridge);
	cartridge->cgb = cartridge->buffer[0x143];
	cartridge->sgb = cartridge->buffer[0x146];
	cartridge->type = cartridge->buffer[0x147];
	cartridge->rom_size = cartridge->buffer[0x148];
	cartridge->ram_size = cartridge->buffer[0x149];
	return cartridge;
}

void cartridge_metadata(cartridge *cartridge)
{
	printf("Title = %s\n", cartridge->title);
	printf("Size = %lu bytes\n", cartridge->size);
	printf("Type = 0x%02X\n", cartridge->type);
	printf("CGB = 0x%02X  SGB = 0x%02X\n", cartridge->cgb, cartridge->sgb);
	printf("ROM = 0x%02X  RAM = 0x%02X\n", cartridge->rom_size,
	       cartridge->ram_size);
}

void cartridge_release(cartridge *cartridge)
{
	if (cartridge->buffer != NULL)
		free(cartridge->buffer);
	cartridge->buffer = NULL;
	if (cartridge != NULL)
		free(cartridge);
	cartridge = NULL;
}
