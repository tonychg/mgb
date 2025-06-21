#ifndef __CPU_H__
#define __CPU_H__

#include "types.h"
#include "register.h"
#include "memory.h"
#include "cartridge.h"

#define CLOCK_PERIOD_NS 2384

enum ExecutionState {
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

typedef struct Instruction {
	u8 opcode;
	u8 length;
	const char *mnemonic;
	const char *op_1;
	const char *op_2;
	u8 cycles;
} Instruction;

typedef struct Cpu {
	u16 sp;
	u16 pc;
	PairRegister *af;
	PairRegister *bc;
	PairRegister *de;
	PairRegister *hl;

	Memory *memory;

	u16 instruction;
	u64 cycles;

	bool halted;
	enum ExecutionState state;

	int multiplier;
} Cpu;

Cpu *cpu_init(void);
void cpu_bind_memory(Cpu *cpu, Memory *memory);
void cpu_reset(Cpu *cpu);
void cpu_sleep_ns(int nanoseconds);
void cpu_debug(Cpu *cpu);
void cpu_tick(Cpu *cpu);
void cpu_release(Cpu *cpu);
void cpu_pc_decrement(Cpu *cpu);
void cpu_pc_increment(Cpu *cpu);
unsigned cpu_get_z(Cpu *cpu);
unsigned cpu_get_n(Cpu *cpu);
unsigned cpu_get_h(Cpu *cpu);
unsigned cpu_get_c(Cpu *cpu);
Instruction cpu_op_decode(u8 opcode);
void cpu_debug_instruction(Instruction instruction);
void cpu_execute(Cpu *cpu, Instruction instruction);

#endif
