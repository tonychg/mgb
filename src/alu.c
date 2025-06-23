#include "cpu.h"
#include "opcodes.h"
#include "types.h"

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
	u8 result = MEM_READ(cpu, HL(cpu)) + 1;

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
	u8 result = MEM_READ(cpu, HL(cpu)) - 1;

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

void opcode_ld_spn(Cpu *cpu)
{
	s8 n = (s8)cpu_read_byte(cpu);
	u16 result = cpu->sp + n;

	cpu_flag_clear(cpu);
	if (((cpu->sp ^ n ^ result) & 0x100) == 0x100)
		cpu_flag_toggle(cpu, FLAG_CARRY);
	if (((cpu->sp ^ n ^ result) & 0x10) == 0x10)
		cpu_flag_toggle(cpu, FLAG_HALF);
	SET_HL(cpu, result);
}

void opcode_add_hl(Cpu *cpu, u16 word)
{
	int result = HL(cpu) + word;

	cpu_flag_set_or_clear(cpu, FLAG_ZERO);
	if (result & 0x10000) {
		cpu_flag_toggle(cpu, FLAG_CARRY);
	}
	if ((HL(cpu) ^ word ^ (result & 0xFFFF)) & 0x1000) {
		cpu_flag_toggle(cpu, FLAG_HALF);
	}
	SET_HL(cpu, (u16)result);
}

void opcode_add_sp(Cpu *cpu, s8 value)
{
	int result = cpu->sp + value;

	cpu_flag_clear(cpu);
	if (((cpu->sp ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100) {
		cpu_flag_toggle(cpu, FLAG_CARRY);
	}
	if (((cpu->sp ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10) {
		cpu_flag_toggle(cpu, FLAG_HALF);
	}
	cpu->sp = (u16)result;
}

void opcode_add(Cpu *cpu, u8 byte)
{
	int result = cpu->a + byte;
	int carrybits = cpu->a ^ byte ^ result;

	cpu->a = (u8)result;
	cpu_flag_clear(cpu);
	if ((u8)result == 0)
		cpu_flag_toggle(cpu, FLAG_ZERO);
	if ((carrybits & 0x100) != 0) {
		cpu_flag_toggle(cpu, FLAG_CARRY);
	}
	if ((carrybits & 0x10) != 0) {
		cpu_flag_toggle(cpu, FLAG_HALF);
	}
}

void opcode_adc(Cpu *cpu, u8 byte)
{
	int carry = cpu_flag_is_set(cpu, FLAG_CARRY) ? 1 : 0;
	int result = cpu->a + byte + carry;

	cpu_flag_clear(cpu);
	if ((u8)result == 0)
		cpu_flag_toggle(cpu, FLAG_ZERO);
	if (result > 0xFF) {
		cpu_flag_toggle(cpu, FLAG_CARRY);
	}
	if (((cpu->a & 0x0F) + (byte & 0x0F) + carry) > 0x0F) {
		cpu_flag_toggle(cpu, FLAG_HALF);
	}
	cpu->a = (u8)result;
}

void opcode_sub(Cpu *cpu, u8 byte)
{
	int result = cpu->a - byte;
	int carrybits = cpu->a ^ byte ^ result;

	cpu->a = (u8)result;
	cpu_flag_set(cpu, FLAG_SUBS);
	if ((u8)result == 0)
		cpu_flag_toggle(cpu, FLAG_ZERO);
	if ((carrybits & 0x100) != 0) {
		cpu_flag_toggle(cpu, FLAG_CARRY);
	}
	if ((carrybits & 0x10) != 0) {
		cpu_flag_toggle(cpu, FLAG_HALF);
	}
}

void opcode_sbc(Cpu *cpu, u8 byte)
{
	int carry = cpu_flag_is_set(cpu, FLAG_CARRY) ? 1 : 0;
	int result = cpu->a - byte - carry;

	cpu_flag_set(cpu, FLAG_SUBS);
	if ((u8)result == 0)
		cpu_flag_toggle(cpu, FLAG_ZERO);
	if (result < 0) {
		cpu_flag_toggle(cpu, FLAG_CARRY);
	}
	if (((cpu->a & 0x0F) - (byte & 0x0F) - carry) < 0) {
		cpu_flag_toggle(cpu, FLAG_HALF);
	}
	cpu->a = (u8)result;
}

void opcode_and(Cpu *cpu, u8 byte)
{
	u8 result = cpu->a & byte;

	cpu->a = result;
	cpu_flag_set(cpu, FLAG_HALF);
	if (result == 0)
		cpu_flag_toggle(cpu, FLAG_ZERO);
}

void opcode_xor(Cpu *cpu, u8 byte)
{
	u8 result = cpu->a ^ byte;

	cpu->a = result;
	cpu_flag_clear(cpu);
	if (result == 0)
		cpu_flag_toggle(cpu, FLAG_ZERO);
}

void opcode_or(Cpu *cpu, u8 byte)
{
	u8 result = cpu->a | byte;

	cpu->a = result;
	cpu_flag_clear(cpu);
	if (result == 0)
		cpu_flag_toggle(cpu, FLAG_ZERO);
}

void opcode_cp(Cpu *cpu, u8 byte)
{
	cpu_flag_set(cpu, FLAG_SUBS);
	if (cpu->a < byte) {
		cpu_flag_toggle(cpu, FLAG_CARRY);
	}
	if (cpu->a == byte) {
		cpu_flag_toggle(cpu, FLAG_ZERO);
	}
	if (((cpu->a - byte) & 0xF) > (cpu->a & 0xF)) {
		cpu_flag_toggle(cpu, FLAG_HALF);
	}
}

void opcode_stack_push(Cpu *cpu, u8 *r1, u8 *r2)
{
	cpu->sp--;
	MEM_WRITE(cpu, cpu->sp, *r1);
	cpu->sp--;
	MEM_WRITE(cpu, cpu->sp, *r2);
}

void opcode_stack_push_pc(Cpu *cpu, u16 *pc)
{
	cpu->sp--;
	MEM_WRITE(cpu, cpu->sp, opcode_get_high(cpu->pc));
	cpu->sp--;
	MEM_WRITE(cpu, cpu->sp, opcode_get_low(cpu->pc));
}

void opcode_stack_pop(Cpu *cpu, u8 *r1, u8 *r2)
{
	*r2 = MEM_READ(cpu, cpu->sp);
	cpu->sp++;
	*r1 = MEM_READ(cpu, cpu->sp);
	cpu->sp++;
}

void opcode_stack_pop_pc(Cpu *cpu, u16 *pc)
{
	u16 result;
	u8 low, high;

	low = MEM_READ(cpu, cpu->sp);
	cpu->sp++;
	high = MEM_READ(cpu, cpu->sp);
	cpu->sp++;
	result = opcode_r16_get(&high, &low);
	*pc = result;
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
