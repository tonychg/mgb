#include <stdlib.h>
#include <stdio.h>
#include "register.h"

pair_register *register_create(void)
{
	pair_register *rg;

	if ((rg = (pair_register *)malloc(sizeof(pair_register))) == NULL) {
		return NULL;
	}
	rg->high = 0;
	rg->low = 0;
	return rg;
}

void register_release(pair_register *rg)
{
	free(rg);
	rg = NULL;
}

u16 register_get_value(pair_register *rg)
{
	return (u16)rg->high << 8 | rg->low;
}

void register_set_value(pair_register *rg, u16 value)
{
	rg->high = value >> 8;
	rg->low = 0xFF & value;
}

u8 register_get_low(pair_register *rg)
{
	return rg->low;
}

void register_set_low(pair_register *rg, u8 value)
{
	rg->low = value;
}

u8 register_get_high(pair_register *rg)
{
	return rg->high;
}

void register_set_high(pair_register *rg, u8 value)
{
	rg->high = value;
}

#ifdef TEST
#include "test.h"

void test_register()
{
	printf("# Testing register.c\n");
	pair_register *rg = register_create();
	register_set_low(rg, 0b10000000);
	register_set_high(rg, 0b00000001);
	assert(0b10000000 == register_get_low(rg));
	assert(0b00000001 == register_get_high(rg));
	assert(0b0000000110000000 == register_get_value(rg));
	register_set_value(rg, 0b1000000010000000);
	assert(0b10000000 == register_get_low(rg));
	assert(0b10000000 == register_get_high(rg));
	register_release(rg);
}
#endif
