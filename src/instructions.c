#include "cpu.h"
#include "memory.h"
#include "opcodes.h"

u8 opcode_get_high(u16 word)
{
	return word >> 8;
}

u8 opcode_get_low(u16 word)
{
	return 0xFF & word;
}

u16 opcode_r16_get(u8 *r1, u8 *r2)
{
	return (u16)*r1 << 8 | *r2;
}

void opcode_r16_set(u8 *r1, u8 *r2, u16 word)
{
	*r1 = opcode_get_high(word);
	*r2 = opcode_get_low(word);
}

void opcode_r16_inc(u8 *r1, u8 *r2)
{
	u16 result = opcode_r16_get(r1, r2) + 1;
	opcode_r16_set(r1, r2, result);
}

void opcode_r16_dec(u8 *r1, u8 *r2)
{
	u16 result = opcode_r16_get(r1, r2) - 1;
	opcode_r16_set(r1, r2, result);
}

void opcode_increment(Cpu *cpu, u8 *reg)
{
	u8 result = *reg + 1;

	*reg = result;
	cpu_flag_set_or_clear(cpu, FLAG_CARRY);
	if (result == 0)
		cpu_flag_toggle(cpu, FLAG_ZERO);
	if ((result & 0x0F) == 0x00)
		cpu_flag_toggle(cpu, FLAG_HALF);
}

void opcode_inc_hl(Cpu *cpu)
{
	u8 result = HL(cpu) + 1;

	MEM_WRITE(cpu, HL(cpu), result);
	cpu_flag_set_or_clear(cpu, FLAG_CARRY);
	if (result == 0)
		cpu_flag_toggle(cpu, FLAG_ZERO);
	if ((result & 0x0F) == 0x00)
		cpu_flag_toggle(cpu, FLAG_HALF);
}

void opcode_decrement(Cpu *cpu, u8 *reg)
{
	u8 result = *reg - 1;

	*reg = result;
	cpu_flag_set_or_clear(cpu, FLAG_CARRY);
	cpu_flag_toggle(cpu, FLAG_SUBS);
	if (result == 0)
		cpu_flag_toggle(cpu, FLAG_ZERO);
	if ((result & 0x0F) == 0x0F)
		cpu_flag_toggle(cpu, FLAG_HALF);
}

void opcode_dec_hl(Cpu *cpu)
{
	u8 result = HL(cpu) - 1;

	MEM_WRITE(cpu, HL(cpu), result);
	cpu_flag_set_or_clear(cpu, FLAG_CARRY);
	cpu_flag_toggle(cpu, FLAG_SUBS);
	if (result == 0)
		cpu_flag_toggle(cpu, FLAG_ZERO);
	if ((result & 0x0F) == 0x0F)
		cpu_flag_toggle(cpu, FLAG_HALF);
}

void opcode_rlc(Cpu *cpu, u8 *reg, bool is_a)
{
	u8 result = *reg;

	if ((result & 0x80) != 0) {
		cpu_flag_set(cpu, FLAG_CARRY);
		result <<= 1;
		result |= 0x1;
	} else {
		cpu_flag_clear(cpu);
		result <<= 1;
	}
	*reg = result;
	if (!is_a && result == 0) {
		cpu_flag_toggle(cpu, FLAG_ZERO);
	}
}

void opcode_rrc(Cpu *cpu, u8 *reg, bool is_a)
{
	u8 result = *reg;

	if ((result & 0x01) != 0) {
		cpu_flag_set(cpu, FLAG_CARRY);
		result >>= 1;
		result |= 0x80;
	} else {
		cpu_flag_clear(cpu);
		result >>= 1;
	}
	*reg = result;
	if (!is_a && result == 0) {
		cpu_flag_toggle(cpu, FLAG_ZERO);
	}
}

void opcode_rl(Cpu *cpu, u8 *reg, bool is_a)
{
	u8 carry = cpu_flag_is_set(cpu, FLAG_CARRY) ? 1 : 0;
	u8 result = *reg;

	((result & 0x80) != 0) ? cpu_flag_set(cpu, FLAG_CARRY) :
				 cpu_flag_clear(cpu);
	result <<= 1;
	result |= carry;
	*reg = result;
	if (!is_a && result == 0) {
		cpu_flag_toggle(cpu, FLAG_ZERO);
	}
}

void opcode_rr(Cpu *cpu, u8 *reg, bool is_a)
{
	u8 carry = cpu_flag_is_set(cpu, FLAG_CARRY) ? 0x80 : 0x00;
	u8 result = *reg;

	((result & 0x01) != 0) ? cpu_flag_set(cpu, FLAG_CARRY) :
				 cpu_flag_clear(cpu);
	result >>= 1;
	result |= carry;
	*reg = result;
	if (!is_a && result == 0) {
		cpu_flag_toggle(cpu, FLAG_ZERO);
	}
}

void opcode_ld(u8 *reg, u8 byte)
{
	*reg = byte;
}

void opcode_ld_a16(Cpu *cpu, u8 *reg, u16 address)
{
	*reg = MEM_READ(cpu, address);
}

void opcode_ld_nn(Cpu *cpu, u16 *reg)
{
	MEM_WRITE_LE(cpu, cpu_read_word(cpu), *reg);
}

void opcode_add_hl(Cpu *cpu, u16 word)
{
	u16 result = HL(cpu) + word;

	cpu_flag_set_or_clear(cpu, FLAG_ZERO);
	if (result & 0x10000) {
		cpu_flag_toggle(cpu, FLAG_CARRY);
	}
	if ((HL(cpu) ^ word ^ (result & 0xFFFF)) & 0x1000) {
		cpu_flag_toggle(cpu, FLAG_HALF);
	}
	SET_HL(cpu, result);
}

void opcode_daa(Cpu *cpu)
{
	int a = cpu->a;

	if (!cpu_flag_is_set(cpu, FLAG_SUBS)) {
		if (cpu_flag_is_set(cpu, FLAG_HALF) || ((a & 0xF) > 9))
			a += 0x06;
		if (cpu_flag_is_set(cpu, FLAG_CARRY) || (a > 0x9F))
			a += 0x60;
	} else {
		if (cpu_flag_is_set(cpu, FLAG_HALF))
			a = (a - 6) & 0xFF;
		if (cpu_flag_is_set(cpu, FLAG_CARRY))
			a -= 0x60;
	}
	cpu_flag_untoggle(cpu, FLAG_HALF);
	cpu_flag_untoggle(cpu, FLAG_ZERO);
	if ((a & 0x100) == 0x100)
		cpu_flag_toggle(cpu, FLAG_CARRY);
	a &= 0xFF;
	if (a == 0)
		cpu_flag_toggle(cpu, FLAG_ZERO);
	cpu->a = a;
}
