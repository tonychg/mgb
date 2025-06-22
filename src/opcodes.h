#ifndef __OPCODE_H__
#define __OPCODE_H__

#include "cpu.h"

// instructions.c
u8 opcode_get_high(u16 word);
u8 opcode_get_low(u16 word);
u16 opcode_r16_get(u8 *r1, u8 *r2);
void opcode_r16_set(u8 *r1, u8 *r2, u16 word);
void opcode_r16_inc(u8 *r1, u8 *r2);
void opcode_r16_dec(u8 *r1, u8 *r2);
void opcode_increment(Cpu *cpu, u8 *reg);
void opcode_decrement(Cpu *cpu, u8 *reg);
void opcode_rlc(Cpu *cpu, u8 *reg, bool is_a);
void opcode_rrc(Cpu *cpu, u8 *reg, bool is_a);
void opcode_rl(Cpu *cpu, u8 *reg, bool is_a);
void opcode_rr(Cpu *cpu, u8 *reg, bool is_a);
void opcode_ld(u8 *reg, u8 byte);
void opcode_ld_a16(Cpu *cpu, u8 *reg, u16 address);
void opcode_ld_nn(Cpu *cpu, u16 *reg);
void opcode_add_hl(Cpu *cpu, u16 word);
void opcode_daa(Cpu *cpu);

#define AF(cpu) opcode_r16_get(&cpu->a, &cpu->f)
#define BC(cpu) opcode_r16_get(&cpu->b, &cpu->c)
#define DE(cpu) opcode_r16_get(&cpu->d, &cpu->e)
#define HL(cpu) opcode_r16_get(&cpu->h, &cpu->l)

#define SET_AF(cpu, word) opcode_r16_set(&cpu->a, &cpu->f, word)
#define SET_BC(cpu, word) opcode_r16_set(&cpu->b, &cpu->c, word)
#define SET_DE(cpu, word) opcode_r16_set(&cpu->d, &cpu->e, word)
#define SET_HL(cpu, word) opcode_r16_set(&cpu->h, &cpu->l, word)

#define INC_AF(cpu) opcode_r16_inc(&cpu->a, &cpu->f)
#define INC_BC(cpu) opcode_r16_inc(&cpu->b, &cpu->c)
#define INC_DE(cpu) opcode_r16_inc(&cpu->d, &cpu->e)
#define INC_HL(cpu) opcode_r16_inc(&cpu->h, &cpu->l)

#define DEC_AF(cpu) opcode_r16_dec(&cpu->a, &cpu->f)
#define DEC_BC(cpu) opcode_r16_dec(&cpu->b, &cpu->c)
#define DEC_DE(cpu) opcode_r16_dec(&cpu->d, &cpu->e)
#define DEC_HL(cpu) opcode_r16_dec(&cpu->h, &cpu->l)

// opcodes.c
void opcode_execute(Cpu *cpu, Instruction instruction);
void opcode_execute_cb(Cpu *cpu, Instruction instruction);

#endif
