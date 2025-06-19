#ifndef __REGISTER_H__
#define __REGISTER_H__

#include "types.h"

typedef struct pair_register {
	u8 high;
	u8 low;
} pair_register;

pair_register *register_create(void);
void register_release(pair_register *rg);
u16 register_get_value(pair_register *rg);
void register_set_value(pair_register *rg, u16 value);
u8 register_get_low(pair_register *rg);
void register_set_low(pair_register *rg, u8 value);
u8 register_get_high(pair_register *rg);
void register_set_high(pair_register *rg, u8 value);

#endif
