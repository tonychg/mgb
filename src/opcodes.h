#ifndef __OPCODE_H__
#define __OPCODE_H__

#include "cpu.h"

u8 opcode_get_high(u16 word);
u8 opcode_get_low(u16 word);
u16 opcode_get_value(u8 r1, u8 r2);
void opcode_increment(Cpu *cpu, u8 *reg);
void opcode_decrement(Cpu *cpu, u8 *reg);
void opcode_set_af(Cpu *cpu, u16 word);
void opcode_increment_af(Cpu *cpu);
void opcode_decrement_af(Cpu *cpu);
void opcode_set_bc(Cpu *cpu, u16 word);
u16 opcode_get_bc(Cpu *cpu);
void opcode_increment_bc(Cpu *cpu);
void opcode_decrement_bc(Cpu *cpu);
void opcode_set_de(Cpu *cpu, u16 word);
u16 opcode_get_de(Cpu *cpu);
void opcode_increment_de(Cpu *cpu);
void opcode_decrement_de(Cpu *cpu);
void opcode_set_hl(Cpu *cpu, u16 word);
u16 opcode_get_hl(Cpu *cpu);
void opcode_increment_hl(Cpu *cpu);
void opcode_decrement_hl(Cpu *cpu);
void opcode_execute(Cpu *cpu, Instruction instruction);
void opcode_execute_cb(Cpu *cpu, Instruction instruction);

#endif
