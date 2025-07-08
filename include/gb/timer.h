#ifndef __TIMER_H__
#define __TIMER_H__

#include "types.h"
#include "cpu.h"

#define TIME_ENABLED 1 << 3

static const int CLOCK_RATE[4] = {
	256,
	4,
	16,
	64,
};

struct timer_control {
	bool tima_enabled;
	u16 clock_rate;
};

void timer_debug(struct cpu *cpu);
struct timer_control timer_tac(struct cpu *cpu);
void timer_reset_div(struct cpu *cpu);

#endif
