#include "cpu.h"
#include "opcodes.h"

void opcode_execute(Cpu *cpu, Instruction instruction)
{
	if (cpu->debug)
		cpu_debug_instruction(instruction);

	switch (instruction.opcode) {
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
		// DEC de
		DEC_DE(cpu);
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
			cpu->branch_taken = true;
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
			cpu->branch_taken = true;
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
		opcode_ld(&cpu->a, HL(cpu));
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
			cpu->branch_taken = true;
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
		MEM_WRITE(cpu, HL(cpu), MEM_READ(cpu, cpu->pc));
		cpu_pc_increment(cpu);
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
			cpu->branch_taken = true;
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
		opcode_ld(&cpu->a, HL(cpu));
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
		break;
	case 0x81:
		break;
	case 0x82:
		break;
	case 0x83:
		break;
	case 0x84:
		break;
	case 0x85:
		break;
	case 0x86:
		break;
	case 0x87:
		break;
	case 0x88:
		break;
	case 0x89:
		break;
	case 0x8A:
		break;
	case 0x8B:
		break;
	case 0x8C:
		break;
	case 0x8D:
		break;
	case 0x8E:
		break;
	case 0x8F:
		break;
	case 0x90:
		break;
	case 0x91:
		break;
	case 0x92:
		break;
	case 0x93:
		break;
	case 0x94:
		break;
	case 0x95:
		break;
	case 0x96:
		break;
	case 0x97:
		break;
	case 0x98:
		break;
	case 0x99:
		break;
	case 0x9A:
		break;
	case 0x9B:
		break;
	case 0x9C:
		break;
	case 0x9D:
		break;
	case 0x9E:
		break;
	case 0x9F:
		break;
	case 0xA0:
		break;
	case 0xA1:
		break;
	case 0xA2:
		break;
	case 0xA3:
		break;
	case 0xA4:
		break;
	case 0xA5:
		break;
	case 0xA6:
		break;
	case 0xA7:
		break;
	case 0xA8:
		break;
	case 0xAA:
		break;
	case 0xAB:
		break;
	case 0xAC:
		break;
	case 0xAD:
		break;
	case 0xAE:
		break;
	case 0xAF:
		break;
	case 0xB0:
		break;
	case 0xB1:
		break;
	case 0xB2:
		break;
	case 0xB3:
		break;
	case 0xB4:
		break;
	case 0xB5:
		break;
	case 0xB6:
		break;
	case 0xB7:
		break;
	case 0xB8:
		break;
	case 0xBA:
		break;
	case 0xBB:
		break;
	case 0xBC:
		break;
	case 0xBD:
		break;
	case 0xBE:
		break;
	case 0xBF:
		break;
	case 0xC0:
		break;
	case 0xC1:
		break;
	case 0xC2:
		break;
	case 0xC3:
		cpu->pc = cpu_read_word(cpu);
		break;
	case 0xC4:
		break;
	case 0xC5:
		break;
	case 0xC6:
		break;
	case 0xC7:
		break;
	case 0xC8:
		break;
	case 0xCA:
		break;
	case 0xCB:
		// Prefix
		break;
	case 0xCC:
		break;
	case 0xCD:
		break;
	case 0xCE:
		break;
	case 0xCF:
		break;
	case 0xD0:
		break;
	case 0xD1:
		break;
	case 0xD2:
		break;
	case 0xD3:
		// No code
		break;
	case 0xD4:
		break;
	case 0xD5:
		break;
	case 0xD6:
		break;
	case 0xD7:
		break;
	case 0xD8:
		break;
	case 0xDA:
		break;
	case 0xDB:
		// No code
		break;
	case 0xDC:
		break;
	case 0xDD:
		// No code
		break;
	case 0xDE:
		break;
	case 0xDF:
		break;
	case 0xE0:
		break;
	case 0xE1:
		break;
	case 0xE2:
		break;
	case 0xE3:
		// No code
		break;
	case 0xE4:
		// No code
		break;
	case 0xE5:
		break;
	case 0xE6:
		break;
	case 0xE7:
		break;
	case 0xE8:
		break;
	case 0xEA:
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
		break;
	case 0xEF:
		break;
	case 0xF0:
		break;
	case 0xF1:
		break;
	case 0xF2:
		break;
	case 0xF3:
		break;
	case 0xF4:
		// No code
		break;
	case 0xF5:
		break;
	case 0xF6:
		break;
	case 0xF7:
		break;
	case 0xF8:
		break;
	case 0xFA:
		break;
	case 0xFB:
		break;
	case 0xFC:
		// No code
		break;
	case 0xFD:
		// No code
		break;
	case 0xFE:
		break;
	case 0xFF:
		break;
	}
}

void opcode_execute_cb(Cpu *cpu, Instruction instruction)
{
	if (cpu->debug)
		cpu_debug_instruction(instruction);
	switch (instruction.opcode) {
	case 0x00:
		break;
	case 0x01:
		break;
	case 0x02:
		break;
	case 0x03:
		break;
	case 0x04:
		break;
	case 0x05:
		break;
	case 0x06:
		break;
	case 0x07:
		break;
	case 0x08:
		break;
	case 0x09:
		break;
	case 0x0A:
		break;
	case 0x0B:
		break;
	case 0x0C:
		break;
	case 0x0D:
		break;
	case 0x0E:
		break;
	case 0x0F:
		break;
	case 0x10:
		break;
	case 0x11:
		break;
	case 0x12:
		break;
	case 0x13:
		break;
	case 0x14:
		break;
	case 0x15:
		break;
	case 0x16:
		break;
	case 0x17:
		break;
	case 0x18:
		break;
	case 0x19:
		break;
	case 0x1A:
		break;
	case 0x1B:
		break;
	case 0x1C:
		break;
	case 0x1D:
		break;
	case 0x1E:
		break;
	case 0x1F:
		break;
	case 0x20:
		break;
	case 0x21:
		break;
	case 0x22:
		break;
	case 0x23:
		break;
	case 0x24:
		break;
	case 0x25:
		break;
	case 0x26:
		break;
	case 0x27:
		break;
	case 0x28:
		break;
	case 0x29:
		break;
	case 0x2A:
		break;
	case 0x2B:
		break;
	case 0x2C:
		break;
	case 0x2D:
		break;
	case 0x2E:
		break;
	case 0x2F:
		break;
	case 0x30:
		break;
	case 0x31:
		break;
	case 0x32:
		break;
	case 0x33:
		break;
	case 0x34:
		break;
	case 0x35:
		break;
	case 0x36:
		break;
	case 0x37:
		break;
	case 0x38:
		break;
	case 0x39:
		break;
	case 0x3A:
		break;
	case 0x3B:
		break;
	case 0x3C:
		break;
	case 0x3D:
		break;
	case 0x3E:
		break;
	case 0x3F:
		break;
	case 0x40:
		break;
	case 0x41:
		break;
	case 0x42:
		break;
	case 0x43:
		break;
	case 0x44:
		break;
	case 0x45:
		break;
	case 0x46:
		break;
	case 0x47:
		break;
	case 0x48:
		break;
	case 0x49:
		break;
	case 0x4A:
		break;
	case 0x4B:
		break;
	case 0x4C:
		break;
	case 0x4D:
		break;
	case 0x4E:
		break;
	case 0x4F:
		break;
	case 0x50:
		break;
	case 0x51:
		break;
	case 0x52:
		break;
	case 0x53:
		break;
	case 0x54:
		break;
	case 0x55:
		break;
	case 0x56:
		break;
	case 0x57:
		break;
	case 0x58:
		break;
	case 0x59:
		break;
	case 0x5A:
		break;
	case 0x5B:
		break;
	case 0x5C:
		break;
	case 0x5D:
		break;
	case 0x5E:
		break;
	case 0x5F:
		break;
	case 0x60:
		break;
	case 0x61:
		break;
	case 0x62:
		break;
	case 0x63:
		break;
	case 0x64:
		break;
	case 0x65:
		break;
	case 0x66:
		break;
	case 0x67:
		break;
	case 0x68:
		break;
	case 0x69:
		break;
	case 0x6A:
		break;
	case 0x6B:
		break;
	case 0x6C:
		break;
	case 0x6D:
		break;
	case 0x6E:
		break;
	case 0x6F:
		break;
	case 0x70:
		break;
	case 0x71:
		break;
	case 0x72:
		break;
	case 0x73:
		break;
	case 0x74:
		break;
	case 0x75:
		break;
	case 0x76:
		break;
	case 0x77:
		break;
	case 0x78:
		break;
	case 0x79:
		break;
	case 0x7A:
		break;
	case 0x7B:
		break;
	case 0x7C:
		break;
	case 0x7D:
		break;
	case 0x7E:
		break;
	case 0x7F:
		break;
	case 0x80:
		break;
	case 0x81:
		break;
	case 0x82:
		break;
	case 0x83:
		break;
	case 0x84:
		break;
	case 0x85:
		break;
	case 0x86:
		break;
	case 0x87:
		break;
	case 0x88:
		break;
	case 0x89:
		break;
	case 0x8A:
		break;
	case 0x8B:
		break;
	case 0x8C:
		break;
	case 0x8D:
		break;
	case 0x8E:
		break;
	case 0x8F:
		break;
	case 0x90:
		break;
	case 0x91:
		break;
	case 0x92:
		break;
	case 0x93:
		break;
	case 0x94:
		break;
	case 0x95:
		break;
	case 0x96:
		break;
	case 0x97:
		break;
	case 0x98:
		break;
	case 0x99:
		break;
	case 0x9A:
		break;
	case 0x9B:
		break;
	case 0x9C:
		break;
	case 0x9D:
		break;
	case 0x9E:
		break;
	case 0x9F:
		break;
	case 0xA0:
		break;
	case 0xA1:
		break;
	case 0xA2:
		break;
	case 0xA3:
		break;
	case 0xA4:
		break;
	case 0xA5:
		break;
	case 0xA6:
		break;
	case 0xA7:
		break;
	case 0xA8:
		break;
	case 0xA9:
		break;
	case 0xAA:
		break;
	case 0xAB:
		break;
	case 0xAC:
		break;
	case 0xAD:
		break;
	case 0xAE:
		break;
	case 0xAF:
		break;
	case 0xB0:
		break;
	case 0xB1:
		break;
	case 0xB2:
		break;
	case 0xB3:
		break;
	case 0xB4:
		break;
	case 0xB5:
		break;
	case 0xB6:
		break;
	case 0xB7:
		break;
	case 0xB8:
		break;
	case 0xB9:
		break;
	case 0xBA:
		break;
	case 0xBB:
		break;
	case 0xBC:
		break;
	case 0xBD:
		break;
	case 0xBE:
		break;
	case 0xBF:
		break;
	case 0xC0:
		break;
	case 0xC1:
		break;
	case 0xC2:
		break;
	case 0xC3:
		break;
	case 0xC4:
		break;
	case 0xC5:
		break;
	case 0xC6:
		break;
	case 0xC7:
		break;
	case 0xC8:
		break;
	case 0xC9:
		break;
	case 0xCA:
		break;
	case 0xCB:
		break;
	case 0xCC:
		break;
	case 0xCD:
		break;
	case 0xCE:
		break;
	case 0xCF:
		break;
	case 0xD0:
		break;
	case 0xD1:
		break;
	case 0xD2:
		break;
	case 0xD3:
		break;
	case 0xD4:
		break;
	case 0xD5:
		break;
	case 0xD6:
		break;
	case 0xD7:
		break;
	case 0xD8:
		break;
	case 0xD9:
		break;
	case 0xDA:
		break;
	case 0xDB:
		break;
	case 0xDC:
		break;
	case 0xDD:
		break;
	case 0xDE:
		break;
	case 0xDF:
		break;
	case 0xE0:
		break;
	case 0xE1:
		break;
	case 0xE2:
		break;
	case 0xE3:
		break;
	case 0xE4:
		break;
	case 0xE5:
		break;
	case 0xE6:
		break;
	case 0xE7:
		break;
	case 0xE8:
		break;
	case 0xE9:
		break;
	case 0xEA:
		break;
	case 0xEB:
		break;
	case 0xEC:
		break;
	case 0xED:
		break;
	case 0xEE:
		break;
	case 0xEF:
		break;
	case 0xF0:
		break;
	case 0xF1:
		break;
	case 0xF2:
		break;
	case 0xF3:
		break;
	case 0xF4:
		break;
	case 0xF5:
		break;
	case 0xF6:
		break;
	case 0xF7:
		break;
	case 0xF8:
		break;
	case 0xF9:
		break;
	case 0xFA:
		break;
	case 0xFB:
		break;
	case 0xFC:
		break;
	case 0xFD:
		break;
	case 0xFE:
		break;
	case 0xFF:
		break;
	}
}
