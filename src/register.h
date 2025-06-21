#ifndef __REGISTER_H__
#define __REGISTER_H__

#include "types.h"

typedef struct PairRegister {
	u8 high;
	u8 low;
} PairRegister;

PairRegister *register_create(void);
void register_release(PairRegister *rg);
u16 register_get_value(PairRegister *rg);
void register_set_value(PairRegister *rg, u16 value);
u8 register_get_low(PairRegister *rg);
void register_set_low(PairRegister *rg, u8 value);
u8 register_get_high(PairRegister *rg);
void register_set_high(PairRegister *rg, u8 value);

#endif
