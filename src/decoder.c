#include "cpu.h"

static const char *OP_TABLES_MNEMONIC[256] = {
	"NOP",	      "LD",	    "LD",	  "INC",	"INC",
	"DEC",	      "LD",	    "RLCA",	  "LD",		"ADD",
	"LD",	      "DEC",	    "INC",	  "DEC",	"LD",
	"RRCA",	      "STOP",	    "LD",	  "LD",		"INC",
	"INC",	      "DEC",	    "LD",	  "RLA",	"JR",
	"ADD",	      "LD",	    "DEC",	  "INC",	"DEC",
	"LD",	      "RRA",	    "JR",	  "LD",		"LD",
	"INC",	      "INC",	    "DEC",	  "LD",		"DAA",
	"JR",	      "ADD",	    "LD",	  "DEC",	"INC",
	"DEC",	      "LD",	    "CPL",	  "JR",		"LD",
	"LD",	      "INC",	    "INC",	  "DEC",	"LD",
	"SCF",	      "JR",	    "ADD",	  "LD",		"DEC",
	"INC",	      "DEC",	    "LD",	  "CCF",	"LD",
	"LD",	      "LD",	    "LD",	  "LD",		"LD",
	"LD",	      "LD",	    "LD",	  "LD",		"LD",
	"LD",	      "LD",	    "LD",	  "LD",		"LD",
	"LD",	      "LD",	    "LD",	  "LD",		"LD",
	"LD",	      "LD",	    "LD",	  "LD",		"LD",
	"LD",	      "LD",	    "LD",	  "LD",		"LD",
	"LD",	      "LD",	    "LD",	  "LD",		"LD",
	"LD",	      "LD",	    "LD",	  "LD",		"LD",
	"LD",	      "LD",	    "LD",	  "LD",		"LD",
	"LD",	      "LD",	    "LD",	  "LD",		"LD",
	"LD",	      "LD",	    "LD",	  "HALT",	"LD",
	"LD",	      "LD",	    "LD",	  "LD",		"LD",
	"LD",	      "LD",	    "LD",	  "ADD",	"ADD",
	"ADD",	      "ADD",	    "ADD",	  "ADD",	"ADD",
	"ADD",	      "ADC",	    "ADC",	  "ADC",	"ADC",
	"ADC",	      "ADC",	    "ADC",	  "ADC",	"SUB",
	"SUB",	      "SUB",	    "SUB",	  "SUB",	"SUB",
	"SUB",	      "SUB",	    "SBC",	  "SBC",	"SBC",
	"SBC",	      "SBC",	    "SBC",	  "SBC",	"SBC",
	"AND",	      "AND",	    "AND",	  "AND",	"AND",
	"AND",	      "AND",	    "AND",	  "XOR",	"XOR",
	"XOR",	      "XOR",	    "XOR",	  "XOR",	"XOR",
	"XOR",	      "OR",	    "OR",	  "OR",		"OR",
	"OR",	      "OR",	    "OR",	  "OR",		"CP",
	"CP",	      "CP",	    "CP",	  "CP",		"CP",
	"CP",	      "CP",	    "RET",	  "POP",	"JP",
	"JP",	      "CALL",	    "PUSH",	  "ADD",	"RST",
	"RET",	      "RET",	    "JP",	  "PREFIX",	"CALL",
	"CALL",	      "ADC",	    "RST",	  "RET",	"POP",
	"JP",	      "ILLEGAL_D3", "CALL",	  "PUSH",	"SUB",
	"RST",	      "RET",	    "RETI",	  "JP",		"ILLEGAL_DB",
	"CALL",	      "ILLEGAL_DD", "SBC",	  "RST",	"LDH",
	"POP",	      "LDH",	    "ILLEGAL_E3", "ILLEGAL_E4", "PUSH",
	"AND",	      "RST",	    "ADD",	  "JP",		"LD",
	"ILLEGAL_EB", "ILLEGAL_EC", "ILLEGAL_ED", "XOR",	"RST",
	"LDH",	      "POP",	    "LDH",	  "DI",		"ILLEGAL_F4",
	"PUSH",	      "OR",	    "RST",	  "LD",		"LD",
	"LD",	      "EI",	    "ILLEGAL_FC", "ILLEGAL_FD", "CP",
	"RST",
};

static const char *OP_TABLES_CB_MNEMONIC[256] = {
	"RLC",	"RLC",	"RLC", "RLC",  "RLC",  "RLC",  "RLC",  "RLC",  "RRC",
	"RRC",	"RRC",	"RRC", "RRC",  "RRC",  "RRC",  "RRC",  "RL",   "RL",
	"RL",	"RL",	"RL",  "RL",   "RL",   "RL",   "RR",   "RR",   "RR",
	"RR",	"RR",	"RR",  "RR",   "RR",   "SLA",  "SLA",  "SLA",  "SLA",
	"SLA",	"SLA",	"SLA", "SLA",  "SRA",  "SRA",  "SRA",  "SRA",  "SRA",
	"SRA",	"SRA",	"SRA", "SWAP", "SWAP", "SWAP", "SWAP", "SWAP", "SWAP",
	"SWAP", "SWAP", "SRL", "SRL",  "SRL",  "SRL",  "SRL",  "SRL",  "SRL",
	"SRL",	"BIT",	"BIT", "BIT",  "BIT",  "BIT",  "BIT",  "BIT",  "BIT",
	"BIT",	"BIT",	"BIT", "BIT",  "BIT",  "BIT",  "BIT",  "BIT",  "BIT",
	"BIT",	"BIT",	"BIT", "BIT",  "BIT",  "BIT",  "BIT",  "BIT",  "BIT",
	"BIT",	"BIT",	"BIT", "BIT",  "BIT",  "BIT",  "BIT",  "BIT",  "BIT",
	"BIT",	"BIT",	"BIT", "BIT",  "BIT",  "BIT",  "BIT",  "BIT",  "BIT",
	"BIT",	"BIT",	"BIT", "BIT",  "BIT",  "BIT",  "BIT",  "BIT",  "BIT",
	"BIT",	"BIT",	"BIT", "BIT",  "BIT",  "BIT",  "BIT",  "BIT",  "BIT",
	"BIT",	"BIT",	"RES", "RES",  "RES",  "RES",  "RES",  "RES",  "RES",
	"RES",	"RES",	"RES", "RES",  "RES",  "RES",  "RES",  "RES",  "RES",
	"RES",	"RES",	"RES", "RES",  "RES",  "RES",  "RES",  "RES",  "RES",
	"RES",	"RES",	"RES", "RES",  "RES",  "RES",  "RES",  "RES",  "RES",
	"RES",	"RES",	"RES", "RES",  "RES",  "RES",  "RES",  "RES",  "RES",
	"RES",	"RES",	"RES", "RES",  "RES",  "RES",  "RES",  "RES",  "RES",
	"RES",	"RES",	"RES", "RES",  "RES",  "RES",  "RES",  "RES",  "RES",
	"RES",	"RES",	"RES", "SET",  "SET",  "SET",  "SET",  "SET",  "SET",
	"SET",	"SET",	"SET", "SET",  "SET",  "SET",  "SET",  "SET",  "SET",
	"SET",	"SET",	"SET", "SET",  "SET",  "SET",  "SET",  "SET",  "SET",
	"SET",	"SET",	"SET", "SET",  "SET",  "SET",  "SET",  "SET",  "SET",
	"SET",	"SET",	"SET", "SET",  "SET",  "SET",  "SET",  "SET",  "SET",
	"SET",	"SET",	"SET", "SET",  "SET",  "SET",  "SET",  "SET",  "SET",
	"SET",	"SET",	"SET", "SET",  "SET",  "SET",  "SET",  "SET",  "SET",
	"SET",	"SET",	"SET", "SET"
};

static int OPCODE_WIDTH[256] = {
	/*      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/* 0 */ 1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1,
	/* 1 */ 2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
	/* 2 */ 2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
	/* 3 */ 2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
	/* 4 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 6 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 8 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 9 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* A */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* B */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* C */ 1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 2, 3, 3, 2, 1,
	/* D */ 1, 1, 3, 0, 3, 1, 2, 1, 1, 1, 3, 0, 3, 0, 2, 1,
	/* E */ 2, 1, 1, 0, 0, 1, 2, 1, 2, 1, 3, 0, 0, 0, 2, 1,
	/* F */ 2, 1, 1, 1, 0, 1, 2, 1, 2, 1, 3, 1, 0, 0, 2, 1,
};

int cpu_instruction_length(u8 opcode)
{
	return OPCODE_WIDTH[opcode];
}

const char *cpu_opcode_mnemonic(u8 opcode)
{
	if (opcode != 0xCB)
		return OP_TABLES_MNEMONIC[opcode];
	else
		return OP_TABLES_CB_MNEMONIC[opcode];
}
