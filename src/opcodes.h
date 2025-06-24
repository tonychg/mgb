#ifndef __OPCODE_H__
#define __OPCODE_H__

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

void opcode_increment(Cpu *cpu, u8 *reg);
void opcode_inc_hl(Cpu *cpu);
void opcode_decrement(Cpu *cpu, u8 *reg);
void opcode_dec_hl(Cpu *cpu);

void opcode_rlc(Cpu *cpu, u8 *reg, bool is_a);
void opcode_rlc_hl(Cpu *cpu);
void opcode_rrc(Cpu *cpu, u8 *reg, bool is_a);
void opcode_rrc_hl(Cpu *cpu);
void opcode_rl(Cpu *cpu, u8 *reg, bool is_a);
void opcode_rl_hl(Cpu *cpu);
void opcode_rr(Cpu *cpu, u8 *reg, bool is_a);
void opcode_rr_hl(Cpu *cpu);
void opcode_rst(Cpu *cpu, u8 vec);

void opcode_ld(u8 *reg, u8 byte);
void opcode_ld_a16(Cpu *cpu, u8 *reg, u16 address);
void opcode_ld_r8_a16(Cpu *cpu, u16 address, u8 byte);
void opcode_ld_nn(Cpu *cpu, u16 *reg);
void opcode_ld_spn(Cpu *cpu);

void opcode_add_hl(Cpu *cpu, u16 word);
void opcode_add_sp(Cpu *cpu, s8 value);
void opcode_add(Cpu *cpu, u8 byte);
void opcode_adc(Cpu *cpu, u8 byte);
void opcode_sub(Cpu *cpu, u8 byte);
void opcode_sbc(Cpu *cpu, u8 byte);
void opcode_swap(Cpu *cpu, u8 *reg);
void opcode_swap_hl(Cpu *cpu);
void opcode_srl(Cpu *cpu, u8 *reg);
void opcode_srl_hl(Cpu *cpu);
void opcode_sla(Cpu *cpu, u8 *reg);
void opcode_sla_hl(Cpu *cpu);
void opcode_sra(Cpu *cpu, u8 *reg);
void opcode_sra_hl(Cpu *cpu);

void opcode_and(Cpu *cpu, u8 byte);
void opcode_xor(Cpu *cpu, u8 byte);
void opcode_and(Cpu *cpu, u8 byte);
void opcode_or(Cpu *cpu, u8 byte);
void opcode_bit(Cpu *cpu, u8 *reg, int bit);
void opcode_bit_hl(Cpu *cpu, int bit);
void opcode_set(Cpu *cpu, u8 *reg, int bit);
void opcode_set_hl(Cpu *cpu, int bit);
void opcode_res(Cpu *cpu, u8 *reg, int bit);
void opcode_res_hl(Cpu *cpu, int bit);
void opcode_cp(Cpu *cpu, u8 byte);

void opcode_call_nn(Cpu *cpu);
void opcode_stack_push(Cpu *cpu, u8 *r1, u8 *r2);
void opcode_stack_push_pc(Cpu *cpu, u16 *pc);
void opcode_stack_pop(Cpu *cpu, u8 *r1, u8 *r2);
void opcode_stack_pop_pc(Cpu *cpu, u16 *pc);

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
void opcode_execute(Cpu *cpu, u8 opcode);
void opcode_execute_cb(Cpu *cpu, u8 opcode);

#endif
