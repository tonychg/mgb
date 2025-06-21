#include "alloc.h"
#include <stdlib.h>
#include <stdio.h>
#include "register.h"

PairRegister *register_create(void)
{
	PairRegister *rg;

	if ((rg = (PairRegister *)malloc(sizeof(PairRegister))) == NULL) {
		return NULL;
	}
	rg->high = 0;
	rg->low = 0;
	return rg;
}

void register_release(PairRegister *rg)
{
	zfree(rg);
}

u16 register_get_value(PairRegister *rg)
{
	return (u16)rg->high << 8 | rg->low;
}

void register_set_value(PairRegister *rg, u16 value)
{
	rg->high = value >> 8;
	rg->low = 0xFF & value;
}

u8 register_get_low(PairRegister *rg)
{
	return rg->low;
}

void register_set_low(PairRegister *rg, u8 value)
{
	rg->low = value;
}

u8 register_get_high(PairRegister *rg)
{
	return rg->high;
}

void register_set_high(PairRegister *rg, u8 value)
{
	rg->high = value;
}

#ifdef TEST
#include "tests.h"

void test_register()
{
	printf("# Testing register.c\n");
	PairRegister *rg = register_create();
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
