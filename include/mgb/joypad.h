#include "mgb/mgb.h"
#include "platform/types.h"

enum joypad_button {
	BUTTON_A,
	BUTTON_B,
	BUTTON_SELECT,
	BUTTON_START,
	BUTTON_RIGHT,
	BUTTON_LEFT,
	BUTTON_UP,
	BUTTON_DOWN,
};

u8 update_joypad(struct gb_emulator *gb);
u8 read_keys(u8 keys, u8 joyp);
