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

void joypad_handler(void *arg, enum joypad_button button);

