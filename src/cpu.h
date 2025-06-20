#ifndef __CPU_H__
#define __CPU_H__

#include "types.h"
#include "register.h"
#include "memory.h"

enum execution_state {
	CPU_CORE_FETCH = 3,
	CPU_CORE_IDLE_0 = 0,
	CPU_CORE_IDLE_1 = 1,
	CPU_CORE_EXECUTE = 2,

	CPU_CORE_MEMORY_LOAD = 7,
	CPU_CORE_MEMORY_STORE = 11,
	CPU_CORE_READ_PC = 15,
	CPU_CORE_STALL = 19,
	CPU_CORE_OP2 = 23,
	CPU_CORE_HALT_BUG = 27,
};

typedef struct cpu {
	u16 sp;
	u16 pc;
	pair_register *af;
	pair_register *bc;
	pair_register *de;
	pair_register *hl;

	u16 instruction;
	u64 cycles;

	bool halted;
	enum execution_state state;

	int multiplier;
} cpu;

cpu *cpu_init(void);
void cpu_reset(cpu *cpu);
void cpu_tick(cpu *cpu);
void cpu_release(cpu *cpu);
void cpu_pc_decrement(cpu *cpu);
void cpu_pc_increment(cpu *cpu);
unsigned cpu_get_z(cpu *cpu);
unsigned cpu_get_n(cpu *cpu);
unsigned cpu_get_h(cpu *cpu);
unsigned cpu_get_c(cpu *cpu);
int cpu_instruction_length(u8 opcode);

#endif
