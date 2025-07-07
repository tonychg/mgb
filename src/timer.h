#ifndef __TIMER_H__
#define __TIMER_H__

#include "types.h"
#include "cpu.h"
#include "memory.h"

#define TIME_ENABLED 1 << 3

static const int CLOCK_RATE[4] = {
	256,
	4,
	16,
	64,
};

typedef struct TimerControl {
	bool tima_enabled;
	u16 clock_rate;
} TimerControl;

void timer_debug(Cpu *cpu);
TimerControl timer_tac(Cpu *cpu);
void timer_reset_div(Cpu *cpu);

#endif
