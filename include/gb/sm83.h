#ifndef _SM83_H
#define _SM83_H

#include "gb/types.h"

struct sm83_core;

struct sm83_memory {
	u8 (*load8)(struct sm83_core *, u16 addr);
	u16 (*load16)(struct sm83_core *, u16 addr);
	void (*write8)(struct sm83_core *, u16 addr, u8 value);
	s8 (*loads8)(struct sm83_core *);
	u8 (*read_segment)(struct sm83_core *);
};

struct sm83_instruction {
	u8 opcode;
	const char *mnemonic;
	const char *op1;
	const char *op2;
	u16 length;
	u16 cycles;
	bool prefixed;
};

enum sm83_state {
	SM83_CORE_FETCH,
	SM83_CORE_PC,
	SM83_CORE_READ,
	SM83_CORE_WRITE_0,
	SM83_CORE_WRITE_1,
	SM83_CORE_HALT,
	SM83_CORE_IDLE,
};

enum sm83_flag_register {
	FLAG_NONE = 0,
	FLAG_Z = 1 << 7,
	FLAG_N = 1 << 6,
	FLAG_H = 1 << 5,
	FLAG_C = 1 << 4,
};

enum sm83_irq {
	IRQ_VBLANK,
	IRQ_LCD,
	IRQ_TIMER,
	IRQ_SERIAL,
	IRQ_JOYPAD,
};

enum sm83_irq_vector {
	VEC_VBLANK = 0x40,
	VEC_LCD = 0x48,
	VEC_TIMER = 0x50,
	VEC_SERIAL = 0x50,
	VEC_JOYPAD = 0x60,
};

struct sm83_core {
	u8 a;
	u8 f;
	u8 b;
	u8 c;
	u8 d;
	u8 e;
	u8 h;
	u8 l;
	u16 pc;
	u16 sp;

	u64 cycles;
	u64 ime_cycles;

	u8 bus;
	u16 index;
	u16 ptr;
	u16 acc;
	struct sm83_memory *memory;
	struct sm83_instruction instruction;

	bool ime;
	bool halted;
	enum sm83_state state;

	void *parent;

	u8 multiplier;
};

/* sm83.c */
struct sm83_core *sm83_init(void);
struct sm83_instruction sm83_cpu_fetch(struct sm83_core *cpu);
void sm83_cpu_step(struct sm83_core *cpu);
void sm83_cpu_execute(struct sm83_core *cpu);
void sm83_cpu_reset(struct sm83_core *cpu);
void sm83_cpu_plug_memory(struct sm83_core *cpu, struct sm83_memory *bus);
void sm83_destroy(struct sm83_core *cpu);

/* sm83_isa.c */
void sm83_isa_execute(struct sm83_core *cpu, u8 opcode);
void sm83_isa_cb_execute(struct sm83_core *cpu, u8 opcode);

/* decoder.c */
struct sm83_instruction cpu_decode(struct sm83_core *cpu);

/* debugger.c */
char *sm83_disassemble(struct sm83_core *cpu);
void sm83_cpu_debug(struct sm83_core *cpu);
void sm83_debugger_start(u8 *rom);

static inline u8 msb(u16 value)
{
	return value >> 8;
}

static inline u8 lsb(u16 value)
{
	return 0xFF & value;
}

static inline u16 unsigned_16(u8 lsb, u8 msb)
{
	return (u16)msb << 8 | lsb;
}

static inline void op_r16_set(u8 *r1, u8 *r2, u16 word)
{
	*r1 = msb(word);
	*r2 = lsb(word);
}

#define AF(cpu) unsigned_16(cpu->f, cpu->a)
#define BC(cpu) unsigned_16(cpu->c, cpu->b)
#define DE(cpu) unsigned_16(cpu->e, cpu->d)
#define HL(cpu) unsigned_16(cpu->l, cpu->h)

#define SET_AF(cpu, value) op_r16_set(&cpu->a, &cpu->f, value)
#define SET_BC(cpu, value) op_r16_set(&cpu->b, &cpu->c, value)
#define SET_DE(cpu, value) op_r16_set(&cpu->d, &cpu->e, value)
#define SET_HL(cpu, value) op_r16_set(&cpu->h, &cpu->l, value)

static inline void cpu_flag_set(struct sm83_core *cpu, int flag)
{
	cpu->f = flag;
}

static inline void cpu_flag_toggle(struct sm83_core *cpu, int flag)
{
	cpu->f |= flag;
}

static inline void cpu_flag_untoggle(struct sm83_core *cpu, int flag)
{
	cpu->f &= (~flag);
}

static inline void cpu_flag_clear(struct sm83_core *cpu)
{
	cpu_flag_set(cpu, FLAG_NONE);
}

static inline bool cpu_flag_is_set(struct sm83_core *cpu, int flag)
{
	return (cpu->f & flag) != 0;
}

static inline void cpu_flag_flip(struct sm83_core *cpu, int flag)
{
	cpu->f ^= flag;
}

static inline void cpu_flag_set_or_clear(struct sm83_core *cpu, int flag)
{
	if (cpu_flag_is_set(cpu, flag))
		cpu_flag_set(cpu, flag);
	else
		cpu_flag_clear(cpu);
}

#endif
