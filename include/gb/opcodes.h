#ifndef _OPCODE_H
#define _OPCODE_H

#include "cpu.h"

// op_helpers.c
u16 unsigned_16(u8 lsb, u8 msb);
u8 msb(u16 value);
u8 lsb(u16 value);

u8 opcode_get_high(u16 word);
u8 opcode_get_low(u16 word);

u16 opcode_r16_get(u8 *r1, u8 *r2);
void opcode_r16_set(u8 *r1, u8 *r2, u16 word);
void opcode_r16_inc(u8 *r1, u8 *r2);
void opcode_r16_dec(u8 *r1, u8 *r2);

void opcode_increment(struct cpu *cpu, u8 *reg);
void opcode_inc_hl(struct cpu *cpu);
void opcode_decrement(struct cpu *cpu, u8 *reg);
void opcode_dec_hl(struct cpu *cpu);

void opcode_rlc(struct cpu *cpu, u8 *reg, bool is_a);
void opcode_rlc_hl(struct cpu *cpu);
void opcode_rrc(struct cpu *cpu, u8 *reg, bool is_a);
void opcode_rrc_hl(struct cpu *cpu);
void opcode_rl(struct cpu *cpu, u8 *reg, bool is_a);
void opcode_rl_hl(struct cpu *cpu);
void opcode_rr(struct cpu *cpu, u8 *reg, bool is_a);
void opcode_rr_hl(struct cpu *cpu);
void opcode_rst(struct cpu *cpu, u8 vec);

void opcode_ld(u8 *reg, u8 byte);
void opcode_ld_a16(struct cpu *cpu, u8 *reg, u16 address);
void opcode_ldh_a_n(struct cpu *cpu);
void opcode_ld_r8_a16(struct cpu *cpu, u16 address, u8 byte);
void opcode_ld_nn(struct cpu *cpu, u16 *reg);
void opcode_ld_spn(struct cpu *cpu);

void opcode_add_hl(struct cpu *cpu, u16 word);
void opcode_add_sp(struct cpu *cpu, s8 value);
void opcode_add(struct cpu *cpu, u8 byte);
void opcode_adc(struct cpu *cpu, u8 byte);
void opcode_sub(struct cpu *cpu, u8 byte);
void opcode_sbc(struct cpu *cpu, u8 byte);
void opcode_swap(struct cpu *cpu, u8 *reg);
void opcode_swap_hl(struct cpu *cpu);
void opcode_srl(struct cpu *cpu, u8 *reg);
void opcode_srl_hl(struct cpu *cpu);
void opcode_sla(struct cpu *cpu, u8 *reg);
void opcode_sla_hl(struct cpu *cpu);
void opcode_sra(struct cpu *cpu, u8 *reg);
void opcode_sra_hl(struct cpu *cpu);

void opcode_and(struct cpu *cpu, u8 byte);
void opcode_xor(struct cpu *cpu, u8 byte);
void opcode_and(struct cpu *cpu, u8 byte);
void opcode_or(struct cpu *cpu, u8 byte);
void opcode_bit(struct cpu *cpu, u8 *reg, int bit);
void opcode_bit_hl(struct cpu *cpu, int bit);
void opcode_set(struct cpu *cpu, u8 *reg, int bit);
void opcode_set_hl(struct cpu *cpu, int bit);
void opcode_res(struct cpu *cpu, u8 *reg, int bit);
void opcode_res_hl(struct cpu *cpu, int bit);
void opcode_cp(struct cpu *cpu, u8 byte);

void opcode_call_nn(struct cpu *cpu);
void opcode_stack_push(struct cpu *cpu, u8 *r1, u8 *r2);
void opcode_stack_push_pc(struct cpu *cpu, u16 *pc);
void opcode_stack_pop(struct cpu *cpu, u8 *r1, u8 *r2);
void opcode_stack_pop_pc(struct cpu *cpu, u16 *pc);

void opcode_daa(struct cpu *cpu);

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
void opcode_execute(struct cpu *cpu, u8 opcode);
void opcode_execute_cb(struct cpu *cpu, u8 opcode);

#endif
