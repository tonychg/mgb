#include "emu/joypad.h"
#include "emu/memory.h"

u8 read_keys(u8 keys, u8 joyp)
{
	switch (joyp & 0x30) {
	case 0x30:
		keys = 0;
		break;
	// D-Pad
	case 0x20:
		keys = keys >> 4;
		break;
	// Buttons
	case 0x10:
		break;
	case 0x00:
		keys |= keys >> 4;
		break;
	}
	return (0xCF | joyp) ^ (keys & 0xF);
}

u8 update_joypad(struct gb_emulator *gb)
{
	// https://gbdev.io/pandocs/Joypad_Input.html#ff00--p1joyp-joypad
	struct ppu *gpu = gb->gpu;
	u8 keys = gb->keys;
	u8 joypad = read_keys(gb->keys, gpu->memory->array->bytes[P1_JOYP]);
	gpu->memory->array->bytes[P1_JOYP] = (0xCF | joypad) ^ (keys & 0xF);
	if ((joypad & ~gpu->memory->array->bytes[P1_JOYP] & 0xF) != 0) {
		gpu->memory->array->bytes[IF] |= (1 << IRQ_JOYPAD);
	}
	// printf("Read joypad: %08b Keys: %08b\n",
	//        gpu->memory->array->bytes[P1_JOYP], keys);
	return gpu->memory->array->bytes[P1_JOYP];
}
