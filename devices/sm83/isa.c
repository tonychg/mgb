#include "emu/memory.h"
#include "emu/sm83.h"

#define INC_AF(cpu) op_r16_inc(&cpu->a, &cpu->f)
#define INC_BC(cpu) op_r16_inc(&cpu->b, &cpu->c)
#define INC_DE(cpu) op_r16_inc(&cpu->d, &cpu->e)
#define INC_HL(cpu) op_r16_inc(&cpu->h, &cpu->l)

#define DEC_AF(cpu) op_r16_dec(&cpu->a, &cpu->f)
#define DEC_BC(cpu) op_r16_dec(&cpu->b, &cpu->c)
#define DEC_DE(cpu) op_r16_dec(&cpu->d, &cpu->e)
#define DEC_HL(cpu) op_r16_dec(&cpu->h, &cpu->l)

static void op_r16_inc(u8 *r1, u8 *r2)
{
	u16 result = unsigned_16(*r2, *r1) + 1;
	op_r16_set(r1, r2, result);
}

static void op_r16_dec(u8 *r1, u8 *r2)
{
	u16 result = unsigned_16(*r2, *r1) - 1;
	op_r16_set(r1, r2, result);
}

/*
 * Load instructions
 */
static void op_ld(struct sm83_core *cpu, u8 *reg, u16 addr)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = addr;
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		*reg = cpu->bus;
	}
}

static void op_ld_n(struct sm83_core *cpu, u8 *reg, u16 addr)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = addr;
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		*reg = cpu->bus;
		cpu->pc++;
	}
}

static void op_ld_hl_r8(struct sm83_core *cpu, u8 value)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->ptr = HL(cpu);
		cpu->bus = value;
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_ld_r8(u8 *reg, u8 value)
{
	*reg = value;
}

static void op_ld_rr_a(struct sm83_core *cpu, u8 *msb, u8 *lsb)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->ptr = unsigned_16(*lsb, *msb);
		cpu->bus = cpu->a;
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_ld_hli_a(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->ptr = HL(cpu);
		cpu->bus = cpu->a;
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
		INC_HL(cpu);
	}
}

static void op_ld_hld_a(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->ptr = HL(cpu);
		cpu->bus = cpu->a;
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
		DEC_HL(cpu);
	}
}

static void op_ldh_a_n(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = unsigned_16(cpu->bus, 0xFF);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		cpu->a = cpu->bus;
	}
}

static void op_ld_nn_sp(struct sm83_core *cpu, u16 *reg)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = cpu->pc;
		cpu->acc = cpu->bus;
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->acc |= cpu->bus << 8;
		cpu->bus = lsb(*reg);
		cpu->ptr = cpu->acc;
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_WRITE_1;
		cpu->bus = msb(*reg);
		cpu->ptr = cpu->acc + 1;
	} else if (cpu->state == SM83_CORE_WRITE_1) {
		cpu->state = SM83_CORE_FETCH;
		++cpu->pc;
	}
}

static void op_ld_r16_nn(struct sm83_core *cpu, u8 *high, u8 *low)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = cpu->pc;
		*low = cpu->bus;
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		*high = cpu->bus;
		cpu->pc++;
	}
}

static void op_ld_spn(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_IDLE_0;
		s8 n = (s8)cpu->bus;
		u16 result = cpu->sp + n;
		cpu_flag_clear(cpu);
		if (((cpu->sp ^ n ^ result) & 0x100) == 0x100)
			cpu_flag_toggle(cpu, FLAG_C);
		if (((cpu->sp ^ n ^ result) & 0x10) == 0x10)
			cpu_flag_toggle(cpu, FLAG_H);
		cpu->acc = result;
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_FETCH;
		SET_HL(cpu, cpu->acc);
	}
}

static void op_ld_sp_nn(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = cpu->pc;
		cpu->acc = cpu->bus;
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		cpu->acc |= cpu->bus << 8;
		cpu->sp = cpu->acc;
		++cpu->pc;
	}
}

static void op_ld_ffn_a(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->ptr = (u16)(0xFF00 + cpu->bus);
		cpu->bus = cpu->a;
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_ld_a_ffc(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = (u16)(0xFF00 + cpu->c);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		cpu->a = cpu->bus;
	}
}

static void op_ld_ffc_a(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->ptr = (u16)(0xFF00 + cpu->c);
		cpu->bus = cpu->a;
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_ld_nn_a(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = cpu->pc;
		cpu->acc = cpu->bus;
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->acc |= cpu->bus << 8;
		cpu->ptr = cpu->acc;
		cpu->bus = cpu->a;
		++cpu->pc;
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_ld_sp_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		cpu->sp = HL(cpu);
	}
}

static void op_ld_a_nn(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = cpu->pc;
		cpu->acc = cpu->bus;
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_READ_1;
		cpu->acc |= cpu->bus << 8;
		cpu->ptr = cpu->acc;
		++cpu->pc;
	} else if (cpu->state == SM83_CORE_READ_1) {
		cpu->state = SM83_CORE_FETCH;
		cpu->a = cpu->bus;
	}
}

static void op_ld_hl_n(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

/*
 * Jump instructions
 */
static void op_jp_n_nn(struct sm83_core *cpu, bool condition)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = cpu->pc;
		cpu->acc = cpu->bus;
	} else if (cpu->state == SM83_CORE_READ_0) {
		if (condition) {
			cpu->state = SM83_CORE_IDLE_0;
			cpu->acc |= cpu->bus << 8;
			++cpu->pc;
		} else {
			cpu->state = SM83_CORE_FETCH;
			++cpu->pc;
		}
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_FETCH;
		cpu->pc = cpu->acc;
	}
}

static void op_jr_n_e8(struct sm83_core *cpu, bool condition)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = cpu->pc;
		cpu->pc++;
	} else if (cpu->state == SM83_CORE_READ_0) {
		if (condition) {
			cpu->state = SM83_CORE_IDLE_0;
		} else {
			cpu->state = SM83_CORE_FETCH;
		}
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_FETCH;
		cpu->pc += (s8)cpu->bus;
	}
}

/*
 * IDU
 */
static void op_inc(struct sm83_core *cpu, u8 *reg)
{
	u8 result = *reg + 1;

	*reg = result;
	cpu_flag_set_or_clear(cpu, FLAG_C);
	if (result == 0)
		cpu_flag_toggle(cpu, FLAG_Z);
	if ((result & 0x0F) == 0x00)
		cpu_flag_toggle(cpu, FLAG_H);
}

static void op_inc_rr(struct sm83_core *cpu, u8 *high, u8 *low)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_IDLE_0;
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_FETCH;
		u16 result = unsigned_16(*low, *high) + 1;
		*high = msb(result);
		*low = lsb(result);
	}
}

static void op_inc_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->bus++;
		cpu->state = SM83_CORE_WRITE_0;
		cpu_flag_set_or_clear(cpu, FLAG_C);
		if (cpu->bus == 0)
			cpu_flag_toggle(cpu, FLAG_Z);
		if ((cpu->bus & 0x0F) == 0x00)
			cpu_flag_toggle(cpu, FLAG_H);
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_inc_sp(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_IDLE_0;
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_FETCH;
		cpu->sp++;
	}
}

static void op_dec(struct sm83_core *cpu, u8 *reg)
{
	u8 result = *reg - 1;

	*reg = result;
	cpu_flag_set_or_clear(cpu, FLAG_C);
	cpu_flag_toggle(cpu, FLAG_N);
	if (result == 0)
		cpu_flag_toggle(cpu, FLAG_Z);
	if ((result & 0x0F) == 0x0F)
		cpu_flag_toggle(cpu, FLAG_H);
}

static void op_dec_rr(struct sm83_core *cpu, u8 *high, u8 *low)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_IDLE_0;
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_FETCH;
		u16 result = unsigned_16(*low, *high) - 1;
		*high = msb(result);
		*low = lsb(result);
	}
}

static void op_dec_sp(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_IDLE_0;
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_FETCH;
		cpu->sp--;
	}
}

static void op_dec_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->bus--;
		cpu->state = SM83_CORE_WRITE_0;
		cpu_flag_set_or_clear(cpu, FLAG_C);
		cpu_flag_toggle(cpu, FLAG_N);
		if (cpu->bus == 0)
			cpu_flag_toggle(cpu, FLAG_Z);
		if ((cpu->bus & 0x0F) == 0x0F)
			cpu_flag_toggle(cpu, FLAG_H);
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

/*
 * Bitwise Operations
 */
static void op_rlc(struct sm83_core *cpu, u8 *reg, bool is_a)
{
	if (cpu->state == SM83_CORE_PC || cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_FETCH;
		u8 result = *reg;

		if ((result & 0x80) != 0) {
			cpu_flag_set(cpu, FLAG_C);
			result <<= 1;
			result |= 0x1;
		} else {
			cpu_flag_clear(cpu);
			result <<= 1;
		}
		*reg = result;
		if (!is_a && result == 0) {
			cpu_flag_toggle(cpu, FLAG_Z);
		}
	}
}

static void op_rlc_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_WRITE_0;
		u8 result = cpu->bus;
		if ((result & 0x80) != 0) {
			cpu_flag_set(cpu, FLAG_C);
			result <<= 1;
			result |= 0x1;
		} else {
			cpu_flag_clear(cpu);
			result <<= 1;
		}
		cpu->bus = result;
		cpu->ptr = HL(cpu);
		if (result == 0) {
			cpu_flag_toggle(cpu, FLAG_Z);
		}
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_rrc(struct sm83_core *cpu, u8 *reg, bool is_a)
{
	if (cpu->state == SM83_CORE_PC || cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_FETCH;
		u8 result = *reg;

		if ((result & 0x01) != 0) {
			cpu_flag_set(cpu, FLAG_C);
			result >>= 1;
			result |= 0x80;
		} else {
			cpu_flag_clear(cpu);
			result >>= 1;
		}
		*reg = result;
		if (!is_a && result == 0) {
			cpu_flag_toggle(cpu, FLAG_Z);
		}
	}
}

static void op_rrc_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_WRITE_0;
		if ((cpu->bus & 0x01) != 0) {
			cpu_flag_set(cpu, FLAG_C);
			cpu->bus >>= 1;
			cpu->bus |= 0x80;
		} else {
			cpu_flag_clear(cpu);
			cpu->bus >>= 1;
		}
		if (cpu->bus == 0) {
			cpu_flag_toggle(cpu, FLAG_Z);
		}
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_rl(struct sm83_core *cpu, u8 *reg, bool is_a)
{
	if (cpu->state == SM83_CORE_PC || cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_FETCH;
		u8 carry = cpu_flag_is_set(cpu, FLAG_C) ? 1 : 0;
		u8 result = *reg;

		((result & 0x80) != 0) ? cpu_flag_set(cpu, FLAG_C) :
					 cpu_flag_clear(cpu);
		result <<= 1;
		result |= carry;
		*reg = result;
		if (!is_a && result == 0) {
			cpu_flag_toggle(cpu, FLAG_Z);
		}
	}
}

static void op_rl_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		u8 carry = cpu_flag_is_set(cpu, FLAG_C) ? 1 : 0;
		cpu->state = SM83_CORE_WRITE_0;
		((cpu->bus & 0x80) != 0) ? cpu_flag_set(cpu, FLAG_C) :
					   cpu_flag_clear(cpu);
		cpu->bus <<= 1;
		cpu->bus |= carry;
		if (cpu->bus == 0) {
			cpu_flag_toggle(cpu, FLAG_Z);
		}
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_rr(struct sm83_core *cpu, u8 *reg, bool is_a)
{
	if (cpu->state == SM83_CORE_PC || cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_FETCH;
		u8 carry = cpu_flag_is_set(cpu, FLAG_C) ? 0x80 : 0x00;
		u8 result = *reg;

		((result & 0x01) != 0) ? cpu_flag_set(cpu, FLAG_C) :
					 cpu_flag_clear(cpu);
		result >>= 1;
		result |= carry;
		*reg = result;
		if (!is_a && result == 0) {
			cpu_flag_toggle(cpu, FLAG_Z);
		}
	}
}

static void op_rr_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_WRITE_0;
		u8 carry = cpu_flag_is_set(cpu, FLAG_C) ? 0x80 : 0x00;
		((cpu->bus & 0x01) != 0) ? cpu_flag_set(cpu, FLAG_C) :
					   cpu_flag_clear(cpu);
		cpu->bus >>= 1;
		cpu->bus |= carry;
		if (cpu->bus == 0) {
			cpu_flag_toggle(cpu, FLAG_Z);
		}
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_rst(struct sm83_core *cpu, u8 vec)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_IDLE_0;
		cpu->sp--;
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->ptr = cpu->sp;
		cpu->bus = msb(cpu->pc);
		cpu->sp--;
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_WRITE_1;
		cpu->ptr = cpu->sp;
		cpu->bus = lsb(cpu->pc);
		cpu->pc = unsigned_16(vec, 0x00);
	} else if (cpu->state == SM83_CORE_WRITE_1) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_swap(struct sm83_core *cpu, u8 *reg)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		u8 low_half = *reg & 0x0F;
		u8 high_half = (*reg >> 4) & 0x0F;

		*reg = (low_half << 4) + high_half;
		cpu_flag_clear(cpu);
		if (*reg == 0)
			cpu_flag_toggle(cpu, FLAG_Z);
	}
}

static void op_swap_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_WRITE_0;
		u8 low_half = cpu->bus & 0x0F;
		u8 high_half = (cpu->bus >> 4) & 0x0F;

		cpu->bus = (low_half << 4) + high_half;
		cpu_flag_clear(cpu);
		if (cpu->bus == 0)
			cpu_flag_toggle(cpu, FLAG_Z);
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_srl(struct sm83_core *cpu, u8 *reg)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		u8 result = *reg;

		if ((result & 0x01) != 0)
			cpu_flag_set(cpu, FLAG_C);
		else
			cpu_flag_clear(cpu);
		result >>= 1;
		*reg = result;
		if (result == 0)
			cpu_flag_toggle(cpu, FLAG_Z);
	}
}

static void op_srl_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_WRITE_0;
		if ((cpu->bus & 0x01) != 0)
			cpu_flag_set(cpu, FLAG_C);
		else
			cpu_flag_clear(cpu);
		cpu->bus >>= 1;
		if (cpu->bus == 0)
			cpu_flag_toggle(cpu, FLAG_Z);
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_sla(struct sm83_core *cpu, u8 *reg)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		u8 result = *reg;

		if ((result & 0x80) != 0)
			cpu_flag_set(cpu, FLAG_C);
		else
			cpu_flag_clear(cpu);
		result <<= 1;
		*reg = result;
		if (result == 0)
			cpu_flag_toggle(cpu, FLAG_Z);
	}
}

static void op_sla_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_WRITE_0;
		if ((cpu->bus & 0x80) != 0)
			cpu_flag_set(cpu, FLAG_C);
		else
			cpu_flag_clear(cpu);
		cpu->bus <<= 1;
		if (cpu->bus == 0)
			cpu_flag_toggle(cpu, FLAG_Z);
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_sra(struct sm83_core *cpu, u8 *reg)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		u8 result = *reg;

		if ((result & 0x01) != 0)
			cpu_flag_set(cpu, FLAG_C);
		else
			cpu_flag_clear(cpu);
		if ((result & 0x80) != 0) {
			result >>= 1;
			result |= 0x80;
		} else {
			result >>= 1;
		}
		*reg = result;
		if (result == 0)
			cpu_flag_toggle(cpu, FLAG_Z);
	}
}

static void op_sra_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_WRITE_0;
		int result = cpu->bus;
		if ((result & 0x01) != 0)
			cpu_flag_set(cpu, FLAG_C);
		else
			cpu_flag_clear(cpu);
		if ((result & 0x80) != 0) {
			result >>= 1;
			result |= 0x80;
		} else {
			result >>= 1;
		}
		if (result == 0)
			cpu_flag_toggle(cpu, FLAG_Z);
		cpu->bus = result;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_and(struct sm83_core *cpu, u8 byte)
{
	u8 result = cpu->a & byte;

	cpu->a = result;
	cpu_flag_set(cpu, FLAG_H);
	if (result == 0)
		cpu_flag_toggle(cpu, FLAG_Z);
}

static void op_and_n(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		op_and(cpu, cpu->bus);
	}
}

static void op_and_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		op_and(cpu, cpu->bus);
	}
}

static void op_xor(struct sm83_core *cpu, u8 byte)
{
	u8 result = cpu->a ^ byte;

	cpu->a = result;
	cpu_flag_clear(cpu);
	if (result == 0)
		cpu_flag_toggle(cpu, FLAG_Z);
}

static void op_xor_n(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		op_xor(cpu, cpu->bus);
	}
}

static void op_xor_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		op_xor(cpu, cpu->bus);
	}
}

static void op_or(struct sm83_core *cpu, u8 byte)
{
	u8 result = cpu->a | byte;

	cpu->a = result;
	cpu_flag_clear(cpu);
	if (result == 0)
		cpu_flag_toggle(cpu, FLAG_Z);
}

static void op_or_n(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		op_or(cpu, cpu->bus);
	}
}

static void op_or_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		op_or(cpu, cpu->bus);
	}
}

static void op_bit(struct sm83_core *cpu, u8 *reg, int bit)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		if (((*reg >> bit) & 0x01) == 0)
			cpu_flag_toggle(cpu, FLAG_Z);
		else
			cpu_flag_untoggle(cpu, FLAG_Z);
		cpu_flag_toggle(cpu, FLAG_H);
		cpu_flag_untoggle(cpu, FLAG_N);
	}
}

static void op_bit_hl(struct sm83_core *cpu, int bit)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		if (((cpu->bus >> bit) & 0x01) == 0)
			cpu_flag_toggle(cpu, FLAG_Z);
		else
			cpu_flag_untoggle(cpu, FLAG_Z);
		cpu_flag_toggle(cpu, FLAG_H);
		cpu_flag_untoggle(cpu, FLAG_N);
	}
}

static void op_set(struct sm83_core *cpu, u8 *reg, int bit)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		*reg = (*reg | (0x01 << bit));
	}
}

static void op_set_hl(struct sm83_core *cpu, int bit)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->bus |= (0x1 << bit);
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_res(struct sm83_core *cpu, u8 *reg, int bit)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		*reg = (*reg & (~(0x1 << bit)));
	}
}

static void op_res_hl(struct sm83_core *cpu, int bit)
{
	if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->bus &= (~(0x1 << bit));
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

/*
 * Arithmetic operations
 */
static void op_add_hl(struct sm83_core *cpu, u16 word)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_IDLE_0;
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_FETCH;
		int result = HL(cpu) + word;
		cpu_flag_set_or_clear(cpu, FLAG_Z);
		if (result & 0x10000) {
			cpu_flag_toggle(cpu, FLAG_C);
		}
		if ((HL(cpu) ^ word ^ (result & 0xFFFF)) & 0x1000) {
			cpu_flag_toggle(cpu, FLAG_H);
		}
		SET_HL(cpu, (u16)result);
	}
}

static void op_add_sp(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_IDLE_0;
		cpu->acc = cpu->sp + (s8)cpu->bus;
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_IDLE_1;
		s8 value = (s8)cpu->bus;
		int result = cpu->sp + value;
		cpu_flag_clear(cpu);
		if (((cpu->sp ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100) {
			cpu_flag_toggle(cpu, FLAG_C);
		}
		if (((cpu->sp ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10) {
			cpu_flag_toggle(cpu, FLAG_H);
		}
		cpu->acc = (u16)result;
	} else if (cpu->state == SM83_CORE_IDLE_1) {
		cpu->state = SM83_CORE_FETCH;
		cpu->sp = cpu->acc;
	}
}

static void op_add(struct sm83_core *cpu, u8 byte)
{
	int result = cpu->a + byte;
	int carrybits = cpu->a ^ byte ^ result;

	cpu->a = (u8)result;
	cpu_flag_clear(cpu);
	if ((u8)result == 0)
		cpu_flag_toggle(cpu, FLAG_Z);
	if ((carrybits & 0x100) != 0) {
		cpu_flag_toggle(cpu, FLAG_C);
	}
	if ((carrybits & 0x10) != 0) {
		cpu_flag_toggle(cpu, FLAG_H);
	}
}

static void op_add_a_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		op_add(cpu, cpu->bus);
	}
}

static void op_add_n(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		op_add(cpu, cpu->bus);
	}
}

static void op_adc(struct sm83_core *cpu, u8 byte)
{
	int carry = cpu_flag_is_set(cpu, FLAG_C) ? 1 : 0;
	int result = cpu->a + byte + carry;

	cpu_flag_clear(cpu);
	if ((u8)result == 0)
		cpu_flag_toggle(cpu, FLAG_Z);
	if (result > 0xFF) {
		cpu_flag_toggle(cpu, FLAG_C);
	}
	if (((cpu->a & 0x0F) + (byte & 0x0F) + carry) > 0x0F) {
		cpu_flag_toggle(cpu, FLAG_H);
	}
	cpu->a = (u8)result;
}

static void op_adc_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		op_adc(cpu, cpu->bus);
	}
}

static void op_adc_n(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		op_adc(cpu, cpu->bus);
	}
}

static void op_sub(struct sm83_core *cpu, u8 byte)
{
	int result = cpu->a - byte;
	int carrybits = cpu->a ^ byte ^ result;

	cpu->a = (u8)result;
	cpu_flag_set(cpu, FLAG_N);
	if ((u8)result == 0)
		cpu_flag_toggle(cpu, FLAG_Z);
	if ((carrybits & 0x100) != 0) {
		cpu_flag_toggle(cpu, FLAG_C);
	}
	if ((carrybits & 0x10) != 0) {
		cpu_flag_toggle(cpu, FLAG_H);
	}
}

static void op_sub_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		op_sub(cpu, cpu->bus);
	}
}

static void op_sub_n(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		op_sub(cpu, cpu->bus);
	}
}

static void op_sbc(struct sm83_core *cpu, u8 byte)
{
	int carry = cpu_flag_is_set(cpu, FLAG_C) ? 1 : 0;
	int result = cpu->a - byte - carry;

	cpu_flag_set(cpu, FLAG_N);
	if ((u8)result == 0)
		cpu_flag_toggle(cpu, FLAG_Z);
	if (result < 0) {
		cpu_flag_toggle(cpu, FLAG_C);
	}
	if (((cpu->a & 0x0F) - (byte & 0x0F) - carry) < 0) {
		cpu_flag_toggle(cpu, FLAG_H);
	}
	cpu->a = (u8)result;
}

static void op_sbc_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		op_sbc(cpu, cpu->bus);
	}
}

static void op_sbc_n(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		op_sbc(cpu, cpu->bus);
	}
}

/*
 * Stack operations
 */
static void op_call_nn(struct sm83_core *cpu, bool condition)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_READ_0;
		cpu->acc = cpu->bus;
		cpu->ptr = cpu->pc;
	} else if (cpu->state == SM83_CORE_READ_0) {
		if (condition) {
			cpu->state = SM83_CORE_WRITE_0;
			cpu->acc |= cpu->bus << 8;
			cpu->ptr = --cpu->sp;
			cpu->bus = msb(cpu->pc + 1);
		} else {
			cpu->state = SM83_CORE_FETCH;
			cpu->pc++;
		}
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_WRITE_1;
		cpu->ptr = --cpu->sp;
		cpu->bus = lsb(cpu->pc + 1);
	} else if (cpu->state == SM83_CORE_WRITE_1) {
		cpu->state = SM83_CORE_IDLE_0;
		cpu->pc = cpu->acc;
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_FETCH;
	}
}

static void op_push_rr(struct sm83_core *cpu, u8 *msb, u8 *lsb)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_IDLE_0;
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_WRITE_0;
		cpu->ptr = --cpu->sp;
		cpu->bus = *msb;
	} else if (cpu->state == SM83_CORE_WRITE_0) {
		cpu->state = SM83_CORE_WRITE_1;
		cpu->ptr = --cpu->sp;
		cpu->bus = *lsb;
	} else if (cpu->state == SM83_CORE_WRITE_1) {
		cpu->state = SM83_CORE_FETCH;
		cpu->ptr = cpu->sp;
	}
}

static void op_ret(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = cpu->sp++;
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_READ_1;
		cpu->acc = cpu->bus;
		cpu->ptr = cpu->sp++;
		cpu->pc++;
	} else if (cpu->state == SM83_CORE_READ_1) {
		cpu->state = SM83_CORE_IDLE_0;
		cpu->acc |= cpu->bus << 8;
		cpu->pc++;
	} else if (cpu->state == SM83_CORE_IDLE_0) {
		cpu->state = SM83_CORE_FETCH;
		cpu->pc = cpu->acc;
	}
}

static void op_ret_n(struct sm83_core *cpu, bool condition)
{
	if (!condition) {
		if (cpu->state == SM83_CORE_FETCH) {
			cpu->state = SM83_CORE_IDLE_0;
		} else if (cpu->state == SM83_CORE_IDLE_0) {
			cpu->state = SM83_CORE_FETCH;
		}
	} else {
		if (cpu->state == SM83_CORE_FETCH) {
			cpu->state = SM83_CORE_READ_0;
			cpu->ptr = cpu->sp++;
		} else if (cpu->state == SM83_CORE_READ_0) {
			cpu->state = SM83_CORE_READ_1;
			cpu->acc = cpu->bus;
			cpu->ptr = cpu->sp++;
			cpu->pc++;
		} else if (cpu->state == SM83_CORE_READ_1) {
			cpu->state = SM83_CORE_IDLE_0;
			cpu->acc |= cpu->bus << 8;
			cpu->pc++;
		} else if (cpu->state == SM83_CORE_IDLE_0) {
			cpu->state = SM83_CORE_IDLE_1;
		} else if (cpu->state == SM83_CORE_IDLE_1) {
			cpu->state = SM83_CORE_FETCH;
			cpu->pc = cpu->acc;
		}
	}
}

static void op_pop(struct sm83_core *cpu, u8 *r1, u8 *r2)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = cpu->sp;
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_READ_1;
		*r2 = cpu->bus;
		cpu->ptr = ++cpu->sp;
	} else if (cpu->state == SM83_CORE_READ_1) {
		cpu->state = SM83_CORE_FETCH;
		*r1 = cpu->bus;
		cpu->sp++;
	}
}

/*
 * Special register operations
 */
static void op_cp(struct sm83_core *cpu, u8 byte)
{
	cpu_flag_set(cpu, FLAG_N);
	if (cpu->a < byte) {
		cpu_flag_toggle(cpu, FLAG_C);
	}
	if (cpu->a == byte) {
		cpu_flag_toggle(cpu, FLAG_Z);
	}
	if (((cpu->a - byte) & 0xF) > (cpu->a & 0xF)) {
		cpu_flag_toggle(cpu, FLAG_H);
	}
}

static void op_cp_n(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_PC;
	} else if (cpu->state == SM83_CORE_PC) {
		cpu->state = SM83_CORE_FETCH;
		op_cp(cpu, cpu->bus);
	}
}

static void op_cp_a_hl(struct sm83_core *cpu)
{
	if (cpu->state == SM83_CORE_FETCH) {
		cpu->state = SM83_CORE_READ_0;
		cpu->ptr = HL(cpu);
	} else if (cpu->state == SM83_CORE_READ_0) {
		cpu->state = SM83_CORE_FETCH;
		op_cp(cpu, cpu->bus);
	}
}

static void op_daa(struct sm83_core *cpu)
{
	int a = cpu->a;

	if (!cpu_flag_is_set(cpu, FLAG_N)) {
		if (cpu_flag_is_set(cpu, FLAG_H) || ((a & 0xF) > 9))
			a += 0x06;
		if (cpu_flag_is_set(cpu, FLAG_C) || (a > 0x9F))
			a += 0x60;
	} else {
		if (cpu_flag_is_set(cpu, FLAG_H))
			a = (a - 6) & 0xFF;
		if (cpu_flag_is_set(cpu, FLAG_C))
			a -= 0x60;
	}
	cpu_flag_untoggle(cpu, FLAG_H);
	cpu_flag_untoggle(cpu, FLAG_Z);
	if ((a & 0x100) == 0x100)
		cpu_flag_toggle(cpu, FLAG_C);
	a &= 0xFF;
	if (a == 0)
		cpu_flag_toggle(cpu, FLAG_Z);
	cpu->a = a;
}

static void sm83_isa_execute_non_prefixed(struct sm83_core *cpu)
{
	u8 opcode;

	opcode = cpu->instruction.opcode;
	switch (opcode) {
	case 0x00:
		// NOOP
		break;
	case 0x01:
		op_ld_r16_nn(cpu, &cpu->b, &cpu->c);
		break;
	case 0x02:
		// LD (BC),a
		op_ld_rr_a(cpu, &cpu->b, &cpu->c);
		break;
	case 0x03:
		// INC BC
		op_inc_rr(cpu, &cpu->b, &cpu->c);
		break;
	case 0x04:
		// Z 0 H -
		op_inc(cpu, &cpu->b);
		break;
	case 0x05:
		// DEC B
		// Z 1 H -
		op_dec(cpu, &cpu->b);
		break;
	case 0x06:
		// LD B,n
		op_ld_n(cpu, &cpu->b, cpu->pc);
		break;
	case 0x07:
		// RLCA
		op_rlc(cpu, &cpu->a, true);
		break;
	case 0x08:
		// LD (nn),SP
		op_ld_nn_sp(cpu, &cpu->sp);
		break;
	case 0x09:
		// ADD HL, BC
		// - 0 H C
		op_add_hl(cpu, BC(cpu));
		break;
	case 0x0A:
		// LD a,(BC)
		op_ld(cpu, &cpu->a, BC(cpu));
		break;
	case 0x0B:
		// DEC BC
		op_dec_rr(cpu, &cpu->b, &cpu->c);
		break;
	case 0x0C:
		// Z 0 H -
		op_inc(cpu, &cpu->c);
		break;
	case 0x0D:
		// Z 1 H -
		op_dec(cpu, &cpu->c);
		break;
	case 0x0E:
		// LD c,n
		op_ld_n(cpu, &cpu->c, cpu->pc);
		break;
	case 0x0F:
		// RRCA
		// 0 0 0 C
		op_rrc(cpu, &cpu->a, true);
		break;
	case 0x10:
		// STOP n8
		cpu->memory->write8(cpu, DIV, 0);
		break;
	case 0x11:
		// LD DE,nn
		op_ld_r16_nn(cpu, &cpu->d, &cpu->e);
		break;
	case 0x12:
		// LD (DE),a
		op_ld_rr_a(cpu, &cpu->d, &cpu->e);
		break;
	case 0x13:
		// INC de
		op_inc_rr(cpu, &cpu->d, &cpu->e);
		break;
	case 0x14:
		// Z 0 H -
		op_inc(cpu, &cpu->d);
		break;
	case 0x15:
		// Z 1 H -
		op_dec(cpu, &cpu->d);
		break;
	case 0x16:
		// LD D,n
		op_ld_n(cpu, &cpu->d, cpu->pc);
		break;
	case 0x17:
		// RLA
		// 0 0 0 C
		op_rl(cpu, &cpu->a, true);
		break;
	case 0x18:
		// JR e8
		op_jr_n_e8(cpu, true);
		break;
	case 0x19:
		// ADD HL, DE
		// - 0 H C
		op_add_hl(cpu, DE(cpu));
		break;
	case 0x1A:
		// LD,A,(DE)
		op_ld(cpu, &cpu->a, DE(cpu));
		break;
	case 0x1B:
		// DEC DE
		op_dec_rr(cpu, &cpu->d, &cpu->e);
		break;
	case 0x1C:
		// Z 0 H -
		op_inc(cpu, &cpu->e);
		break;
	case 0x1D:
		// Z 1 H -
		op_dec(cpu, &cpu->e);
		break;
	case 0x1E:
		// LD E,n
		op_ld_n(cpu, &cpu->e, cpu->pc);
		break;
	case 0x1F:
		// RRA
		// 0 0 0 C
		op_rr(cpu, &cpu->a, true);
		break;
	case 0x20:
		// JR NZ,e8
		op_jr_n_e8(cpu, !cpu_flag_is_set(cpu, FLAG_Z));
		break;
	case 0x21:
		// LD HL,nn
		op_ld_r16_nn(cpu, &cpu->h, &cpu->l);
		break;
	case 0x22:
		// LD [HL+], A
		op_ld_hli_a(cpu);
		break;
	case 0x23:
		// INC HL
		op_inc_rr(cpu, &cpu->h, &cpu->l);
		break;
	case 0x24:
		// INC h
		// Z 0 H -
		op_inc(cpu, &cpu->h);
		break;
	case 0x25:
		// DEC h
		// Z 1 H -
		op_dec(cpu, &cpu->h);
		break;
	case 0x26:
		// LD h,n
		op_ld_n(cpu, &cpu->h, cpu->pc);
		break;
	case 0x27:
		// DAA
		// Z - 0 C
		op_daa(cpu);
		break;
	case 0x28:
		// JR Z,e8
		op_jr_n_e8(cpu, cpu_flag_is_set(cpu, FLAG_Z));
		break;
	case 0x29:
		// ADD HL,HL
		// - 0 H C
		op_add_hl(cpu, HL(cpu));
		break;
	case 0x2A:
		// LD A,[HL+]
		op_ld(cpu, &cpu->a, HL(cpu));
		if (cpu->state == SM83_CORE_READ_0)
			INC_HL(cpu);
		break;
	case 0x2B:
		// DEC HL
		op_dec_rr(cpu, &cpu->h, &cpu->l);
		break;
	case 0x2C:
		// INC l
		// Z 0 H -
		op_inc(cpu, &cpu->l);
		break;
	case 0x2D:
		// DEC l
		// Z 1 H -
		op_dec(cpu, &cpu->l);
		break;
	case 0x2E:
		// LD L,n
		op_ld_n(cpu, &cpu->l, cpu->pc);
		break;
	case 0x2F:
		// CPL
		// - 1 1 -
		cpu->a = ~cpu->a;
		cpu_flag_toggle(cpu, FLAG_H);
		cpu_flag_toggle(cpu, FLAG_N);
		break;
	case 0x30:
		// JR NC,e8
		op_jr_n_e8(cpu, !cpu_flag_is_set(cpu, FLAG_C));
		break;
	case 0x31:
		// LD sp,nn
		op_ld_sp_nn(cpu);
		break;
	case 0x32:
		// LD (HLD), A
		op_ld_hld_a(cpu);
		break;
	case 0x33:
		// INC sp
		op_inc_sp(cpu);
		break;
	case 0x34:
		// INC (HL)
		op_inc_hl(cpu);
		break;
	case 0x35:
		// DEC (HL)
		op_dec_hl(cpu);
		break;
	case 0x36:
		// LD (HL),n
		op_ld_hl_n(cpu);
		break;
	case 0x37:
		// SCF
		cpu_flag_toggle(cpu, FLAG_C);
		cpu_flag_untoggle(cpu, FLAG_H);
		cpu_flag_untoggle(cpu, FLAG_N);
		break;
	case 0x38:
		// JR C,n
		op_jr_n_e8(cpu, cpu_flag_is_set(cpu, FLAG_C));
		break;
	case 0x39:
		// ADD HL,SP
		op_add_hl(cpu, cpu->sp);
		break;
	case 0x3A:
		// LD A,(HLD)
		op_ld(cpu, &cpu->a, HL(cpu));
		if (cpu->state == SM83_CORE_READ_0)
			DEC_HL(cpu);
		break;
	case 0x3B:
		// DEC SP
		op_dec_sp(cpu);
		break;
	case 0x3C:
		// INC A
		op_inc(cpu, &cpu->a);
		break;
	case 0x3D:
		// DEC A
		op_dec(cpu, &cpu->a);
		break;
	case 0x3E:
		// LD A,n
		op_ld_n(cpu, &cpu->a, cpu->pc);
		break;
	case 0x3F:
		// CCF
		cpu_flag_flip(cpu, FLAG_C);
		cpu_flag_untoggle(cpu, FLAG_H);
		cpu_flag_untoggle(cpu, FLAG_N);
		break;
	case 0x40:
		// LD B,B
		op_ld_r8(&cpu->b, cpu->b);
		break;
	case 0x41:
		// LD B,C
		op_ld_r8(&cpu->b, cpu->c);
		break;
	case 0x42:
		// LD B,D
		op_ld_r8(&cpu->b, cpu->d);
		break;
	case 0x43:
		// LD B,E
		op_ld_r8(&cpu->b, cpu->e);
		break;
	case 0x44:
		// LD B,H
		op_ld_r8(&cpu->b, cpu->h);
		break;
	case 0x45:
		// LD B,L
		op_ld_r8(&cpu->b, cpu->l);
		break;
	case 0x46:
		// LD B,[HL]
		op_ld(cpu, &cpu->b, HL(cpu));
		break;
	case 0x47:
		// LD B,A
		op_ld_r8(&cpu->b, cpu->a);
		break;
	case 0x48:
		// LD C,B
		op_ld_r8(&cpu->c, cpu->b);
		break;
	case 0x49:
		// LD C,C
		op_ld_r8(&cpu->c, cpu->c);
		break;
	case 0x4A:
		// LD C,D
		op_ld_r8(&cpu->c, cpu->d);
		break;
	case 0x4B:
		// LD C,E
		op_ld_r8(&cpu->c, cpu->e);
		break;
	case 0x4C:
		// LD C,H
		op_ld_r8(&cpu->c, cpu->h);
		break;
	case 0x4D:
		// LD C,L
		op_ld_r8(&cpu->c, cpu->l);
		break;
	case 0x4E:
		// LD C,[HL]
		op_ld(cpu, &cpu->c, HL(cpu));
		break;
	case 0x4F:
		// LD C,A
		op_ld_r8(&cpu->c, cpu->a);
		break;
	case 0x50:
		// LD D,B
		op_ld_r8(&cpu->d, cpu->b);
		break;
	case 0x51:
		// LD D,C
		op_ld_r8(&cpu->d, cpu->c);
		break;
	case 0x52:
		// LD D,D
		op_ld_r8(&cpu->d, cpu->d);
		break;
	case 0x53:
		// LD D,E
		op_ld_r8(&cpu->d, cpu->e);
		break;
	case 0x54:
		// LD D,H
		op_ld_r8(&cpu->d, cpu->h);
		break;
	case 0x55:
		// LD D,L
		op_ld_r8(&cpu->d, cpu->l);
		break;
	case 0x56:
		// LD D,[HL]
		op_ld(cpu, &cpu->d, HL(cpu));
		break;
	case 0x57:
		// LD D,A
		op_ld_r8(&cpu->d, cpu->a);
		break;
	case 0x58:
		// LD E,B
		op_ld_r8(&cpu->e, cpu->b);
		break;
	case 0x59:
		// LD E,C
		op_ld_r8(&cpu->e, cpu->c);
		break;
	case 0x5A:
		// LD E,D
		op_ld_r8(&cpu->e, cpu->d);
		break;
	case 0x5B:
		// LD E,E
		op_ld_r8(&cpu->e, cpu->e);
		break;
	case 0x5C:
		// LD E,H
		op_ld_r8(&cpu->e, cpu->h);
		break;
	case 0x5D:
		// LD E,L
		op_ld_r8(&cpu->e, cpu->l);
		break;
	case 0x5E:
		// LD E,[HL]
		op_ld(cpu, &cpu->e, HL(cpu));
		break;
	case 0x5F:
		// LD E,A
		op_ld_r8(&cpu->e, cpu->a);
		break;
	case 0x60:
		// LD H,B
		op_ld_r8(&cpu->h, cpu->b);
		break;
	case 0x61:
		// LD H,C
		op_ld_r8(&cpu->h, cpu->c);
		break;
	case 0x62:
		// LD H,D
		op_ld_r8(&cpu->h, cpu->d);
		break;
	case 0x63:
		// LD H,E
		op_ld_r8(&cpu->h, cpu->e);
		break;
	case 0x64:
		// LD H,H
		op_ld_r8(&cpu->h, cpu->h);
		break;
	case 0x65:
		// LD H,L
		op_ld_r8(&cpu->h, cpu->l);
		break;
	case 0x66:
		// LD H,[HL]
		op_ld(cpu, &cpu->h, HL(cpu));
		break;
	case 0x67:
		// LD H,A
		op_ld_r8(&cpu->h, cpu->a);
		break;
	case 0x68:
		// LD L,B
		op_ld_r8(&cpu->l, cpu->b);
		break;
	case 0x69:
		// LD L,C
		op_ld_r8(&cpu->l, cpu->c);
		break;
	case 0x6A:
		// LD L,D
		op_ld_r8(&cpu->l, cpu->d);
		break;
	case 0x6B:
		// LD L,E
		op_ld_r8(&cpu->l, cpu->e);
		break;
	case 0x6C:
		// LD L,H
		op_ld_r8(&cpu->l, cpu->h);
		break;
	case 0x6D:
		// LD L,L
		op_ld_r8(&cpu->l, cpu->l);
		break;
	case 0x6E:
		// LD L,[HL]
		op_ld(cpu, &cpu->l, HL(cpu));
		break;
	case 0x6F:
		// LD L,A
		op_ld_r8(&cpu->l, cpu->a);
		break;
	case 0x70:
		// LD [HL],B
		op_ld_hl_r8(cpu, cpu->b);
		break;
	case 0x71:
		// LD [HL],C
		op_ld_hl_r8(cpu, cpu->c);
		break;
	case 0x72:
		// LD [HL],D
		op_ld_hl_r8(cpu, cpu->d);
		break;
	case 0x73:
		// LD [HL],E
		op_ld_hl_r8(cpu, cpu->e);
		break;
	case 0x74:
		// LD [HL],H
		op_ld_hl_r8(cpu, cpu->h);
		break;
	case 0x75:
		// LD [HL],L
		op_ld_hl_r8(cpu, cpu->l);
		break;
	case 0x76:
		// HALT
		cpu->previous = cpu->state;
		cpu->cycles -= cpu->multiplier;
		sm83_halt(cpu);
		break;
	case 0x77:
		// LD [HL],A
		op_ld_hl_r8(cpu, cpu->a);
		break;
	case 0x78:
		// LD A,B
		op_ld_r8(&cpu->a, cpu->b);
		break;
	case 0x79:
		// LD A,C
		op_ld_r8(&cpu->a, cpu->c);
		break;
	case 0x7A:
		// LD A,D
		op_ld_r8(&cpu->a, cpu->d);
		break;
	case 0x7B:
		// LD A,E
		op_ld_r8(&cpu->a, cpu->e);
		break;
	case 0x7C:
		// LD A,H
		op_ld_r8(&cpu->a, cpu->h);
		break;
	case 0x7D:
		// LD A,L
		op_ld_r8(&cpu->a, cpu->l);
		break;
	case 0x7E:
		// LD A,[HL]
		op_ld(cpu, &cpu->a, HL(cpu));
		break;
	case 0x7F:
		// LD A,A
		op_ld_r8(&cpu->a, cpu->a);
		break;
	case 0x80:
		// ADD A,B
		op_add(cpu, cpu->b);
		break;
	case 0x81:
		// ADD A,C
		op_add(cpu, cpu->c);
		break;
	case 0x82:
		// ADD A,D
		op_add(cpu, cpu->d);
		break;
	case 0x83:
		// ADD A,E
		op_add(cpu, cpu->e);
		break;
	case 0x84:
		// ADD A,H
		op_add(cpu, cpu->h);
		break;
	case 0x85:
		// ADD A,L
		op_add(cpu, cpu->l);
		break;
	case 0x86:
		// ADD A,HL
		op_add_a_hl(cpu);
		break;
	case 0x87:
		// ADD A,A
		op_add(cpu, cpu->a);
		break;
	case 0x88:
		// ADC A,B
		op_adc(cpu, cpu->b);
		break;
	case 0x89:
		// ADC A,C
		op_adc(cpu, cpu->c);
		break;
	case 0x8A:
		// ADC A,D
		op_adc(cpu, cpu->d);
		break;
	case 0x8B:
		// ADC A,E
		op_adc(cpu, cpu->e);
		break;
	case 0x8C:
		// ADC A,H
		op_adc(cpu, cpu->h);
		break;
	case 0x8D:
		// ADC A,L
		op_adc(cpu, cpu->l);
		break;
	case 0x8E:
		// ADC A,[HL]
		op_adc_hl(cpu);
		break;
	case 0x8F:
		// ADC A,A
		op_adc(cpu, cpu->a);
		break;
	case 0x90:
		// SUB A,B
		op_sub(cpu, cpu->b);
		break;
	case 0x91:
		// SUB A,C
		op_sub(cpu, cpu->c);
		break;
	case 0x92:
		// SUB A,D
		op_sub(cpu, cpu->d);
		break;
	case 0x93:
		// SUB A,E
		op_sub(cpu, cpu->e);
		break;
	case 0x94:
		// SUB A,H
		op_sub(cpu, cpu->h);
		break;
	case 0x95:
		// SUB A,L
		op_sub(cpu, cpu->l);
		break;
	case 0x96:
		// SUB A,[HL]
		op_sub_hl(cpu);
		break;
	case 0x97:
		// SUB A,A
		op_sub(cpu, cpu->a);
		break;
	case 0x98:
		// SBC A,B
		op_sbc(cpu, cpu->b);
		break;
	case 0x99:
		// SBC A,C
		op_sbc(cpu, cpu->c);
		break;
	case 0x9A:
		// SBC A,D
		op_sbc(cpu, cpu->d);
		break;
	case 0x9B:
		// SBC A,E
		op_sbc(cpu, cpu->e);
		break;
	case 0x9C:
		// SBC A,H
		op_sbc(cpu, cpu->h);
		break;
	case 0x9D:
		// SBC A,L
		op_sbc(cpu, cpu->l);
		break;
	case 0x9E:
		// SBC A,[HL]
		op_sbc_hl(cpu);
		break;
	case 0x9F:
		// SBC A,A
		op_sbc(cpu, cpu->a);
		break;
	case 0xA0:
		// AND A,B
		op_and(cpu, cpu->b);
		break;
	case 0xA1:
		// AND A,C
		op_and(cpu, cpu->c);
		break;
	case 0xA2:
		// AND A,D
		op_and(cpu, cpu->d);
		break;
	case 0xA3:
		// AND A,E
		op_and(cpu, cpu->e);
		break;
	case 0xA4:
		// AND A,H
		op_and(cpu, cpu->h);
		break;
	case 0xA5:
		// AND A,L
		op_and(cpu, cpu->l);
		break;
	case 0xA6:
		// AND A,[HL]
		op_and_hl(cpu);
		break;
	case 0xA7:
		// AND A,A
		op_and(cpu, cpu->a);
		break;
	case 0xA8:
		// XOR A,B
		op_xor(cpu, cpu->b);
		break;
	case 0xA9:
		// XOR A,C
		op_xor(cpu, cpu->c);
		break;
	case 0xAA:
		// XOR A,D
		op_xor(cpu, cpu->d);
		break;
	case 0xAB:
		// XOR A,E
		op_xor(cpu, cpu->e);
		break;
	case 0xAC:
		// XOR A,H
		op_xor(cpu, cpu->h);
		break;
	case 0xAD:
		// XOR A,L
		op_xor(cpu, cpu->l);
		break;
	case 0xAE:
		// XOR A,[HL]
		op_xor_hl(cpu);
		break;
	case 0xAF:
		// XOR A,A
		op_xor(cpu, cpu->a);
		break;
	case 0xB0:
		// OR A,B
		op_or(cpu, cpu->b);
		break;
	case 0xB1:
		// OR A,C
		op_or(cpu, cpu->c);
		break;
	case 0xB2:
		// OR A,D
		op_or(cpu, cpu->d);
		break;
	case 0xB3:
		// OR A,E
		op_or(cpu, cpu->e);
		break;
	case 0xB4:
		// OR A,H
		op_or(cpu, cpu->h);
		break;
	case 0xB5:
		// OR A,L
		op_or(cpu, cpu->l);
		break;
	case 0xB6:
		// OR A,[HL]
		op_or_hl(cpu);
		break;
	case 0xB7:
		// OR A,A
		op_or(cpu, cpu->a);
		break;
	case 0xB8:
		// CP A,B
		op_cp(cpu, cpu->b);
		break;
	case 0xB9:
		// CP A,C
		op_cp(cpu, cpu->c);
		break;
	case 0xBA:
		// CP A,D
		op_cp(cpu, cpu->d);
		break;
	case 0xBB:
		// CP A,E
		op_cp(cpu, cpu->e);
		break;
	case 0xBC:
		// CP A,H
		op_cp(cpu, cpu->h);
		break;
	case 0xBD:
		// CP A,L
		op_cp(cpu, cpu->l);
		break;
	case 0xBE:
		// CP A,[HL]
		op_cp_a_hl(cpu);
		break;
	case 0xBF:
		// CP A,A
		cpu_flag_clear(cpu);
		cpu_flag_toggle(cpu, FLAG_Z);
		cpu_flag_toggle(cpu, FLAG_N);
		break;
	case 0xC0:
		// RET NZ
		op_ret_n(cpu, !cpu_flag_is_set(cpu, FLAG_Z));
		break;
	case 0xC1:
		// POP BC
		op_pop(cpu, &cpu->b, &cpu->c);
		break;
	case 0xC2:
		// JP NZ,nn
		op_jp_n_nn(cpu, !cpu_flag_is_set(cpu, FLAG_Z));
		break;
	case 0xC3:
		// JP nn
		op_jp_n_nn(cpu, true);
		break;
	case 0xC4:
		// CALL NZ,nn
		op_call_nn(cpu, !cpu_flag_is_set(cpu, FLAG_Z));
		break;
	case 0xC5:
		// PUSH BC
		op_push_rr(cpu, &cpu->b, &cpu->c);
		break;
	case 0xC6:
		// ADD A,n
		op_add_n(cpu);
		break;
	case 0xC7:
		// RST 00H
		op_rst(cpu, 0x00);
		break;
	case 0xC8:
		// RET Z
		op_ret_n(cpu, cpu_flag_is_set(cpu, FLAG_Z));
		break;
	case 0xC9:
		// RET
		op_ret(cpu);
		break;
	case 0xCA:
		// JP Z,nn
		op_jp_n_nn(cpu, cpu_flag_is_set(cpu, FLAG_Z));
		break;
	case 0xCB:
		// Prefix
		break;
	case 0xCC:
		// CALL Z,nn
		op_call_nn(cpu, cpu_flag_is_set(cpu, FLAG_Z));
		break;
	case 0xCD:
		// CALL nn
		op_call_nn(cpu, true);
		break;
	case 0xCE:
		// ADC A, n
		op_adc_n(cpu);
		break;
	case 0xCF:
		// RST 08H
		op_rst(cpu, 0x08);
		break;
	case 0xD0:
		// RET NC
		op_ret_n(cpu, !cpu_flag_is_set(cpu, FLAG_C));
		break;
	case 0xD1:
		// POP DE
		op_pop(cpu, &cpu->d, &cpu->e);
		break;
	case 0xD2:
		// JP NC,nn
		op_jp_n_nn(cpu, !cpu_flag_is_set(cpu, FLAG_C));
		break;
	case 0xD3:
		// No code
		break;
	case 0xD4:
		// CALL NC,nn
		op_call_nn(cpu, !cpu_flag_is_set(cpu, FLAG_C));
		break;
	case 0xD5:
		// PUSH DE
		op_push_rr(cpu, &cpu->d, &cpu->e);
		break;
	case 0xD6:
		// SUB n
		op_sub_n(cpu);
		break;
	case 0xD7:
		// RST 10H
		op_rst(cpu, 0x10);
		break;
	case 0xD8:
		// RET C
		op_ret_n(cpu, cpu_flag_is_set(cpu, FLAG_C));
		break;
	case 0xD9:
		// RETI
		op_ret(cpu);
		if (cpu->state == SM83_CORE_IDLE_0)
			cpu->ime = true;
		break;
	case 0xDA:
		// JP C,nn
		op_jp_n_nn(cpu, cpu_flag_is_set(cpu, FLAG_C));
		break;
	case 0xDB:
		// No code
		break;
	case 0xDC:
		// CALL C,nn
		op_call_nn(cpu, cpu_flag_is_set(cpu, FLAG_C));
		break;
	case 0xDD:
		// No code
		break;
	case 0xDE:
		// SBC n
		op_sbc_n(cpu);
		break;
	case 0xDF:
		// RST 18H
		op_rst(cpu, 0x18);
		break;
	case 0xE0:
		// LD (0xFF00+n),A
		op_ld_ffn_a(cpu);
		break;
	case 0xE1:
		// POP HL
		op_pop(cpu, &cpu->h, &cpu->l);
		break;
	case 0xE2:
		// LD (0xFF00+C),A
		op_ld_ffc_a(cpu);
		break;
	case 0xE3:
		// No code
		break;
	case 0xE4:
		// No code
		break;
	case 0xE5:
		// PUSH HL
		op_push_rr(cpu, &cpu->h, &cpu->l);
		break;
	case 0xE6:
		// AND n
		op_and_n(cpu);
		break;
	case 0xE7:
		// RST 20H
		op_rst(cpu, 0x20);
		break;
	case 0xE8:
		// ADD SP,n
		op_add_sp(cpu);
		break;
	case 0xE9:
		// JP (HL)
		cpu->pc = HL(cpu);
		break;
	case 0xEA:
		// LD (nn),A
		op_ld_nn_a(cpu);
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
		op_xor_n(cpu);
		break;
	case 0xEF:
		// RST 28H
		op_rst(cpu, 0x28);
		break;
	case 0xF0:
		// LD A,(0xFF00+n)
		op_ldh_a_n(cpu);
		break;
	case 0xF1:
		// POP AF
		op_pop(cpu, &cpu->a, &cpu->f);
		cpu->f &= 0xF0;
		break;
	case 0xF2:
		// LD A,(C)
		op_ld_a_ffc(cpu);
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
		op_push_rr(cpu, &cpu->a, &cpu->f);
		break;
	case 0xF6:
		// OR n
		op_or_n(cpu);
		break;
	case 0xF7:
		// RST 30H
		op_rst(cpu, 0x30);
		break;
	case 0xF8:
		// LD HL,SP+n
		op_ld_spn(cpu);
		break;
	case 0xF9:
		// LD SP,HL
		op_ld_sp_hl(cpu);
		break;
	case 0xFA:
		// LD A,(nn)
		op_ld_a_nn(cpu);
		break;
	case 0xFB:
		// EI
		cpu->ime = true;
		break;
	case 0xFC:
		// No code
		break;
	case 0xFD:
		// No code
		break;
	case 0xFE:
		// CP n
		op_cp_n(cpu);
		break;
	case 0xFF:
		// RST 38H
		op_rst(cpu, 0x38);
		break;
	}
}

static void sm83_isa_cb_execute(struct sm83_core *cpu)
{
	u8 opcode;

	opcode = cpu->instruction.opcode;
	switch (opcode) {
	case 0x00:
		// RLC B
		op_rlc(cpu, &cpu->b, false);
		break;
	case 0x01:
		// RLC C
		op_rlc(cpu, &cpu->c, false);
		break;
	case 0x02:
		// RLC D
		op_rlc(cpu, &cpu->d, false);
		break;
	case 0x03:
		// RLC E
		op_rlc(cpu, &cpu->e, false);
		break;
	case 0x04:
		// RLC H
		op_rlc(cpu, &cpu->h, false);
		break;
	case 0x05:
		// RLC L
		op_rlc(cpu, &cpu->l, false);
		break;
	case 0x06:
		// RLC [HL]
		op_rlc_hl(cpu);
		break;
	case 0x07:
		// RLC A
		op_rlc(cpu, &cpu->a, false);
		break;
	case 0x08:
		// RRC B
		op_rrc(cpu, &cpu->b, false);
		break;
	case 0x09:
		// RRC C
		op_rrc(cpu, &cpu->c, false);
		break;
	case 0x0A:
		// RRC D
		op_rrc(cpu, &cpu->d, false);
		break;
	case 0x0B:
		// RRC E
		op_rrc(cpu, &cpu->e, false);
		break;
	case 0x0C:
		// RRC H
		op_rrc(cpu, &cpu->h, false);
		break;
	case 0x0D:
		// RRC L
		op_rrc(cpu, &cpu->l, false);
		break;
	case 0x0E:
		// RRC [HL]
		op_rrc_hl(cpu);
		break;
	case 0x0F:
		// RRC A
		op_rrc(cpu, &cpu->a, false);
		break;
	case 0x10:
		// RL B
		op_rl(cpu, &cpu->b, false);
		break;
	case 0x11:
		// RL C
		op_rl(cpu, &cpu->c, false);
		break;
	case 0x12:
		// RL D
		op_rl(cpu, &cpu->d, false);
		break;
	case 0x13:
		// RL E
		op_rl(cpu, &cpu->e, false);
		break;
	case 0x14:
		// RL H
		op_rl(cpu, &cpu->h, false);
		break;
	case 0x15:
		// RL L
		op_rl(cpu, &cpu->l, false);
		break;
	case 0x16:
		// RL [HL]
		op_rl_hl(cpu);
		break;
	case 0x17:
		// RL A
		op_rl(cpu, &cpu->a, false);
		break;
	case 0x18:
		// RR B
		op_rr(cpu, &cpu->b, false);
		break;
	case 0x19:
		// RR C
		op_rr(cpu, &cpu->c, false);
		break;
	case 0x1A:
		// RR D
		op_rr(cpu, &cpu->d, false);
		break;
	case 0x1B:
		// RR E
		op_rr(cpu, &cpu->e, false);
		break;
	case 0x1C:
		// RR H
		op_rr(cpu, &cpu->h, false);
		break;
	case 0x1D:
		// RR L
		op_rr(cpu, &cpu->l, false);
		break;
	case 0x1E:
		// RR [HL]
		op_rr_hl(cpu);
		break;
	case 0x1F:
		// RR A
		op_rr(cpu, &cpu->a, false);
		break;
	case 0x20:
		// SLA B
		op_sla(cpu, &cpu->b);
		break;
	case 0x21:
		// SLA C
		op_sla(cpu, &cpu->c);
		break;
	case 0x22:
		// SLA D
		op_sla(cpu, &cpu->d);
		break;
	case 0x23:
		// SLA E
		op_sla(cpu, &cpu->e);
		break;
	case 0x24:
		// SLA H
		op_sla(cpu, &cpu->h);
		break;
	case 0x25:
		// SLA L
		op_sla(cpu, &cpu->l);
		break;
	case 0x26:
		// SLA [HL]
		op_sla_hl(cpu);
		break;
	case 0x27:
		// SLA A
		op_sla(cpu, &cpu->a);
		break;
	case 0x28:
		// SRA B
		op_sra(cpu, &cpu->b);
		break;
	case 0x29:
		// SRA C
		op_sra(cpu, &cpu->c);
		break;
	case 0x2A:
		// SRA D
		op_sra(cpu, &cpu->d);
		break;
	case 0x2B:
		// SRA E
		op_sra(cpu, &cpu->e);
		break;
	case 0x2C:
		// SRA H
		op_sra(cpu, &cpu->h);
		break;
	case 0x2D:
		// SRA L
		op_sra(cpu, &cpu->l);
		break;
	case 0x2E:
		// SRA [HL]
		op_sra_hl(cpu);
		break;
	case 0x2F:
		// SRA A
		op_sra(cpu, &cpu->a);
		break;
	case 0x30:
		// SWAP B
		op_swap(cpu, &cpu->b);
		break;
	case 0x31:
		// SWAP C
		op_swap(cpu, &cpu->c);
		break;
	case 0x32:
		// SWAP D
		op_swap(cpu, &cpu->d);
		break;
	case 0x33:
		// SWAP E
		op_swap(cpu, &cpu->e);
		break;
	case 0x34:
		// SWAP H
		op_swap(cpu, &cpu->h);
		break;
	case 0x35:
		// SWAP L
		op_swap(cpu, &cpu->l);
		break;
	case 0x36:
		// SWAP [HL]
		op_swap_hl(cpu);
		break;
	case 0x37:
		// SWAP A
		op_swap(cpu, &cpu->a);
		break;
	case 0x38:
		// SRL B
		op_srl(cpu, &cpu->b);
		break;
	case 0x39:
		// SRL C
		op_srl(cpu, &cpu->c);
		break;
	case 0x3A:
		// SRL D
		op_srl(cpu, &cpu->d);
		break;
	case 0x3B:
		// SRL E
		op_srl(cpu, &cpu->e);
		break;
	case 0x3C:
		// SRL H
		op_srl(cpu, &cpu->h);
		break;
	case 0x3D:
		// SRL L
		op_srl(cpu, &cpu->l);
		break;
	case 0x3E:
		// SRL [HL]
		op_srl_hl(cpu);
		break;
	case 0x3F:
		// SRL A
		op_srl(cpu, &cpu->a);
		break;
	case 0x40:
		// BIT 0 B
		op_bit(cpu, &cpu->b, 0);
		break;
	case 0x41:
		// BIT 0 C
		op_bit(cpu, &cpu->c, 0);
		break;
	case 0x42:
		// BIT 0 D
		op_bit(cpu, &cpu->d, 0);
		break;
	case 0x43:
		// BIT 0 E
		op_bit(cpu, &cpu->e, 0);
		break;
	case 0x44:
		// BIT 0 H
		op_bit(cpu, &cpu->h, 0);
		break;
	case 0x45:
		// BIT 0 L
		op_bit(cpu, &cpu->l, 0);
		break;
	case 0x46:
		// BIT 0 [HL]
		op_bit_hl(cpu, 0);
		break;
	case 0x47:
		// BIT 0 A
		op_bit(cpu, &cpu->a, 0);
		break;
	case 0x48:
		// BIT 1 B
		op_bit(cpu, &cpu->b, 1);
		break;
	case 0x49:
		// BIT 1 C
		op_bit(cpu, &cpu->c, 1);
		break;
	case 0x4A:
		// BIT 1 D
		op_bit(cpu, &cpu->d, 1);
		break;
	case 0x4B:
		// BIT 1 E
		op_bit(cpu, &cpu->e, 1);
		break;
	case 0x4C:
		// BIT 1 H
		op_bit(cpu, &cpu->h, 1);
		break;
	case 0x4D:
		// BIT 1 L
		op_bit(cpu, &cpu->l, 1);
		break;
	case 0x4E:
		// BIT 1 [HL]
		op_bit_hl(cpu, 1);
		break;
	case 0x4F:
		// BIT 1 A
		op_bit(cpu, &cpu->a, 1);
		break;
	case 0x50:
		// BIT 2 B
		op_bit(cpu, &cpu->b, 2);
		break;
	case 0x51:
		// BIT 2 C
		op_bit(cpu, &cpu->c, 2);
		break;
	case 0x52:
		// BIT 2 D
		op_bit(cpu, &cpu->d, 2);
		break;
	case 0x53:
		// BIT 2 E
		op_bit(cpu, &cpu->e, 2);
		break;
	case 0x54:
		// BIT 2 H
		op_bit(cpu, &cpu->h, 2);
		break;
	case 0x55:
		// BIT 2 L
		op_bit(cpu, &cpu->l, 2);
		break;
	case 0x56:
		// BIT 2 [HL]
		op_bit_hl(cpu, 2);
		break;
	case 0x57:
		// BIT 2 A
		op_bit(cpu, &cpu->a, 2);
		break;
	case 0x58:
		// BIT 3 B
		op_bit(cpu, &cpu->b, 3);
		break;
	case 0x59:
		// BIT 3 C
		op_bit(cpu, &cpu->c, 3);
		break;
	case 0x5A:
		// BIT 3 D
		op_bit(cpu, &cpu->d, 3);
		break;
	case 0x5B:
		// BIT 3 E
		op_bit(cpu, &cpu->e, 3);
		break;
	case 0x5C:
		// BIT 3 H
		op_bit(cpu, &cpu->h, 3);
		break;
	case 0x5D:
		// BIT 3 L
		op_bit(cpu, &cpu->l, 3);
		break;
	case 0x5E:
		// BIT 3 [HL]
		op_bit_hl(cpu, 3);
		break;
	case 0x5F:
		// BIT 3 A
		op_bit(cpu, &cpu->a, 3);
		break;
	case 0x60:
		// BIT 4 B
		op_bit(cpu, &cpu->b, 4);
		break;
	case 0x61:
		// BIT 4 C
		op_bit(cpu, &cpu->c, 4);
		break;
	case 0x62:
		// BIT 4 D
		op_bit(cpu, &cpu->d, 4);
		break;
	case 0x63:
		// BIT 4 E
		op_bit(cpu, &cpu->e, 4);
		break;
	case 0x64:
		// BIT 4 H
		op_bit(cpu, &cpu->h, 4);
		break;
	case 0x65:
		// BIT 4 L
		op_bit(cpu, &cpu->l, 4);
		break;
	case 0x66:
		// BIT 4 [HL]
		op_bit_hl(cpu, 4);
		break;
	case 0x67:
		// BIT 4 A
		op_bit(cpu, &cpu->a, 4);
		break;
	case 0x68:
		// BIT 5 B
		op_bit(cpu, &cpu->b, 5);
		break;
	case 0x69:
		// BIT 5 C
		op_bit(cpu, &cpu->c, 5);
		break;
	case 0x6A:
		// BIT 5 D
		op_bit(cpu, &cpu->d, 5);
		break;
	case 0x6B:
		// BIT 5 E
		op_bit(cpu, &cpu->e, 5);
		break;
	case 0x6C:
		// BIT 5 H
		op_bit(cpu, &cpu->h, 5);
		break;
	case 0x6D:
		// BIT 5 L
		op_bit(cpu, &cpu->l, 5);
		break;
	case 0x6E:
		// BIT 5 [HL]
		op_bit_hl(cpu, 5);
		break;
	case 0x6F:
		// BIT 5 A
		op_bit(cpu, &cpu->a, 5);
		break;
	case 0x70:
		// BIT 6 B
		op_bit(cpu, &cpu->b, 6);
		break;
	case 0x71:
		// BIT 6 C
		op_bit(cpu, &cpu->c, 6);
		break;
	case 0x72:
		// BIT 6 D
		op_bit(cpu, &cpu->d, 6);
		break;
	case 0x73:
		// BIT 6 E
		op_bit(cpu, &cpu->e, 6);
		break;
	case 0x74:
		// BIT 6 H
		op_bit(cpu, &cpu->h, 6);
		break;
	case 0x75:
		// BIT 6 L
		op_bit(cpu, &cpu->l, 6);
		break;
	case 0x76:
		// BIT 6 [HL]
		op_bit_hl(cpu, 6);
		break;
	case 0x77:
		// BIT 6 A
		op_bit(cpu, &cpu->a, 6);
		break;
	case 0x78:
		// BIT 7 B
		op_bit(cpu, &cpu->b, 7);
		break;
	case 0x79:
		// BIT 7 C
		op_bit(cpu, &cpu->c, 7);
		break;
	case 0x7A:
		// BIT 7 D
		op_bit(cpu, &cpu->d, 7);
		break;
	case 0x7B:
		// BIT 7 E
		op_bit(cpu, &cpu->e, 7);
		break;
	case 0x7C:
		// BIT 7 H
		op_bit(cpu, &cpu->h, 7);
		break;
	case 0x7D:
		// BIT 7 L
		op_bit(cpu, &cpu->l, 7);
		break;
	case 0x7E:
		// BIT 7 [HL]
		op_bit_hl(cpu, 7);
		break;
	case 0x7F:
		// BIT 7 A
		op_bit(cpu, &cpu->a, 7);
		break;
	case 0x80:
		// RES 0 B
		op_res(cpu, &cpu->b, 0);
		break;
	case 0x81:
		// RES 0 C
		op_res(cpu, &cpu->c, 0);
		break;
	case 0x82:
		// RES 0 D
		op_res(cpu, &cpu->d, 0);
		break;
	case 0x83:
		// RES 0 E
		op_res(cpu, &cpu->e, 0);
		break;
	case 0x84:
		// RES 0 H
		op_res(cpu, &cpu->h, 0);
		break;
	case 0x85:
		// RES 0 L
		op_res(cpu, &cpu->l, 0);
		break;
	case 0x86:
		// RES 0 [HL]
		op_res_hl(cpu, 0);
		break;
	case 0x87:
		// RES 0 A
		op_res(cpu, &cpu->a, 0);
		break;
	case 0x88:
		// RES 1 B
		op_res(cpu, &cpu->b, 1);
		break;
	case 0x89:
		// RES 1 C
		op_res(cpu, &cpu->c, 1);
		break;
	case 0x8A:
		// RES 1 D
		op_res(cpu, &cpu->d, 1);
		break;
	case 0x8B:
		// RES 1 E
		op_res(cpu, &cpu->e, 1);
		break;
	case 0x8C:
		// RES 1 H
		op_res(cpu, &cpu->h, 1);
		break;
	case 0x8D:
		// RES 1 L
		op_res(cpu, &cpu->l, 1);
		break;
	case 0x8E:
		// RES 1 [HL]
		op_res_hl(cpu, 1);
		break;
	case 0x8F:
		// RES 1 A
		op_res(cpu, &cpu->a, 1);
		break;
	case 0x90:
		// RES 2 B
		op_res(cpu, &cpu->b, 2);
		break;
	case 0x91:
		// RES 2 C
		op_res(cpu, &cpu->c, 2);
		break;
	case 0x92:
		// RES 2 D
		op_res(cpu, &cpu->d, 2);
		break;
	case 0x93:
		// RES 2 E
		op_res(cpu, &cpu->e, 2);
		break;
	case 0x94:
		// RES 2 H
		op_res(cpu, &cpu->h, 2);
		break;
	case 0x95:
		// RES 2 L
		op_res(cpu, &cpu->l, 2);
		break;
	case 0x96:
		// RES 2 [HL]
		op_res_hl(cpu, 2);
		break;
	case 0x97:
		// RES 2 A
		op_res(cpu, &cpu->a, 2);
		break;
	case 0x98:
		// RES 3 B
		op_res(cpu, &cpu->b, 3);
		break;
	case 0x99:
		// RES 3 C
		op_res(cpu, &cpu->c, 3);
		break;
	case 0x9A:
		// RES 3 D
		op_res(cpu, &cpu->d, 3);
		break;
	case 0x9B:
		// RES 3 E
		op_res(cpu, &cpu->e, 3);
		break;
	case 0x9C:
		// RES 3 H
		op_res(cpu, &cpu->h, 3);
		break;
	case 0x9D:
		// RES 3 L
		op_res(cpu, &cpu->l, 3);
		break;
	case 0x9E:
		// RES 3 [HL]
		op_res_hl(cpu, 3);
		break;
	case 0x9F:
		// RES 3 A
		op_res(cpu, &cpu->a, 3);
		break;
	case 0xa0:
		// RES 4 B
		op_res(cpu, &cpu->b, 4);
		break;
	case 0xa1:
		// RES 4 C
		op_res(cpu, &cpu->c, 4);
		break;
	case 0xa2:
		// RES 4 D
		op_res(cpu, &cpu->d, 4);
		break;
	case 0xa3:
		// RES 4 E
		op_res(cpu, &cpu->e, 4);
		break;
	case 0xa4:
		// RES 4 H
		op_res(cpu, &cpu->h, 4);
		break;
	case 0xa5:
		// RES 4 L
		op_res(cpu, &cpu->l, 4);
		break;
	case 0xa6:
		// RES 4 [HL]
		op_res_hl(cpu, 4);
		break;
	case 0xa7:
		// RES 4 A
		op_res(cpu, &cpu->a, 4);
		break;
	case 0xa8:
		// RES 5 B
		op_res(cpu, &cpu->b, 5);
		break;
	case 0xa9:
		// RES 5 C
		op_res(cpu, &cpu->c, 5);
		break;
	case 0xAA:
		// RES 5 D
		op_res(cpu, &cpu->d, 5);
		break;
	case 0xAB:
		// RES 5 E
		op_res(cpu, &cpu->e, 5);
		break;
	case 0xAC:
		// RES 5 H
		op_res(cpu, &cpu->h, 5);
		break;
	case 0xAD:
		// RES 5 L
		op_res(cpu, &cpu->l, 5);
		break;
	case 0xAE:
		// RES 5 [HL]
		op_res_hl(cpu, 5);
		break;
	case 0xAF:
		// RES 5 A
		op_res(cpu, &cpu->a, 5);
		break;
	case 0xb0:
		// RES 6 B
		op_res(cpu, &cpu->b, 6);
		break;
	case 0xb1:
		// RES 6 C
		op_res(cpu, &cpu->c, 6);
		break;
	case 0xb2:
		// RES 6 D
		op_res(cpu, &cpu->d, 6);
		break;
	case 0xb3:
		// RES 6 E
		op_res(cpu, &cpu->e, 6);
		break;
	case 0xb4:
		// RES 6 H
		op_res(cpu, &cpu->h, 6);
		break;
	case 0xb5:
		// RES 6 L
		op_res(cpu, &cpu->l, 6);
		break;
	case 0xb6:
		// RES 6 [HL]
		op_res_hl(cpu, 6);
		break;
	case 0xb7:
		// RES 6 A
		op_res(cpu, &cpu->a, 6);
		break;
	case 0xb8:
		// RES 7 B
		op_res(cpu, &cpu->b, 7);
		break;
	case 0xb9:
		// RES 7 C
		op_res(cpu, &cpu->c, 7);
		break;
	case 0xBA:
		// RES 7 D
		op_res(cpu, &cpu->d, 7);
		break;
	case 0xBB:
		// RES 7 E
		op_res(cpu, &cpu->e, 7);
		break;
	case 0xBC:
		// RES 7 H
		op_res(cpu, &cpu->h, 7);
		break;
	case 0xBD:
		// RES 7 L
		op_res(cpu, &cpu->l, 7);
		break;
	case 0xBE:
		// RES 7 [HL]
		op_res_hl(cpu, 7);
		break;
	case 0xBF:
		// RES 7 A
		op_res(cpu, &cpu->a, 7);
		break;
	case 0xC0:
		// SET 0 B
		op_set(cpu, &cpu->b, 0);
		break;
	case 0xC1:
		// SET 0 C
		op_set(cpu, &cpu->c, 0);
		break;
	case 0xC2:
		// SET 0 D
		op_set(cpu, &cpu->d, 0);
		break;
	case 0xC3:
		// SET 0 E
		op_set(cpu, &cpu->e, 0);
		break;
	case 0xC4:
		// SET 0 H
		op_set(cpu, &cpu->h, 0);
		break;
	case 0xC5:
		// SET 0 L
		op_set(cpu, &cpu->l, 0);
		break;
	case 0xC6:
		// SET 0 [HL]
		op_set_hl(cpu, 0);
		break;
	case 0xC7:
		// SET 0 A
		op_set(cpu, &cpu->a, 0);
		break;
	case 0xC8:
		// SET 1 B
		op_set(cpu, &cpu->b, 1);
		break;
	case 0xC9:
		// SET 1 C
		op_set(cpu, &cpu->c, 1);
		break;
	case 0xCA:
		// SET 1 D
		op_set(cpu, &cpu->d, 1);
		break;
	case 0xCB:
		// SET 1 E
		op_set(cpu, &cpu->e, 1);
		break;
	case 0xCC:
		// SET 1 H
		op_set(cpu, &cpu->h, 1);
		break;
	case 0xCD:
		// SET 1 L
		op_set(cpu, &cpu->l, 1);
		break;
	case 0xCE:
		// SET 1 [HL]
		op_set_hl(cpu, 1);
		break;
	case 0xCF:
		// SET 1 A
		op_set(cpu, &cpu->a, 1);
		break;
	case 0xD0:
		// SET 2 B
		op_set(cpu, &cpu->b, 2);
		break;
	case 0xD1:
		// SET 2 C
		op_set(cpu, &cpu->c, 2);
		break;
	case 0xD2:
		// SET 2 D
		op_set(cpu, &cpu->d, 2);
		break;
	case 0xD3:
		// SET 2 E
		op_set(cpu, &cpu->e, 2);
		break;
	case 0xD4:
		// SET 2 H
		op_set(cpu, &cpu->h, 2);
		break;
	case 0xD5:
		// SET 2 L
		op_set(cpu, &cpu->l, 2);
		break;
	case 0xD6:
		// SET 2 [HL]
		op_set_hl(cpu, 2);
		break;
	case 0xD7:
		// SET 2 A
		op_set(cpu, &cpu->a, 2);
		break;
	case 0xD8:
		// SET 3 B
		op_set(cpu, &cpu->b, 3);
		break;
	case 0xD9:
		// SET 3 C
		op_set(cpu, &cpu->c, 3);
		break;
	case 0xDA:
		// SET 3 D
		op_set(cpu, &cpu->d, 3);
		break;
	case 0xDB:
		// SET 3 E
		op_set(cpu, &cpu->e, 3);
		break;
	case 0xDC:
		// SET 3 H
		op_set(cpu, &cpu->h, 3);
		break;
	case 0xDD:
		// SET 3 L
		op_set(cpu, &cpu->l, 3);
		break;
	case 0xDE:
		// SET 3 [HL]
		op_set_hl(cpu, 3);
		break;
	case 0xDF:
		// SET 3 A
		op_set(cpu, &cpu->a, 3);
		break;
	case 0xE0:
		// SET 4 B
		op_set(cpu, &cpu->b, 4);
		break;
	case 0xE1:
		// SET 4 C
		op_set(cpu, &cpu->c, 4);
		break;
	case 0xE2:
		// SET 4 D
		op_set(cpu, &cpu->d, 4);
		break;
	case 0xE3:
		// SET 4 E
		op_set(cpu, &cpu->e, 4);
		break;
	case 0xE4:
		// SET 4 H
		op_set(cpu, &cpu->h, 4);
		break;
	case 0xE5:
		// SET 4 L
		op_set(cpu, &cpu->l, 4);
		break;
	case 0xE6:
		// SET 4 [HL]
		op_set_hl(cpu, 4);
		break;
	case 0xE7:
		// SET 4 A
		op_set(cpu, &cpu->a, 4);
		break;
	case 0xE8:
		// SET 5 B
		op_set(cpu, &cpu->b, 5);
		break;
	case 0xE9:
		// SET 5 C
		op_set(cpu, &cpu->c, 5);
		break;
	case 0xEA:
		// SET 5 D
		op_set(cpu, &cpu->d, 5);
		break;
	case 0xEB:
		// SET 5 E
		op_set(cpu, &cpu->e, 5);
		break;
	case 0xEC:
		// SET 5 H
		op_set(cpu, &cpu->h, 5);
		break;
	case 0xED:
		// SET 5 L
		op_set(cpu, &cpu->l, 5);
		break;
	case 0xEE:
		// SET 5 [HL]
		op_set_hl(cpu, 5);
		break;
	case 0xEF:
		// SET 5 A
		op_set(cpu, &cpu->a, 5);
		break;
	case 0xF0:
		// SET 6 B
		op_set(cpu, &cpu->b, 6);
		break;
	case 0xF1:
		// SET 6 C
		op_set(cpu, &cpu->c, 6);
		break;
	case 0xF2:
		// SET 6 D
		op_set(cpu, &cpu->d, 6);
		break;
	case 0xF3:
		// SET 6 E
		op_set(cpu, &cpu->e, 6);
		break;
	case 0xF4:
		// SET 6 H
		op_set(cpu, &cpu->h, 6);
		break;
	case 0xF5:
		// SET 6 L
		op_set(cpu, &cpu->l, 6);
		break;
	case 0xF6:
		// SET 6 [HL]
		op_set_hl(cpu, 6);
		break;
	case 0xF7:
		// SET 6 A
		op_set(cpu, &cpu->a, 6);
		break;
	case 0xF8:
		// SET 7 B
		op_set(cpu, &cpu->b, 7);
		break;
	case 0xF9:
		// SET 7 C
		op_set(cpu, &cpu->c, 7);
		break;
	case 0xFA:
		// SET 7 D
		op_set(cpu, &cpu->d, 7);
		break;
	case 0xFB:
		// SET 7 E
		op_set(cpu, &cpu->e, 7);
		break;
	case 0xFC:
		// SET 7 H
		op_set(cpu, &cpu->h, 7);
		break;
	case 0xFD:
		// SET 7 L
		op_set(cpu, &cpu->l, 7);
		break;
	case 0xFE:
		// SET 7 [HL]
		op_set_hl(cpu, 7);
		break;
	case 0xFF:
		// SET 7 A
		op_set(cpu, &cpu->a, 7);
		break;
	}
}

void sm83_isa_execute(struct sm83_core *cpu)
{
	if (!cpu->instruction.prefixed) {
		sm83_isa_execute_non_prefixed(cpu);
	} else {
		if (cpu->state == SM83_CORE_FETCH) {
			cpu->state = SM83_CORE_PC;
		} else {
			sm83_isa_cb_execute(cpu);
		}
	}
}
