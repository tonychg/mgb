#include "emu/joypad.h"
#include "emu/memory.h"

void joypad_handler(struct gb_context *ctx)
{
	// https://gbdev.io/pandocs/Joypad_Input.html#ff00--p1joyp-joypad
	struct ppu *gpu = ctx->gb->gpu;
	u8 keys = ctx->keys;
	u8 joypad = load_u8(gpu->memory, P1_JOYP);
	switch (joypad & 0x30) {
	case 0x30: {
		keys = 0;
		break;
	}
	case 0x20: {
		keys >>= 4;
		break;
	}
	case 0x10: {
		keys |= keys >> 4;
		break;
	}
	case 0x00: {
		break;
        }
	}
	gpu->memory->array->bytes[P1_JOYP] = (0xCF | joypad) ^ (keys & 0xF);
	if (joypad & ~gpu->memory->array->bytes[P1_JOYP] & 0xF) {
		gpu->memory->array->bytes[IF] |= (1 << IRQ_JOYPAD);
	}
}
