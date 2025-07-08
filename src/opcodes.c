#include "gb/cpu.h"
#include "gb/opcodes.h"
#include "gb/timer.h"

void opcode_execute(Cpu *cpu, u8 opcode)
{
	switch (opcode) {
	case 0x00:
		// NOOP
		break;
	case 0x01:
		SET_BC(cpu, cpu_read_word(cpu));
		break;
	case 0x02:
		// LD (BC),a
		MEM_WRITE(cpu, BC(cpu), cpu->a);
		break;
	case 0x03:
		// INC BC
		INC_BC(cpu);
		break;
	case 0x04:
		// Z 0 H -
		opcode_increment(cpu, &cpu->b);
		break;
	case 0x05:
		// Z 1 H -
		opcode_decrement(cpu, &cpu->b);
		break;
	case 0x06:
		// LD B,n
		opcode_ld(&cpu->b, cpu_read_byte(cpu));
		break;
	case 0x07:
		// RLCA
		opcode_rlc(cpu, &cpu->a, true);
		break;
	case 0x08:
		// LD (nn),SP
		opcode_ld_nn(cpu, &cpu->sp);
		break;
	case 0x09:
		// ADD HL, BC
		// - 0 H C
		opcode_add_hl(cpu, BC(cpu));
		break;
	case 0x0A:
		// LD a,(BC)
		opcode_ld(&cpu->a, MEM_READ(cpu, BC(cpu)));
		break;
	case 0x0B:
		// DEC BC
		DEC_BC(cpu);
		break;
	case 0x0C:
		// Z 0 H -
		opcode_increment(cpu, &cpu->c);
		break;
	case 0x0D:
		// Z 1 H -
		opcode_decrement(cpu, &cpu->c);
		break;
	case 0x0E:
		// LD c,n
		opcode_ld(&cpu->c, cpu_read_byte(cpu));
		break;
	case 0x0F:
		// RRCA
		// 0 0 0 C
		opcode_rrc(cpu, &cpu->a, true);
		break;
	case 0x10:
		// STOP n8
		timer_reset_div(cpu);
		break;
	case 0x11:
		// LD DE,nn
		SET_DE(cpu, cpu_read_word(cpu));
		break;
	case 0x12:
		// LD (DE),a
		MEM_WRITE(cpu, DE(cpu), cpu->a);
		break;
	case 0x13:
		// INC de
		INC_DE(cpu);
		break;
	case 0x14:
		// Z 0 H -
		opcode_increment(cpu, &cpu->d);
		break;
	case 0x15:
		// Z 1 H -
		opcode_decrement(cpu, &cpu->d);
		break;
	case 0x16:
		// LD D,n
		opcode_ld(&cpu->d, cpu_read_byte(cpu));
		break;
	case 0x17:
		// RLA
		// 0 0 0 C
		opcode_rl(cpu, &cpu->a, true);
		break;
	case 0x18:
		// JR e8
		cpu->pc = MEM_READ_PC_S8(cpu);
		break;
	case 0x19:
		// ADD HL, DE
		// - 0 H C
		opcode_add_hl(cpu, DE(cpu));
		break;
	case 0x1A:
		// LD,A,(DE)
		opcode_ld(&cpu->a, MEM_READ(cpu, DE(cpu)));
		break;
	case 0x1B:
		// DEC DE
		DEC_DE(cpu);
		break;
	case 0x1C:
		// Z 0 H -
		opcode_increment(cpu, &cpu->e);
		break;
	case 0x1D:
		// Z 1 H -
		opcode_decrement(cpu, &cpu->e);
		break;
	case 0x1E:
		// LD E,n
		opcode_ld(&cpu->e, cpu_read_byte(cpu));
		break;
	case 0x1F:
		// RRA
		// 0 0 0 C
		opcode_rr(cpu, &cpu->a, true);
		break;
	case 0x20:
		// JR NZ,e8
		if (!cpu_flag_is_set(cpu, FLAG_ZERO)) {
			cpu->pc = MEM_READ_PC_S8(cpu);
			cpu->cycles = 3;
		} else {
			cpu_pc_increment(cpu);
		}
		break;
	case 0x21:
		// LD HL,nn
		SET_HL(cpu, cpu_read_word(cpu));
		break;
	case 0x22:
		// LD [HL+], A
		MEM_WRITE(cpu, HL(cpu), cpu->a);
		INC_HL(cpu);
		break;
	case 0x23:
		// INC hl
		INC_HL(cpu);
		break;
	case 0x24:
		// INC h
		// Z 0 H -
		opcode_increment(cpu, &cpu->h);
		break;
	case 0x25:
		// DEC h
		// Z 1 H -
		opcode_decrement(cpu, &cpu->h);
		break;
	case 0x26:
		// LD h,n
		opcode_ld(&cpu->h, cpu_read_byte(cpu));
		break;
	case 0x27:
		// DAA
		// Z - 0 C
		opcode_daa(cpu);
		break;
	case 0x28:
		// JR Z,e8
		if (cpu_flag_is_set(cpu, FLAG_ZERO)) {
			cpu->pc = MEM_READ_PC_S8(cpu);
			cpu->cycles = 3;
		} else {
			cpu_pc_increment(cpu);
		}
		break;
	case 0x29:
		// ADD HL,HL
		// - 0 H C
		opcode_add_hl(cpu, HL(cpu));
		break;
	case 0x2A:
		// LD A,[HL+]
		opcode_ld(&cpu->a, MEM_READ(cpu, HL(cpu)));
		INC_HL(cpu);
		break;
	case 0x2B:
		// DEC hl
		DEC_HL(cpu);
		break;
	case 0x2C:
		// INC l
		// Z 0 H -
		opcode_increment(cpu, &cpu->l);
		break;
	case 0x2D:
		// DEC l
		// Z 1 H -
		opcode_decrement(cpu, &cpu->l);
		break;
	case 0x2E:
		// LD L,n
		cpu->l = cpu_read_byte(cpu);
		break;
	case 0x2F:
		// CPL
		// - 1 1 -
		cpu->a = ~cpu->a;
		cpu_flag_toggle(cpu, FLAG_HALF);
		cpu_flag_toggle(cpu, FLAG_SUBS);
		break;
	case 0x30:
		// JR NC,n
		if (!cpu_flag_is_set(cpu, FLAG_CARRY)) {
			cpu->pc = MEM_READ_PC_S8(cpu);
			cpu->cycles = 3;
		} else {
			cpu_pc_increment(cpu);
		}
		break;
	case 0x31:
		// LD sp,nn
		cpu->sp = cpu_read_word(cpu);
		break;
	case 0x32:
		// LD (HLD), A
		MEM_WRITE(cpu, HL(cpu), cpu->a);
		DEC_HL(cpu);
		break;
	case 0x33:
		// INC sp
		cpu->sp++;
		break;
	case 0x34:
		// INC (HL)
		opcode_inc_hl(cpu);
		break;
	case 0x35:
		// DEC (HL)
		opcode_dec_hl(cpu);
		break;
	case 0x36:
		// LD (HL),n
		MEM_WRITE(cpu, HL(cpu), cpu_read_byte(cpu));
		break;
	case 0x37:
		// SCF
		cpu_flag_toggle(cpu, FLAG_CARRY);
		cpu_flag_untoggle(cpu, FLAG_HALF);
		cpu_flag_untoggle(cpu, FLAG_SUBS);
		break;
	case 0x38:
		// JR C,n
		if (cpu_flag_is_set(cpu, FLAG_CARRY)) {
			cpu->pc = MEM_READ_PC_S8(cpu);
			cpu->cycles = 3;
		} else {
			cpu_pc_increment(cpu);
		}
		break;
	case 0x39:
		// ADD HL,SP
		opcode_add_hl(cpu, cpu->sp);
		break;
	case 0x3A:
		// LD A,(HLD)
		opcode_ld(&cpu->a, MEM_READ(cpu, HL(cpu)));
		DEC_HL(cpu);
		break;
	case 0x3B:
		// DEC SP
		cpu->sp--;
		break;
	case 0x3C:
		// INC A
		opcode_increment(cpu, &cpu->a);
		break;
	case 0x3D:
		// DEC A
		opcode_decrement(cpu, &cpu->a);
		break;
	case 0x3E:
		// LD A,n
		opcode_ld(&cpu->a, cpu_read_byte(cpu));
		break;
	case 0x3F:
		// CCF
		cpu_flag_flip(cpu, FLAG_CARRY);
		cpu_flag_untoggle(cpu, FLAG_HALF);
		cpu_flag_untoggle(cpu, FLAG_SUBS);
		break;
	case 0x40:
		// LD B,B
		opcode_ld(&cpu->b, cpu->b);
		break;
	case 0x41:
		// LD B,C
		opcode_ld(&cpu->b, cpu->c);
		break;
	case 0x42:
		// LD B,D
		opcode_ld(&cpu->b, cpu->d);
		break;
	case 0x43:
		// LD B,E
		opcode_ld(&cpu->b, cpu->e);
		break;
	case 0x44:
		// LD B,H
		opcode_ld(&cpu->b, cpu->h);
		break;
	case 0x45:
		// LD B,L
		opcode_ld(&cpu->b, cpu->l);
		break;
	case 0x46:
		// LD B,[HL]
		opcode_ld(&cpu->b, MEM_READ(cpu, HL(cpu)));
		break;
	case 0x47:
		// LD B,A
		opcode_ld(&cpu->b, cpu->a);
		break;
	case 0x48:
		// LD C,B
		opcode_ld(&cpu->c, cpu->b);
		break;
	case 0x49:
		// LD C,C
		opcode_ld(&cpu->c, cpu->c);
		break;
	case 0x4A:
		// LD C,D
		opcode_ld(&cpu->c, cpu->d);
		break;
	case 0x4B:
		// LD C,E
		opcode_ld(&cpu->c, cpu->e);
		break;
	case 0x4C:
		// LD C,H
		opcode_ld(&cpu->c, cpu->h);
		break;
	case 0x4D:
		// LD C,L
		opcode_ld(&cpu->c, cpu->l);
		break;
	case 0x4E:
		// LD C,[HL]
		opcode_ld(&cpu->c, MEM_READ(cpu, HL(cpu)));
		break;
	case 0x4F:
		// LD C,A
		opcode_ld(&cpu->c, cpu->a);
		break;
	case 0x50:
		// LD D,B
		opcode_ld(&cpu->d, cpu->b);
		break;
	case 0x51:
		// LD D,C
		opcode_ld(&cpu->d, cpu->c);
		break;
	case 0x52:
		// LD D,D
		opcode_ld(&cpu->d, cpu->d);
		break;
	case 0x53:
		// LD D,E
		opcode_ld(&cpu->d, cpu->e);
		break;
	case 0x54:
		// LD D,H
		opcode_ld(&cpu->d, cpu->h);
		break;
	case 0x55:
		// LD D,L
		opcode_ld(&cpu->d, cpu->l);
		break;
	case 0x56:
		// LD D,[HL]
		opcode_ld(&cpu->d, MEM_READ(cpu, HL(cpu)));
		break;
	case 0x57:
		// LD D,A
		opcode_ld(&cpu->d, cpu->a);
		break;
	case 0x58:
		// LD E,B
		opcode_ld(&cpu->e, cpu->b);
		break;
	case 0x59:
		// LD E,C
		opcode_ld(&cpu->e, cpu->c);
		break;
	case 0x5A:
		// LD E,D
		opcode_ld(&cpu->e, cpu->d);
		break;
	case 0x5B:
		// LD E,E
		opcode_ld(&cpu->e, cpu->e);
		break;
	case 0x5C:
		// LD E,H
		opcode_ld(&cpu->e, cpu->h);
		break;
	case 0x5D:
		// LD E,L
		opcode_ld(&cpu->e, cpu->l);
		break;
	case 0x5E:
		// LD E,[HL]
		opcode_ld(&cpu->e, MEM_READ(cpu, HL(cpu)));
		break;
	case 0x5F:
		// LD E,A
		opcode_ld(&cpu->e, cpu->a);
		break;
	case 0x60:
		// LD H,B
		opcode_ld(&cpu->h, cpu->b);
		break;
	case 0x61:
		// LD H,C
		opcode_ld(&cpu->h, cpu->c);
		break;
	case 0x62:
		// LD H,D
		opcode_ld(&cpu->h, cpu->d);
		break;
	case 0x63:
		// LD H,E
		opcode_ld(&cpu->h, cpu->e);
		break;
	case 0x64:
		// LD H,H
		opcode_ld(&cpu->h, cpu->h);
		break;
	case 0x65:
		// LD H,L
		opcode_ld(&cpu->h, cpu->l);
		break;
	case 0x66:
		// LD H,[HL]
		opcode_ld(&cpu->h, MEM_READ(cpu, HL(cpu)));
		break;
	case 0x67:
		// LD H,A
		opcode_ld(&cpu->h, cpu->a);
		break;
	case 0x68:
		// LD L,B
		opcode_ld(&cpu->l, cpu->b);
		break;
	case 0x69:
		// LD L,C
		opcode_ld(&cpu->l, cpu->c);
		break;
	case 0x6A:
		// LD L,D
		opcode_ld(&cpu->l, cpu->d);
		break;
	case 0x6B:
		// LD L,E
		opcode_ld(&cpu->l, cpu->e);
		break;
	case 0x6C:
		// LD L,H
		opcode_ld(&cpu->l, cpu->h);
		break;
	case 0x6D:
		// LD L,L
		opcode_ld(&cpu->l, cpu->l);
		break;
	case 0x6E:
		// LD L,[HL]
		opcode_ld(&cpu->l, MEM_READ(cpu, HL(cpu)));
		break;
	case 0x6F:
		// LD L,A
		opcode_ld(&cpu->l, cpu->a);
		break;
	case 0x70:
		// LD [HL],B
		MEM_WRITE(cpu, HL(cpu), cpu->b);
		break;
	case 0x71:
		// LD [HL],C
		MEM_WRITE(cpu, HL(cpu), cpu->c);
		break;
	case 0x72:
		// LD [HL],D
		MEM_WRITE(cpu, HL(cpu), cpu->d);
		break;
	case 0x73:
		// LD [HL],E
		MEM_WRITE(cpu, HL(cpu), cpu->e);
		break;
	case 0x74:
		// LD [HL],H
		MEM_WRITE(cpu, HL(cpu), cpu->h);
		break;
	case 0x75:
		// LD [HL],L
		MEM_WRITE(cpu, HL(cpu), cpu->l);
		break;
	case 0x76:
		// HALT
		// TODO
		cpu->halted = true;
		break;
	case 0x77:
		// LD [HL],A
		MEM_WRITE(cpu, HL(cpu), cpu->a);
		break;
	case 0x78:
		// LD A,B
		opcode_ld(&cpu->a, cpu->b);
		break;
	case 0x79:
		// LD A,C
		opcode_ld(&cpu->a, cpu->c);
		break;
	case 0x7A:
		// LD A,D
		opcode_ld(&cpu->a, cpu->d);
		break;
	case 0x7B:
		// LD A,E
		opcode_ld(&cpu->a, cpu->e);
		break;
	case 0x7C:
		// LD A,H
		opcode_ld(&cpu->a, cpu->h);
		break;
	case 0x7D:
		// LD A,L
		opcode_ld(&cpu->a, cpu->l);
		break;
	case 0x7E:
		// LD A,[HL]
		opcode_ld(&cpu->a, MEM_READ(cpu, HL(cpu)));
		break;
	case 0x7F:
		// LD A,A
		opcode_ld(&cpu->a, cpu->a);
		break;
	case 0x80:
		// ADD A,B
		opcode_add(cpu, cpu->b);
		break;
	case 0x81:
		// ADD A,C
		opcode_add(cpu, cpu->c);
		break;
	case 0x82:
		// ADD A,D
		opcode_add(cpu, cpu->d);
		break;
	case 0x83:
		// ADD A,E
		opcode_add(cpu, cpu->e);
		break;
	case 0x84:
		// ADD A,H
		opcode_add(cpu, cpu->h);
		break;
	case 0x85:
		// ADD A,L
		opcode_add(cpu, cpu->l);
		break;
	case 0x86:
		// ADD A,L
		opcode_add(cpu, MEM_READ(cpu, HL(cpu)));
		break;
	case 0x87:
		// ADD A,A
		opcode_add(cpu, cpu->a);
		break;
	case 0x88:
		// ADC A,B
		opcode_adc(cpu, cpu->b);
		break;
	case 0x89:
		// ADC A,C
		opcode_adc(cpu, cpu->c);
		break;
	case 0x8A:
		// ADC A,D
		opcode_adc(cpu, cpu->d);
		break;
	case 0x8B:
		// ADC A,E
		opcode_adc(cpu, cpu->e);
		break;
	case 0x8C:
		// ADC A,H
		opcode_adc(cpu, cpu->h);
		break;
	case 0x8D:
		// ADC A,L
		opcode_adc(cpu, cpu->l);
		break;
	case 0x8E:
		// ADC A,[HL]
		opcode_adc(cpu, MEM_READ(cpu, HL(cpu)));
		break;
	case 0x8F:
		// ADC A,A
		opcode_adc(cpu, cpu->a);
		break;
	case 0x90:
		// SUB A,B
		opcode_sub(cpu, cpu->b);
		break;
	case 0x91:
		// SUB A,C
		opcode_sub(cpu, cpu->c);
		break;
	case 0x92:
		// SUB A,D
		opcode_sub(cpu, cpu->d);
		break;
	case 0x93:
		// SUB A,E
		opcode_sub(cpu, cpu->e);
		break;
	case 0x94:
		// SUB A,H
		opcode_sub(cpu, cpu->h);
		break;
	case 0x95:
		// SUB A,L
		opcode_sub(cpu, cpu->l);
		break;
	case 0x96:
		// SUB A,[HL]
		opcode_sub(cpu, MEM_READ(cpu, HL(cpu)));
		break;
	case 0x97:
		// SUB A,A
		opcode_sub(cpu, cpu->a);
		break;
	case 0x98:
		// SBC A,B
		opcode_sbc(cpu, cpu->b);
		break;
	case 0x99:
		// SBC A,C
		opcode_sbc(cpu, cpu->c);
		break;
	case 0x9A:
		// SBC A,D
		opcode_sbc(cpu, cpu->d);
		break;
	case 0x9B:
		// SBC A,E
		opcode_sbc(cpu, cpu->e);
		break;
	case 0x9C:
		// SBC A,H
		opcode_sbc(cpu, cpu->h);
		break;
	case 0x9D:
		// SBC A,L
		opcode_sbc(cpu, cpu->l);
		break;
	case 0x9E:
		// SBC A,[HL]
		opcode_sbc(cpu, MEM_READ(cpu, HL(cpu)));
		break;
	case 0x9F:
		// SBC A,A
		opcode_sbc(cpu, cpu->a);
		break;
	case 0xA0:
		// AND A,B
		opcode_and(cpu, cpu->b);
		break;
	case 0xA1:
		// AND A,C
		opcode_and(cpu, cpu->c);
		break;
	case 0xA2:
		// AND A,D
		opcode_and(cpu, cpu->d);
		break;
	case 0xA3:
		// AND A,E
		opcode_and(cpu, cpu->e);
		break;
	case 0xA4:
		// AND A,H
		opcode_and(cpu, cpu->h);
		break;
	case 0xA5:
		// AND A,L
		opcode_and(cpu, cpu->l);
		break;
	case 0xA6:
		// AND A,[HL]
		opcode_and(cpu, MEM_READ(cpu, HL(cpu)));
		break;
	case 0xA7:
		// AND A,A
		opcode_and(cpu, cpu->a);
		break;
	case 0xA8:
		// XOR A,B
		opcode_xor(cpu, cpu->b);
		break;
	case 0xA9:
		// XOR A,C
		opcode_xor(cpu, cpu->c);
		break;
	case 0xAA:
		// XOR A,D
		opcode_xor(cpu, cpu->d);
		break;
	case 0xAB:
		// XOR A,E
		opcode_xor(cpu, cpu->e);
		break;
	case 0xAC:
		// XOR A,H
		opcode_xor(cpu, cpu->h);
		break;
	case 0xAD:
		// XOR A,L
		opcode_xor(cpu, cpu->l);
		break;
	case 0xAE:
		// XOR A,[HL]
		opcode_xor(cpu, MEM_READ(cpu, HL(cpu)));
		break;
	case 0xAF:
		// XOR A,A
		opcode_xor(cpu, cpu->a);
		break;
	case 0xB0:
		// OR A,B
		opcode_or(cpu, cpu->b);
		break;
	case 0xB1:
		// OR A,C
		opcode_or(cpu, cpu->c);
		break;
	case 0xB2:
		// OR A,D
		opcode_or(cpu, cpu->d);
		break;
	case 0xB3:
		// OR A,E
		opcode_or(cpu, cpu->e);
		break;
	case 0xB4:
		// OR A,H
		opcode_or(cpu, cpu->h);
		break;
	case 0xB5:
		// OR A,L
		opcode_or(cpu, cpu->l);
		break;
	case 0xB6:
		// OR A,[HL]
		opcode_or(cpu, MEM_READ(cpu, HL(cpu)));
		break;
	case 0xB7:
		// OR A,A
		opcode_or(cpu, cpu->a);
		break;
	case 0xB8:
		// CP A,B
		opcode_cp(cpu, cpu->b);
		break;
	case 0xB9:
		// CP A,C
		opcode_cp(cpu, cpu->c);
		break;
	case 0xBA:
		// CP A,D
		opcode_cp(cpu, cpu->d);
		break;
	case 0xBB:
		// CP A,E
		opcode_cp(cpu, cpu->e);
		break;
	case 0xBC:
		// CP A,H
		opcode_cp(cpu, cpu->h);
		break;
	case 0xBD:
		// CP A,L
		opcode_cp(cpu, cpu->l);
		break;
	case 0xBE:
		// CP A,[HL]
		opcode_cp(cpu, MEM_READ(cpu, HL(cpu)));
		break;
	case 0xBF:
		// CP A,A
		cpu_flag_clear(cpu);
		cpu_flag_toggle(cpu, FLAG_ZERO);
		cpu_flag_toggle(cpu, FLAG_SUBS);
		break;
	case 0xC0:
		// RET NZ
		if (!cpu_flag_is_set(cpu, FLAG_ZERO)) {
			opcode_stack_pop_pc(cpu, &cpu->pc);
			cpu->cycles = 5;
		}
		break;
	case 0xC1:
		// POP BC
		opcode_stack_pop(cpu, &cpu->b, &cpu->c);
		break;
	case 0xC2:
		// JP NZ,nn
		if (!cpu_flag_is_set(cpu, FLAG_ZERO)) {
			cpu->pc = cpu_read_word(cpu);
			cpu->cycles = 4;
		} else {
			cpu_pc_increment(cpu);
			cpu_pc_increment(cpu);
		}
		break;
	case 0xC3:
		// JP nn
		cpu->pc = cpu_read_word(cpu);
		break;
	case 0xC4:
		// CALL NZ,nn
		if (!cpu_flag_is_set(cpu, FLAG_ZERO)) {
			u16 result = cpu_read_word(cpu);
			opcode_stack_push_pc(cpu, &cpu->pc);
			cpu->pc = result;
			cpu->cycles = 6;
		} else {
			cpu_pc_increment(cpu);
			cpu_pc_increment(cpu);
		}
		break;
	case 0xC5:
		// PUSH BC
		opcode_stack_push(cpu, &cpu->b, &cpu->c);
		break;
	case 0xC6:
		// ADD A,n
		opcode_add(cpu, cpu_read_byte(cpu));
		break;
	case 0xC7:
		// RST 00H
		opcode_rst(cpu, 0x00);
		break;
	case 0xC8:
		// RET Z
		if (cpu_flag_is_set(cpu, FLAG_ZERO)) {
			opcode_stack_pop_pc(cpu, &cpu->pc);
			cpu->cycles = 5;
		}
		break;
	case 0xC9:
		// RET
		opcode_stack_pop_pc(cpu, &cpu->pc);
		break;
	case 0xCA:
		// JP Z,nn
		if (cpu_flag_is_set(cpu, FLAG_ZERO)) {
			cpu->pc = cpu_read_word(cpu);
			cpu->cycles = 4;
		} else {
			cpu_pc_increment(cpu);
			cpu_pc_increment(cpu);
		}
		break;
	case 0xCB:
		// Prefix
		break;
	case 0xCC:
		// CALL Z,nn
		if (cpu_flag_is_set(cpu, FLAG_ZERO)) {
			u16 result = cpu_read_word(cpu);
			opcode_stack_push_pc(cpu, &cpu->pc);
			cpu->pc = result;
			cpu->cycles = 6;
		} else {
			cpu_pc_increment(cpu);
			cpu_pc_increment(cpu);
		}
		break;
	case 0xCD:
		// CALL nn
		opcode_call_nn(cpu);
		break;
	case 0xCE:
		// ADC A, n
		opcode_adc(cpu, cpu_read_byte(cpu));
		break;
	case 0xCF:
		// RST 08H
		opcode_rst(cpu, 0x08);
		break;
	case 0xD0:
		// RET NC
		if (!cpu_flag_is_set(cpu, FLAG_CARRY)) {
			opcode_stack_pop_pc(cpu, &cpu->pc);
			cpu->cycles = 5;
		}
		break;
	case 0xD1:
		// POP DE
		opcode_stack_pop(cpu, &cpu->d, &cpu->e);
		break;
	case 0xD2:
		// JP NC,nn
		if (!cpu_flag_is_set(cpu, FLAG_CARRY)) {
			cpu->pc = cpu_read_word(cpu);
			cpu->cycles = 4;
		} else {
			cpu_pc_increment(cpu);
			cpu_pc_increment(cpu);
		}
		break;
	case 0xD3:
		// No code
		break;
	case 0xD4:
		// CALL NC,nn
		if (!cpu_flag_is_set(cpu, FLAG_CARRY)) {
			opcode_call_nn(cpu);
			cpu->cycles = 6;
		} else {
			cpu_pc_increment(cpu);
			cpu_pc_increment(cpu);
		}
		break;
	case 0xD5:
		// PUSH DE
		opcode_stack_push(cpu, &cpu->d, &cpu->e);
		break;
	case 0xD6:
		// SUB n
		opcode_sub(cpu, cpu_read_byte(cpu));
		break;
	case 0xD7:
		// RST 10H
		opcode_rst(cpu, 0x10);
		break;
	case 0xD8:
		// RET C
		if (cpu_flag_is_set(cpu, FLAG_CARRY)) {
			opcode_stack_pop_pc(cpu, &cpu->pc);
			cpu->cycles = 5;
		}
		break;
	case 0xD9:
		// RETI
		opcode_stack_pop_pc(cpu, &cpu->pc);
		cpu->ime = true;
		break;
	case 0xDA:
		// JP C,nn
		if (cpu_flag_is_set(cpu, FLAG_CARRY)) {
			cpu->pc = cpu_read_word(cpu);
			cpu->cycles = 4;
		} else {
			cpu_pc_increment(cpu);
			cpu_pc_increment(cpu);
		}
		break;
	case 0xDB:
		// No code
		break;
	case 0xDC:
		// CALL C,nn
		if (cpu_flag_is_set(cpu, FLAG_CARRY)) {
			opcode_call_nn(cpu);
			cpu->cycles = 6;
		} else {
			cpu_pc_increment(cpu);
			cpu_pc_increment(cpu);
		}
		break;
	case 0xDD:
		// No code
		break;
	case 0xDE:
		// SBC n
		opcode_sbc(cpu, cpu_read_byte(cpu));
		break;
	case 0xDF:
		// RST 18H
		opcode_rst(cpu, 0x18);
		break;
	case 0xE0:
		// LD (0xFF00+n),A
		MEM_WRITE(cpu, (u16)(0xFF00 + MEM_READ(cpu, cpu->pc)), cpu->a);
		cpu_pc_increment(cpu);
		break;
	case 0xE1:
		// POP HL
		opcode_stack_pop(cpu, &cpu->h, &cpu->l);
		break;
	case 0xE2:
		// LD (0xFF00+C),A
		MEM_WRITE(cpu, (u16)(0xFF00 + cpu->c), cpu->a);
		break;
	case 0xE3:
		// No code
		break;
	case 0xE4:
		// No code
		break;
	case 0xE5:
		// PUSH HL
		opcode_stack_push(cpu, &cpu->h, &cpu->l);
		break;
	case 0xE6:
		// AND n
		opcode_and(cpu, cpu_read_byte(cpu));
		break;
	case 0xE7:
		// RST 20H
		opcode_rst(cpu, 0x20);
		break;
	case 0xE8:
		// ADD SP,n
		opcode_add_sp(cpu, (s8)cpu_read_byte(cpu));
		break;
	case 0xE9:
		// JP (HL)
		cpu->pc = HL(cpu);
		break;
	case 0xEA:
		// LD (nn),A
		MEM_WRITE(cpu, cpu_read_word(cpu), cpu->a);
		break;
	case 0xEB:
		// No code
		break;
	case 0xEC:
		// No code
		break;
	case 0xED:
		// No code
		break;
	case 0xEE:
		// XOR n
		opcode_xor(cpu, cpu_read_byte(cpu));
		break;
	case 0xEF:
		// RST 28H
		opcode_rst(cpu, 0x28);
		break;
	case 0xF0:
		// LD A,(0xFF00+n)
		opcode_ldh_a_n(cpu);
		break;
	case 0xF1:
		// POP AF
		opcode_stack_pop(cpu, &cpu->a, &cpu->f);
		cpu->f &= 0xF0;
		break;
	case 0xF2:
		// LD A,(C)
		cpu->a = MEM_READ(cpu, (u16)(0xFF00 + cpu->c));
		break;
	case 0xF3:
		// DI
		cpu->ime = false;
		cpu->ime_cycles = 0;
		break;
	case 0xF4:
		// No code
		break;
	case 0xF5:
		// PUSH AF
		opcode_stack_push(cpu, &cpu->a, &cpu->f);
		break;
	case 0xF6:
		// OR n
		opcode_or(cpu, cpu_read_byte(cpu));
		break;
	case 0xF7:
		// RST 30H
		opcode_rst(cpu, 0x30);
		break;
	case 0xF8:
		// LD HL,SP+n
		opcode_ld_spn(cpu);
		break;
	case 0xF9:
		// LD SP,HL
		cpu->sp = HL(cpu);
		break;
	case 0xFA:
		// LD A,(nn)
		cpu->a = MEM_READ(cpu, cpu_read_word(cpu));
		break;
	case 0xFB:
		// EI
		// TODO
		cpu->ime = true;
		break;
	case 0xFC:
		// No code
		break;
	case 0xFD:
		// No code
		break;
	case 0xFE:
		// CP
		opcode_cp(cpu, cpu_read_byte(cpu));
		break;
	case 0xFF:
		// RST 38H
		opcode_rst(cpu, 0x38);
		break;
	}
}

void opcode_execute_cb(Cpu *cpu, u8 opcode)
{
	switch (opcode) {
	case 0x00:
		// RLC B
		opcode_rlc(cpu, &cpu->b, false);
		break;
	case 0x01:
		// RLC C
		opcode_rlc(cpu, &cpu->c, false);
		break;
	case 0x02:
		// RLC D
		opcode_rlc(cpu, &cpu->d, false);
		break;
	case 0x03:
		// RLC E
		opcode_rlc(cpu, &cpu->e, false);
		break;
	case 0x04:
		// RLC H
		opcode_rlc(cpu, &cpu->h, false);
		break;
	case 0x05:
		// RLC L
		opcode_rlc(cpu, &cpu->l, false);
		break;
	case 0x06:
		// RLC [HL]
		opcode_rlc_hl(cpu);
		break;
	case 0x07:
		// RLC A
		opcode_rlc(cpu, &cpu->a, false);
		break;
	case 0x08:
		// RRC B
		opcode_rrc(cpu, &cpu->b, false);
		break;
	case 0x09:
		// RRC C
		opcode_rrc(cpu, &cpu->c, false);
		break;
	case 0x0A:
		// RRC D
		opcode_rrc(cpu, &cpu->d, false);
		break;
	case 0x0B:
		// RRC E
		opcode_rrc(cpu, &cpu->e, false);
		break;
	case 0x0C:
		// RRC H
		opcode_rrc(cpu, &cpu->h, false);
		break;
	case 0x0D:
		// RRC L
		opcode_rrc(cpu, &cpu->l, false);
		break;
	case 0x0E:
		// RRC [HL]
		opcode_rrc_hl(cpu);
		break;
	case 0x0F:
		// RRC A
		opcode_rrc(cpu, &cpu->a, false);
		break;
	case 0x10:
		// RL B
		opcode_rl(cpu, &cpu->b, false);
		break;
	case 0x11:
		// RL C
		opcode_rl(cpu, &cpu->c, false);
		break;
	case 0x12:
		// RL D
		opcode_rl(cpu, &cpu->d, false);
		break;
	case 0x13:
		// RL E
		opcode_rl(cpu, &cpu->e, false);
		break;
	case 0x14:
		// RL H
		opcode_rl(cpu, &cpu->h, false);
		break;
	case 0x15:
		// RL L
		opcode_rl(cpu, &cpu->l, false);
		break;
	case 0x16:
		// RL [HL]
		opcode_rl_hl(cpu);
		break;
	case 0x17:
		// RL A
		opcode_rl(cpu, &cpu->a, false);
		break;
	case 0x18:
		// RR B
		opcode_rr(cpu, &cpu->b, false);
		break;
	case 0x19:
		// RR C
		opcode_rr(cpu, &cpu->c, false);
		break;
	case 0x1A:
		// RR D
		opcode_rr(cpu, &cpu->d, false);
		break;
	case 0x1B:
		// RR E
		opcode_rr(cpu, &cpu->e, false);
		break;
	case 0x1C:
		// RR H
		opcode_rr(cpu, &cpu->h, false);
		break;
	case 0x1D:
		// RR L
		opcode_rr(cpu, &cpu->l, false);
		break;
	case 0x1E:
		// RR [HL]
		opcode_rr_hl(cpu);
		break;
	case 0x1F:
		// RR A
		opcode_rr(cpu, &cpu->a, false);
		break;
	case 0x20:
		// SLA B
		opcode_sla(cpu, &cpu->b);
		break;
	case 0x21:
		// SLA C
		opcode_sla(cpu, &cpu->c);
		break;
	case 0x22:
		// SLA D
		opcode_sla(cpu, &cpu->d);
		break;
	case 0x23:
		// SLA E
		opcode_sla(cpu, &cpu->e);
		break;
	case 0x24:
		// SLA H
		opcode_sla(cpu, &cpu->h);
		break;
	case 0x25:
		// SLA L
		opcode_sla(cpu, &cpu->l);
		break;
	case 0x26:
		// SLA [HL]
		opcode_sla_hl(cpu);
		break;
	case 0x27:
		// SLA A
		opcode_sla(cpu, &cpu->a);
		break;
	case 0x28:
		// SRA B
		opcode_sra(cpu, &cpu->b);
		break;
	case 0x29:
		// SRA C
		opcode_sra(cpu, &cpu->c);
		break;
	case 0x2A:
		// SRA D
		opcode_sra(cpu, &cpu->d);
		break;
	case 0x2B:
		// SRA E
		opcode_sra(cpu, &cpu->e);
		break;
	case 0x2C:
		// SRA H
		opcode_sra(cpu, &cpu->h);
		break;
	case 0x2D:
		// SRA L
		opcode_sra(cpu, &cpu->l);
		break;
	case 0x2E:
		// SRA [HL]
		opcode_sra_hl(cpu);
		break;
	case 0x2F:
		// SRA A
		opcode_sra(cpu, &cpu->a);
		break;
	case 0x30:
		// SWAP B
		opcode_swap(cpu, &cpu->b);
		break;
	case 0x31:
		// SWAP C
		opcode_swap(cpu, &cpu->c);
		break;
	case 0x32:
		// SWAP D
		opcode_swap(cpu, &cpu->d);
		break;
	case 0x33:
		// SWAP E
		opcode_swap(cpu, &cpu->e);
		break;
	case 0x34:
		// SWAP H
		opcode_swap(cpu, &cpu->h);
		break;
	case 0x35:
		// SWAP L
		opcode_swap(cpu, &cpu->l);
		break;
	case 0x36:
		// SWAP [HL]
		opcode_swap_hl(cpu);
		break;
	case 0x37:
		// SWAP A
		opcode_swap(cpu, &cpu->a);
		break;
	case 0x38:
		// SRL B
		opcode_srl(cpu, &cpu->b);
		break;
	case 0x39:
		// SRL C
		opcode_srl(cpu, &cpu->c);
		break;
	case 0x3A:
		// SRL D
		opcode_srl(cpu, &cpu->d);
		break;
	case 0x3B:
		// SRL E
		opcode_srl(cpu, &cpu->e);
		break;
	case 0x3C:
		// SRL H
		opcode_srl(cpu, &cpu->h);
		break;
	case 0x3D:
		// SRL L
		opcode_srl(cpu, &cpu->l);
		break;
	case 0x3E:
		// SRL [HL]
		opcode_srl_hl(cpu);
		break;
	case 0x3F:
		// SRL A
		opcode_srl(cpu, &cpu->a);
		break;
	case 0x40:
		// BIT 0 B
		opcode_bit(cpu, &cpu->b, 0);
		break;
	case 0x41:
		// BIT 0 C
		opcode_bit(cpu, &cpu->c, 0);
		break;
	case 0x42:
		// BIT 0 D
		opcode_bit(cpu, &cpu->d, 0);
		break;
	case 0x43:
		// BIT 0 E
		opcode_bit(cpu, &cpu->e, 0);
		break;
	case 0x44:
		// BIT 0 H
		opcode_bit(cpu, &cpu->h, 0);
		break;
	case 0x45:
		// BIT 0 L
		opcode_bit(cpu, &cpu->l, 0);
		break;
	case 0x46:
		// BIT 0 [HL]
		opcode_bit_hl(cpu, 0);
		break;
	case 0x47:
		// BIT 0 A
		opcode_bit(cpu, &cpu->a, 0);
		break;
	case 0x48:
		// BIT 1 B
		opcode_bit(cpu, &cpu->b, 1);
		break;
	case 0x49:
		// BIT 1 C
		opcode_bit(cpu, &cpu->c, 1);
		break;
	case 0x4A:
		// BIT 1 D
		opcode_bit(cpu, &cpu->d, 1);
		break;
	case 0x4B:
		// BIT 1 E
		opcode_bit(cpu, &cpu->e, 1);
		break;
	case 0x4C:
		// BIT 1 H
		opcode_bit(cpu, &cpu->h, 1);
		break;
	case 0x4D:
		// BIT 1 L
		opcode_bit(cpu, &cpu->l, 1);
		break;
	case 0x4E:
		// BIT 1 [HL]
		opcode_bit_hl(cpu, 1);
		break;
	case 0x4F:
		// BIT 1 A
		opcode_bit(cpu, &cpu->a, 1);
		break;
	case 0x50:
		// BIT 2 B
		opcode_bit(cpu, &cpu->b, 2);
		break;
	case 0x51:
		// BIT 2 C
		opcode_bit(cpu, &cpu->c, 2);
		break;
	case 0x52:
		// BIT 2 D
		opcode_bit(cpu, &cpu->d, 2);
		break;
	case 0x53:
		// BIT 2 E
		opcode_bit(cpu, &cpu->e, 2);
		break;
	case 0x54:
		// BIT 2 H
		opcode_bit(cpu, &cpu->h, 2);
		break;
	case 0x55:
		// BIT 2 L
		opcode_bit(cpu, &cpu->l, 2);
		break;
	case 0x56:
		// BIT 2 [HL]
		opcode_bit_hl(cpu, 2);
		break;
	case 0x57:
		// BIT 2 A
		opcode_bit(cpu, &cpu->a, 2);
		break;
	case 0x58:
		// BIT 3 B
		opcode_bit(cpu, &cpu->b, 3);
		break;
	case 0x59:
		// BIT 3 C
		opcode_bit(cpu, &cpu->c, 3);
		break;
	case 0x5A:
		// BIT 3 D
		opcode_bit(cpu, &cpu->d, 3);
		break;
	case 0x5B:
		// BIT 3 E
		opcode_bit(cpu, &cpu->e, 3);
		break;
	case 0x5C:
		// BIT 3 H
		opcode_bit(cpu, &cpu->h, 3);
		break;
	case 0x5D:
		// BIT 3 L
		opcode_bit(cpu, &cpu->l, 3);
		break;
	case 0x5E:
		// BIT 3 [HL]
		opcode_bit_hl(cpu, 3);
		break;
	case 0x5F:
		// BIT 3 A
		opcode_bit(cpu, &cpu->a, 3);
		break;
	case 0x60:
		// BIT 4 B
		opcode_bit(cpu, &cpu->b, 4);
		break;
	case 0x61:
		// BIT 4 C
		opcode_bit(cpu, &cpu->c, 4);
		break;
	case 0x62:
		// BIT 4 D
		opcode_bit(cpu, &cpu->d, 4);
		break;
	case 0x63:
		// BIT 4 E
		opcode_bit(cpu, &cpu->e, 4);
		break;
	case 0x64:
		// BIT 4 H
		opcode_bit(cpu, &cpu->h, 4);
		break;
	case 0x65:
		// BIT 4 L
		opcode_bit(cpu, &cpu->l, 4);
		break;
	case 0x66:
		// BIT 4 [HL]
		opcode_bit_hl(cpu, 4);
		break;
	case 0x67:
		// BIT 4 A
		opcode_bit(cpu, &cpu->a, 4);
		break;
	case 0x68:
		// BIT 5 B
		opcode_bit(cpu, &cpu->b, 5);
		break;
	case 0x69:
		// BIT 5 C
		opcode_bit(cpu, &cpu->c, 5);
		break;
	case 0x6A:
		// BIT 5 D
		opcode_bit(cpu, &cpu->d, 5);
		break;
	case 0x6B:
		// BIT 5 E
		opcode_bit(cpu, &cpu->e, 5);
		break;
	case 0x6C:
		// BIT 5 H
		opcode_bit(cpu, &cpu->h, 5);
		break;
	case 0x6D:
		// BIT 5 L
		opcode_bit(cpu, &cpu->l, 5);
		break;
	case 0x6E:
		// BIT 5 [HL]
		opcode_bit_hl(cpu, 5);
		break;
	case 0x6F:
		// BIT 5 A
		opcode_bit(cpu, &cpu->a, 5);
		break;
	case 0x70:
		// BIT 6 B
		opcode_bit(cpu, &cpu->b, 6);
		break;
	case 0x71:
		// BIT 6 C
		opcode_bit(cpu, &cpu->c, 6);
		break;
	case 0x72:
		// BIT 6 D
		opcode_bit(cpu, &cpu->d, 6);
		break;
	case 0x73:
		// BIT 6 E
		opcode_bit(cpu, &cpu->e, 6);
		break;
	case 0x74:
		// BIT 6 H
		opcode_bit(cpu, &cpu->h, 6);
		break;
	case 0x75:
		// BIT 6 L
		opcode_bit(cpu, &cpu->l, 6);
		break;
	case 0x76:
		// BIT 6 [HL]
		opcode_bit_hl(cpu, 6);
		break;
	case 0x77:
		// BIT 6 A
		opcode_bit(cpu, &cpu->a, 6);
		break;
	case 0x78:
		// BIT 7 B
		opcode_bit(cpu, &cpu->b, 7);
		break;
	case 0x79:
		// BIT 7 C
		opcode_bit(cpu, &cpu->c, 7);
		break;
	case 0x7A:
		// BIT 7 D
		opcode_bit(cpu, &cpu->d, 7);
		break;
	case 0x7B:
		// BIT 7 E
		opcode_bit(cpu, &cpu->e, 7);
		break;
	case 0x7C:
		// BIT 7 H
		opcode_bit(cpu, &cpu->h, 7);
		break;
	case 0x7D:
		// BIT 7 L
		opcode_bit(cpu, &cpu->l, 7);
		break;
	case 0x7E:
		// BIT 7 [HL]
		opcode_bit_hl(cpu, 7);
		break;
	case 0x7F:
		// BIT 7 A
		opcode_bit(cpu, &cpu->a, 7);
		break;
	case 0x80:
		// RES 0 B
		opcode_res(cpu, &cpu->b, 0);
		break;
	case 0x81:
		// RES 0 C
		opcode_res(cpu, &cpu->c, 0);
		break;
	case 0x82:
		// RES 0 D
		opcode_res(cpu, &cpu->d, 0);
		break;
	case 0x83:
		// RES 0 E
		opcode_res(cpu, &cpu->e, 0);
		break;
	case 0x84:
		// RES 0 H
		opcode_res(cpu, &cpu->h, 0);
		break;
	case 0x85:
		// RES 0 L
		opcode_res(cpu, &cpu->l, 0);
		break;
	case 0x86:
		// RES 0 [HL]
		opcode_res_hl(cpu, 0);
		break;
	case 0x87:
		// RES 0 A
		opcode_res(cpu, &cpu->a, 0);
		break;
	case 0x88:
		// RES 1 B
		opcode_res(cpu, &cpu->b, 1);
		break;
	case 0x89:
		// RES 1 C
		opcode_res(cpu, &cpu->c, 1);
		break;
	case 0x8A:
		// RES 1 D
		opcode_res(cpu, &cpu->d, 1);
		break;
	case 0x8B:
		// RES 1 E
		opcode_res(cpu, &cpu->e, 1);
		break;
	case 0x8C:
		// RES 1 H
		opcode_res(cpu, &cpu->h, 1);
		break;
	case 0x8D:
		// RES 1 L
		opcode_res(cpu, &cpu->l, 1);
		break;
	case 0x8E:
		// RES 1 [HL]
		opcode_res_hl(cpu, 1);
		break;
	case 0x8F:
		// RES 1 A
		opcode_res(cpu, &cpu->a, 1);
		break;
	case 0x90:
		// RES 2 B
		opcode_res(cpu, &cpu->b, 2);
		break;
	case 0x91:
		// RES 2 C
		opcode_res(cpu, &cpu->c, 2);
		break;
	case 0x92:
		// RES 2 D
		opcode_res(cpu, &cpu->d, 2);
		break;
	case 0x93:
		// RES 2 E
		opcode_res(cpu, &cpu->e, 2);
		break;
	case 0x94:
		// RES 2 H
		opcode_res(cpu, &cpu->h, 2);
		break;
	case 0x95:
		// RES 2 L
		opcode_res(cpu, &cpu->l, 2);
		break;
	case 0x96:
		// RES 2 [HL]
		opcode_res_hl(cpu, 2);
		break;
	case 0x97:
		// RES 2 A
		opcode_res(cpu, &cpu->a, 2);
		break;
	case 0x98:
		// RES 3 B
		opcode_res(cpu, &cpu->b, 3);
		break;
	case 0x99:
		// RES 3 C
		opcode_res(cpu, &cpu->c, 3);
		break;
	case 0x9A:
		// RES 3 D
		opcode_res(cpu, &cpu->d, 3);
		break;
	case 0x9B:
		// RES 3 E
		opcode_res(cpu, &cpu->e, 3);
		break;
	case 0x9C:
		// RES 3 H
		opcode_res(cpu, &cpu->h, 3);
		break;
	case 0x9D:
		// RES 3 L
		opcode_res(cpu, &cpu->l, 3);
		break;
	case 0x9E:
		// RES 3 [HL]
		opcode_res_hl(cpu, 3);
		break;
	case 0x9F:
		// RES 3 A
		opcode_res(cpu, &cpu->a, 3);
		break;
	case 0xa0:
		// RES 4 B
		opcode_res(cpu, &cpu->b, 4);
		break;
	case 0xa1:
		// RES 4 C
		opcode_res(cpu, &cpu->c, 4);
		break;
	case 0xa2:
		// RES 4 D
		opcode_res(cpu, &cpu->d, 4);
		break;
	case 0xa3:
		// RES 4 E
		opcode_res(cpu, &cpu->e, 4);
		break;
	case 0xa4:
		// RES 4 H
		opcode_res(cpu, &cpu->h, 4);
		break;
	case 0xa5:
		// RES 4 L
		opcode_res(cpu, &cpu->l, 4);
		break;
	case 0xa6:
		// RES 4 [HL]
		opcode_res_hl(cpu, 4);
		break;
	case 0xa7:
		// RES 4 A
		opcode_res(cpu, &cpu->a, 4);
		break;
	case 0xa8:
		// RES 5 B
		opcode_res(cpu, &cpu->b, 5);
		break;
	case 0xa9:
		// RES 5 C
		opcode_res(cpu, &cpu->c, 5);
		break;
	case 0xAA:
		// RES 5 D
		opcode_res(cpu, &cpu->d, 5);
		break;
	case 0xAB:
		// RES 5 E
		opcode_res(cpu, &cpu->e, 5);
		break;
	case 0xAC:
		// RES 5 H
		opcode_res(cpu, &cpu->h, 5);
		break;
	case 0xAD:
		// RES 5 L
		opcode_res(cpu, &cpu->l, 5);
		break;
	case 0xAE:
		// RES 5 [HL]
		opcode_res_hl(cpu, 5);
		break;
	case 0xAF:
		// RES 5 A
		opcode_res(cpu, &cpu->a, 5);
		break;
	case 0xb0:
		// RES 6 B
		opcode_res(cpu, &cpu->b, 6);
		break;
	case 0xb1:
		// RES 6 C
		opcode_res(cpu, &cpu->c, 6);
		break;
	case 0xb2:
		// RES 6 D
		opcode_res(cpu, &cpu->d, 6);
		break;
	case 0xb3:
		// RES 6 E
		opcode_res(cpu, &cpu->e, 6);
		break;
	case 0xb4:
		// RES 6 H
		opcode_res(cpu, &cpu->h, 6);
		break;
	case 0xb5:
		// RES 6 L
		opcode_res(cpu, &cpu->l, 6);
		break;
	case 0xb6:
		// RES 6 [HL]
		opcode_res_hl(cpu, 6);
		break;
	case 0xb7:
		// RES 6 A
		opcode_res(cpu, &cpu->a, 6);
		break;
	case 0xb8:
		// RES 7 B
		opcode_res(cpu, &cpu->b, 7);
		break;
	case 0xb9:
		// RES 7 C
		opcode_res(cpu, &cpu->c, 7);
		break;
	case 0xBA:
		// RES 7 D
		opcode_res(cpu, &cpu->d, 7);
		break;
	case 0xBB:
		// RES 7 E
		opcode_res(cpu, &cpu->e, 7);
		break;
	case 0xBC:
		// RES 7 H
		opcode_res(cpu, &cpu->h, 7);
		break;
	case 0xBD:
		// RES 7 L
		opcode_res(cpu, &cpu->l, 7);
		break;
	case 0xBE:
		// RES 7 [HL]
		opcode_res_hl(cpu, 7);
		break;
	case 0xBF:
		// RES 7 A
		opcode_res(cpu, &cpu->a, 7);
		break;
	case 0xC0:
		// SET 0 B
		opcode_set(cpu, &cpu->b, 0);
		break;
	case 0xC1:
		// SET 0 C
		opcode_set(cpu, &cpu->c, 0);
		break;
	case 0xC2:
		// SET 0 D
		opcode_set(cpu, &cpu->d, 0);
		break;
	case 0xC3:
		// SET 0 E
		opcode_set(cpu, &cpu->e, 0);
		break;
	case 0xC4:
		// SET 0 H
		opcode_set(cpu, &cpu->h, 0);
		break;
	case 0xC5:
		// SET 0 L
		opcode_set(cpu, &cpu->l, 0);
		break;
	case 0xC6:
		// SET 0 [HL]
		opcode_set_hl(cpu, 0);
		break;
	case 0xC7:
		// SET 0 A
		opcode_set(cpu, &cpu->a, 0);
		break;
	case 0xC8:
		// SET 1 B
		opcode_set(cpu, &cpu->b, 1);
		break;
	case 0xC9:
		// SET 1 C
		opcode_set(cpu, &cpu->c, 1);
		break;
	case 0xCA:
		// SET 1 D
		opcode_set(cpu, &cpu->d, 1);
		break;
	case 0xCB:
		// SET 1 E
		opcode_set(cpu, &cpu->e, 1);
		break;
	case 0xCC:
		// SET 1 H
		opcode_set(cpu, &cpu->h, 1);
		break;
	case 0xCD:
		// SET 1 L
		opcode_set(cpu, &cpu->l, 1);
		break;
	case 0xCE:
		// SET 1 [HL]
		opcode_set_hl(cpu, 1);
		break;
	case 0xCF:
		// SET 1 A
		opcode_set(cpu, &cpu->a, 1);
		break;
	case 0xD0:
		// SET 2 B
		opcode_set(cpu, &cpu->b, 2);
		break;
	case 0xD1:
		// SET 2 C
		opcode_set(cpu, &cpu->c, 2);
		break;
	case 0xD2:
		// SET 2 D
		opcode_set(cpu, &cpu->d, 2);
		break;
	case 0xD3:
		// SET 2 E
		opcode_set(cpu, &cpu->e, 2);
		break;
	case 0xD4:
		// SET 2 H
		opcode_set(cpu, &cpu->h, 2);
		break;
	case 0xD5:
		// SET 2 L
		opcode_set(cpu, &cpu->l, 2);
		break;
	case 0xD6:
		// SET 2 [HL]
		opcode_set_hl(cpu, 2);
		break;
	case 0xD7:
		// SET 2 A
		opcode_set(cpu, &cpu->a, 2);
		break;
	case 0xD8:
		// SET 3 B
		opcode_set(cpu, &cpu->b, 3);
		break;
	case 0xD9:
		// SET 3 C
		opcode_set(cpu, &cpu->c, 3);
		break;
	case 0xDA:
		// SET 3 D
		opcode_set(cpu, &cpu->d, 3);
		break;
	case 0xDB:
		// SET 3 E
		opcode_set(cpu, &cpu->e, 3);
		break;
	case 0xDC:
		// SET 3 H
		opcode_set(cpu, &cpu->h, 3);
		break;
	case 0xDD:
		// SET 3 L
		opcode_set(cpu, &cpu->l, 3);
		break;
	case 0xDE:
		// SET 3 [HL]
		opcode_set_hl(cpu, 3);
		break;
	case 0xDF:
		// SET 3 A
		opcode_set(cpu, &cpu->a, 3);
		break;
	case 0xE0:
		// SET 4 B
		opcode_set(cpu, &cpu->b, 4);
		break;
	case 0xE1:
		// SET 4 C
		opcode_set(cpu, &cpu->c, 4);
		break;
	case 0xE2:
		// SET 4 D
		opcode_set(cpu, &cpu->d, 4);
		break;
	case 0xE3:
		// SET 4 E
		opcode_set(cpu, &cpu->e, 4);
		break;
	case 0xE4:
		// SET 4 H
		opcode_set(cpu, &cpu->h, 4);
		break;
	case 0xE5:
		// SET 4 L
		opcode_set(cpu, &cpu->l, 4);
		break;
	case 0xE6:
		// SET 4 [HL]
		opcode_set_hl(cpu, 4);
		break;
	case 0xE7:
		// SET 4 A
		opcode_set(cpu, &cpu->a, 4);
		break;
	case 0xE8:
		// SET 5 B
		opcode_set(cpu, &cpu->b, 5);
		break;
	case 0xE9:
		// SET 5 C
		opcode_set(cpu, &cpu->c, 5);
		break;
	case 0xEA:
		// SET 5 D
		opcode_set(cpu, &cpu->d, 5);
		break;
	case 0xEB:
		// SET 5 E
		opcode_set(cpu, &cpu->e, 5);
		break;
	case 0xEC:
		// SET 5 H
		opcode_set(cpu, &cpu->h, 5);
		break;
	case 0xED:
		// SET 5 L
		opcode_set(cpu, &cpu->l, 5);
		break;
	case 0xEE:
		// SET 5 [HL]
		opcode_set_hl(cpu, 5);
		break;
	case 0xEF:
		// SET 5 A
		opcode_set(cpu, &cpu->a, 5);
		break;
	case 0xF0:
		// SET 6 B
		opcode_set(cpu, &cpu->b, 6);
		break;
	case 0xF1:
		// SET 6 C
		opcode_set(cpu, &cpu->c, 6);
		break;
	case 0xF2:
		// SET 6 D
		opcode_set(cpu, &cpu->d, 6);
		break;
	case 0xF3:
		// SET 6 E
		opcode_set(cpu, &cpu->e, 6);
		break;
	case 0xF4:
		// SET 6 H
		opcode_set(cpu, &cpu->h, 6);
		break;
	case 0xF5:
		// SET 6 L
		opcode_set(cpu, &cpu->l, 6);
		break;
	case 0xF6:
		// SET 6 [HL]
		opcode_set_hl(cpu, 6);
		break;
	case 0xF7:
		// SET 6 A
		opcode_set(cpu, &cpu->a, 6);
		break;
	case 0xF8:
		// SET 7 B
		opcode_set(cpu, &cpu->b, 7);
		break;
	case 0xF9:
		// SET 7 C
		opcode_set(cpu, &cpu->c, 7);
		break;
	case 0xFA:
		// SET 7 D
		opcode_set(cpu, &cpu->d, 7);
		break;
	case 0xFB:
		// SET 7 E
		opcode_set(cpu, &cpu->e, 7);
		break;
	case 0xFC:
		// SET 7 H
		opcode_set(cpu, &cpu->h, 7);
		break;
	case 0xFD:
		// SET 7 L
		opcode_set(cpu, &cpu->l, 7);
		break;
	case 0xFE:
		// SET 7 [HL]
		opcode_set_hl(cpu, 7);
		break;
	case 0xFF:
		// SET 7 A
		opcode_set(cpu, &cpu->a, 7);
		break;
	}
}
