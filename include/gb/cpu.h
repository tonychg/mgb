#ifndef _CPU_H
#define _CPU_H

#include "types.h"
#include "list.h"
#include "memory.h"

#define CLOCK_PERIOD_NS 2384

#define FLAG_ZERO 0b10000000
#define FLAG_SUBS 0b01000000
#define FLAG_HALF 0b00100000
#define FLAG_CARRY 0b00010000
#define FLAG_NONE 0

enum execution_state {
	CPU_CORE_RUNNING,
	CPU_CORE_EXECUTE,
	CPU_CORE_INTERRUPT_DISPATCH,
	CPU_CORE_HALT,
};

enum interrupt {
	IR_VBLANK = 1 << 0,
	IR_LCD = 1 << 1,
	IR_TIMER = 1 << 2,
	IR_SERIAL = 1 << 3,
	IR_JOYPAD = 1 << 4,
};

struct instruction {
	u8 opcode;
	u8 length;
	const char *mnemonic;
	const char *op_1;
	const char *op_2;
	u8 cycles;
	bool prefixed;
};

struct cpu {
	u16 sp;
	u16 pc;
	u8 a;
	u8 f;
	u8 b;
	u8 c;
	u8 d;
	u8 e;
	u8 h;
	u8 l;

	struct memory *memory;

	u8 opcode;
	u8 cycles;
	u64 ticks;
	bool ime;
	u64 ime_cycles;
	struct list *irq;

	bool halted;
	bool debug;
	enum execution_state state;

	int multiplier;
};

#define MEM_READ_PC_S8(cpu) (cpu->pc + 1) + (s8)MEM_READ(cpu, cpu->pc)
#define IR_REQUESTED(cpu, ir) \
	((MEM_READ(cpu, IF) & ir) & (MEM_READ(cpu, IE) & ir))

// cpu.c
struct cpu *cpu_init(void);
void cpu_bind_memory(struct cpu *cpu, struct memory *memory);
void cpu_reset(struct cpu *cpu);
void cpu_sleep_ns(int nanoseconds);
void cpu_debug(struct cpu *cpu);
struct instruction cpu_prefetch(struct cpu *cpu);
void cpu_execute(struct cpu *cpu, struct instruction instruction);
void cpu_tick(struct cpu *cpu);
void cpu_cycle(struct cpu *cpu);
void cpu_release(struct cpu *cpu);
void cpu_pc_decrement(struct cpu *cpu);
void cpu_pc_increment(struct cpu *cpu);

void cpu_flag_set(struct cpu *cpu, int flag);
void cpu_flag_toggle(struct cpu *cpu, int flag);
void cpu_flag_untoggle(struct cpu *cpu, int flag);
void cpu_flag_clear(struct cpu *cpu);
bool cpu_flag_is_set(struct cpu *cpu, int flag);
void cpu_flag_flip(struct cpu *cpu, int flag);
void cpu_flag_set_or_clear(struct cpu *cpu, int flag);

u8 cpu_read_pc_addr(struct cpu *cpu);
void cpu_debug_instruction(struct cpu *cpu, struct instruction instruction);
u16 cpu_read_word(struct cpu *cpu);
u8 cpu_read_byte(struct cpu *cpu);

void cpu_enable_display(struct cpu *cpu);

// decoder.c
struct instruction cpu_op_decode(u8 opcode);
struct instruction cpu_op_decode_cb(u8 opcode);
char *cpu_opcode_to_string(u8 opcode);
char *cpu_opcode_cb_to_string(u8 opcode);

#endif
