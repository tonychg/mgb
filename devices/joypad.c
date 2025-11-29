#include "emu/joypad.h"
#include "emu/gb.h"

void joypad_handler(void *arg, enum joypad_button button)
{
	// https://gbdev.io/pandocs/Joypad_Input.html#ff00--p1joyp-joypad
	struct gb_context *ctx = (struct gb_context *)arg;
	struct ppu *gpu = ctx->gb->gpu;
	bool trigger_interrupt = false;
	u8 joypad = load_u8(gpu->memory, P1_JOYP);
	switch (joypad & 0x30) {
	case 0x30: {
		joypad |= 0xF;
		break;
	}
	case 0x20: {
		joypad ^= (1 << button);
		trigger_interrupt = true;
		break;
	}
	case 0x10: {
		joypad ^= (1 << (button - 4));
		trigger_interrupt = true;
		break;
	}
	}
	gpu->memory->array->bytes[P1_JOYP] = joypad;
	if (trigger_interrupt) {
		request_interrupt(gpu->memory, IRQ_JOYPAD);
	}
}
