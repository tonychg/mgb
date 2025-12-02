#include "platform/mm.h"
#include "emu/joypad.h"
#include <criterion/criterion.h>
#include <criterion/new/assert.h>

struct joypad_test_case {
	u8 keys;
	u8 joyp;
	u8 result;
};

static const struct joypad_test_case tests[] = {
	{ .keys = 0b00001000, .joyp = 0b11010111, .result = 0b11010111 },
	{ .keys = 0b01100000, .joyp = 0b11111111, .result = 0b11111111 },
	{ .keys = 0b00000000, .joyp = 0b11111111, .result = 0b11111111 },
	{ .keys = 0b01000000, .joyp = 0b11101111, .result = 0b11101011 },
	{ .keys = 0b00001000, .joyp = 0b11011111, .result = 0b11010111 },
	{ .keys = 0b00000001, .joyp = 0b11011111, .result = 0b11011110 },
	{ .keys = 0b00001000, .joyp = 0b11001111, .result = 0b11000111 },
};

Test(asserts, native)
{
	for (int i = 0; i < ARRAY_SIZE(tests); i++)
		cr_assert(eq(u8, read_keys(tests[i].keys, tests[i].joyp), tests[i].result));
}
