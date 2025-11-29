#ifndef _TIMER_H
#define _TIMER_H

#include "platform/types.h"
#include "emu/sm83.h"

enum div_register_freq {
	DIV_PERIOD = 16384,
	DIV_PERIOD_DOUBLE_SPEED = 32768,
};

static const u64 tima_periods[] = {
	256,
	4,
	16,
	64,
};

void sm83_update_timer_registers(struct sm83_core *cpu);

#endif
