#ifndef __CPU_H__
#define __CPU_H__

#include "types.h"
#include "list.h"
#include "memory.h"
#include "cartridge.h"

#define CLOCK_PERIOD_NS 2384

#define FLAG_ZERO 0b10000000
#define FLAG_SUBS 0b01000000
#define FLAG_HALF 0b00100000
#define FLAG_CARRY 0b00010000
#define FLAG_NONE 0

enum ExecutionState {
	CPU_CORE_RUNNING,
	CPU_CORE_EXECUTE,
	CPU_CORE_INTERRUPT_DISPATCH,
	CPU_CORE_HALT,
};

enum Interrupt {
	IR_VBLANK = 1 << 0,
	IR_LCD = 1 << 1,
	IR_TIMER = 1 << 2,
	IR_SERIAL = 1 << 3,
	IR_JOYPAD = 1 << 4,
};

typedef struct Instruction {
	u8 opcode;
	u8 length;
	const char *mnemonic;
	const char *op_1;
	const char *op_2;
	u8 cycles;
	bool prefixed;
} Instruction;

typedef struct Cpu {
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

	Memory *memory;

	u8 opcode;
	u8 cycles;
	bool ime;
	u64 ime_cycles;
	List *irq;

	bool halted;
	bool debug;
	enum ExecutionState state;

	int multiplier;
} Cpu;

#define MEM_READ(cpu, addr) memory_read(cpu->memory, addr)
#define MEM_WRITE(cpu, addr, byte) memory_write(cpu->memory, addr, byte)
#define MEM_WRITE_BE(cpu, addr, word) \
	memory_write_word(cpu->memory, addr, word, true)
#define MEM_WRITE_LE(cpu, addr, word) \
	memory_write_word(cpu->memory, addr, word, false)
#define MEM_READ_PC_S8(cpu) (cpu->pc + 1) + (s8)MEM_READ(cpu, cpu->pc)
#define IR_REQUESTED(cpu, ir) \
	((MEM_READ(cpu, IF) & ir) & (MEM_READ(cpu, IE) & ir))

// cpu.c
Cpu *cpu_init(void);
void cpu_bind_memory(Cpu *cpu, Memory *memory);
void cpu_reset(Cpu *cpu);
void cpu_sleep_ns(int nanoseconds);
void cpu_debug(Cpu *cpu);
Instruction cpu_prefetch(Cpu *cpu);
void cpu_execute(Cpu *cpu, Instruction instruction);
void cpu_tick(Cpu *cpu);
void cpu_cycle(Cpu *cpu);
void cpu_goto(Cpu *cpu, u16 address);
void cpu_release(Cpu *cpu);
void cpu_pc_decrement(Cpu *cpu);
void cpu_pc_increment(Cpu *cpu);

void cpu_flag_set(Cpu *cpu, int flag);
void cpu_flag_toggle(Cpu *cpu, int flag);
void cpu_flag_untoggle(Cpu *cpu, int flag);
void cpu_flag_clear(Cpu *cpu);
bool cpu_flag_is_set(Cpu *cpu, int flag);
void cpu_flag_flip(Cpu *cpu, int flag);
void cpu_flag_set_or_clear(Cpu *cpu, int flag);

u8 cpu_read_pc_addr(Cpu *cpu);
void cpu_debug_instruction(Cpu *cpu, Instruction instruction);
u16 cpu_read_word(Cpu *cpu);
u8 cpu_read_byte(Cpu *cpu);

void cpu_enable_display(Cpu *cpu);

// decoder.c
Instruction cpu_op_decode(u8 opcode);
Instruction cpu_op_decode_cb(u8 opcode);
char *cpu_opcode_to_string(u8 opcode);
char *cpu_opcode_cb_to_string(u8 opcode);

#endif
