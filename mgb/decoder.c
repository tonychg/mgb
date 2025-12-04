#include "mgb/sm83.h"
#include "mgb/memory.h"
#include "platform/mm.h"
#include "platform/types.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

static const char *OP_TABLES_OP_1[256] = {
	NULL, "BC",  "BC", "BC",  "B",	 "B",	"B",  NULL,  "a16", "HL",
	"A",  "BC",  "C",  "C",	  "C",	 NULL,	"n8", "DE",  "DE",  "DE",
	"D",  "D",   "D",  NULL,  "e8",	 "HL",	"A",  "DE",  "E",   "E",
	"E",  NULL,  "NZ", "HL",  "HL",	 "HL",	"H",  "H",   "H",   NULL,
	"Z",  "HL",  "A",  "HL",  "L",	 "L",	"L",  NULL,  "NC",  "SP",
	"HL", "SP",  "HL", "HL",  "HL",	 NULL,	"C",  "HL",  "A",   "SP",
	"A",  "A",   "A",  NULL,  "B",	 "B",	"B",  "B",   "B",   "B",
	"B",  "B",   "C",  "C",	  "C",	 "C",	"C",  "C",   "C",   "C",
	"D",  "D",   "D",  "D",	  "D",	 "D",	"D",  "D",   "E",   "E",
	"E",  "E",   "E",  "E",	  "E",	 "E",	"H",  "H",   "H",   "H",
	"H",  "H",   "H",  "H",	  "L",	 "L",	"L",  "L",   "L",   "L",
	"L",  "L",   "HL", "HL",  "HL",	 "HL",	"HL", "HL",  NULL,  "HL",
	"A",  "A",   "A",  "A",	  "A",	 "A",	"A",  "A",   "A",   "A",
	"A",  "A",   "A",  "A",	  "A",	 "A",	"A",  "A",   "A",   "A",
	"A",  "A",   "A",  "A",	  "A",	 "A",	"A",  "A",   "A",   "A",
	"A",  "A",   "A",  "A",	  "A",	 "A",	"A",  "A",   "A",   "A",
	"A",  "A",   "A",  "A",	  "A",	 "A",	"A",  "A",   "A",   "A",
	"A",  "A",   "A",  "A",	  "A",	 "A",	"A",  "A",   "A",   "A",
	"A",  "A",   "A",  "A",	  "A",	 "A",	"A",  "A",   "A",   "A",
	"A",  "A",   "NZ", "BC",  "NZ",	 "a16", "NZ", "BC",  "A",   "$00",
	"Z",  NULL,  "Z",  NULL,  "Z",	 "a16", "A",  "$08", "NC",  "DE",
	"NC", NULL,  "NC", "DE",  "A",	 "$10", "C",  NULL,  "C",   NULL,
	"C",  NULL,  "A",  "$18", "a8",	 "HL",	"C",  NULL,  NULL,  "HL",
	"A",  "$20", "SP", "HL",  "a16", NULL,	NULL, NULL,  "A",   "$28",
	"A",  "AF",  "A",  NULL,  NULL,	 "AF",	"A",  "$30", "HL",  "SP",
	"A",  NULL,  NULL, NULL,  "A",	 "$38"
};

static const char *OP_TABLES_OP_2[256] = {
	NULL,  "n16", "A",   NULL,  NULL,  NULL, "n8",	NULL,  "SP",  "BC",
	"BC",  NULL,  NULL,  NULL,  "n8",  NULL, NULL,	"n16", "A",   NULL,
	NULL,  NULL,  "n8",  NULL,  NULL,  "DE", "DE",	NULL,  NULL,  NULL,
	"n8",  NULL,  "e8",  "n16", "A",   NULL, NULL,	NULL,  "n8",  NULL,
	"e8",  "HL",  "HL",  NULL,  NULL,  NULL, "n8",	NULL,  "e8",  "n16",
	"A",   NULL,  NULL,  NULL,  "n8",  NULL, "e8",	"SP",  "HL",  NULL,
	NULL,  NULL,  "n8",  NULL,  "B",   "C",	 "D",	"E",   "H",   "L",
	"HL",  "A",   "B",   "C",   "D",   "E",	 "H",	"L",   "HL",  "A",
	"B",   "C",   "D",   "E",   "H",   "L",	 "HL",	"A",   "B",   "C",
	"D",   "E",   "H",   "L",   "HL",  "A",	 "B",	"C",   "D",   "E",
	"H",   "L",   "HL",  "A",   "B",   "C",	 "D",	"E",   "H",   "L",
	"HL",  "A",   "B",   "C",   "D",   "E",	 "H",	"L",   NULL,  "A",
	"B",   "C",   "D",   "E",   "H",   "L",	 "HL",	"A",   "B",   "C",
	"D",   "E",   "H",   "L",   "HL",  "A",	 "B",	"C",   "D",   "E",
	"H",   "L",   "HL",  "A",   "B",   "C",	 "D",	"E",   "H",   "L",
	"HL",  "A",   "B",   "C",   "D",   "E",	 "H",	"L",   "HL",  "A",
	"B",   "C",   "D",   "E",   "H",   "L",	 "HL",	"A",   "B",   "C",
	"D",   "E",   "H",   "L",   "HL",  "A",	 "B",	"C",   "D",   "E",
	"H",   "L",   "HL",  "A",   "B",   "C",	 "D",	"E",   "H",   "L",
	"HL",  "A",   NULL,  NULL,  "a16", NULL, "a16", NULL,  "n8",  NULL,
	NULL,  NULL,  "a16", NULL,  "a16", NULL, "n8",	NULL,  NULL,  NULL,
	"a16", NULL,  "a16", NULL,  "n8",  NULL, NULL,	NULL,  "a16", NULL,
	"a16", NULL,  "n8",  NULL,  "A",   NULL, "A",	NULL,  NULL,  NULL,
	"n8",  NULL,  "e8",  NULL,  "A",   NULL, NULL,	NULL,  "n8",  NULL,
	"a8",  NULL,  "C",   NULL,  NULL,  NULL, "n8",	NULL,  "SP",  "HL",
	"a16", NULL,  NULL,  NULL,  "n8",  NULL
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

static const char *OP_TABLES_CB_OP_1[256] = {
	"B", "C",  "D",	 "E", "H",  "L", "HL", "A",  "B", "C",	"D",  "E", "H",
	"L", "HL", "A",	 "B", "C",  "D", "E",  "H",  "L", "HL", "A",  "B", "C",
	"D", "E",  "H",	 "L", "HL", "A", "B",  "C",  "D", "E",	"H",  "L", "HL",
	"A", "B",  "C",	 "D", "E",  "H", "L",  "HL", "A", "B",	"C",  "D", "E",
	"H", "L",  "HL", "A", "B",  "C", "D",  "E",  "H", "L",	"HL", "A", "0",
	"0", "0",  "0",	 "0", "0",  "0", "0",  "1",  "1", "1",	"1",  "1", "1",
	"1", "1",  "2",	 "2", "2",  "2", "2",  "2",  "2", "2",	"3",  "3", "3",
	"3", "3",  "3",	 "3", "3",  "4", "4",  "4",  "4", "4",	"4",  "4", "4",
	"5", "5",  "5",	 "5", "5",  "5", "5",  "5",  "6", "6",	"6",  "6", "6",
	"6", "6",  "6",	 "7", "7",  "7", "7",  "7",  "7", "7",	"7",  "0", "0",
	"0", "0",  "0",	 "0", "0",  "0", "1",  "1",  "1", "1",	"1",  "1", "1",
	"1", "2",  "2",	 "2", "2",  "2", "2",  "2",  "2", "3",	"3",  "3", "3",
	"3", "3",  "3",	 "3", "4",  "4", "4",  "4",  "4", "4",	"4",  "4", "5",
	"5", "5",  "5",	 "5", "5",  "5", "5",  "6",  "6", "6",	"6",  "6", "6",
	"6", "6",  "7",	 "7", "7",  "7", "7",  "7",  "7", "7",	"0",  "0", "0",
	"0", "0",  "0",	 "0", "0",  "1", "1",  "1",  "1", "1",	"1",  "1", "1",
	"2", "2",  "2",	 "2", "2",  "2", "2",  "2",  "3", "3",	"3",  "3", "3",
	"3", "3",  "3",	 "4", "4",  "4", "4",  "4",  "4", "4",	"4",  "5", "5",
	"5", "5",  "5",	 "5", "5",  "5", "6",  "6",  "6", "6",	"6",  "6", "6",
	"6", "7",  "7",	 "7", "7",  "7", "7",  "7",  "7"
};

static const char *OP_TABLES_CB_OP_2[256] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, "B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",
	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",
	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",
	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",
	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",
	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",
	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",
	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",
	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",
	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",
	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",
	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",
	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",
	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",
	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",
	"B",  "C",  "D",  "E",	"H",  "L",  "HL", "A",	"B",  "C",  "D",  "E",
	"H",  "L",  "HL", "A"
};

static int OPCODE_LENGTH[256] = {
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

static int OPCODE_MACHINE_CYCLES[256] = {
	1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1, 1, 3, 2, 2, 1, 1, 2, 1,
	3, 2, 2, 2, 1, 1, 2, 1, 2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1,
	2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2,
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 0, 3, 6, 2, 4, 2, 3, 3, 0, 3, 4, 2, 4,
	2, 4, 3, 0, 3, 0, 2, 4, 3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4,
	3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4
};

static int OPCODE_CB_MACHINE_CYCLES[256] = {
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 3, 2,
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2
};

static struct sm83_instruction cpu_decode_non_prefixed(u8 opcode)
{
	struct sm83_instruction instruction;

	instruction.opcode = opcode;
	instruction.length = OPCODE_LENGTH[opcode];
	instruction.mnemonic = OP_TABLES_MNEMONIC[opcode];
	instruction.op1 = OP_TABLES_OP_1[opcode];
	instruction.op2 = OP_TABLES_OP_2[opcode];
	instruction.cycles = OPCODE_MACHINE_CYCLES[opcode];
	instruction.prefixed = false;
	return instruction;
}

static struct sm83_instruction cpu_decode_prefixed(u8 opcode)
{
	struct sm83_instruction instruction;

	instruction.opcode = opcode;
	instruction.length = 2;
	instruction.mnemonic = OP_TABLES_CB_MNEMONIC[opcode];
	instruction.op1 = OP_TABLES_CB_OP_1[opcode];
	instruction.op2 = OP_TABLES_CB_OP_2[opcode];
	instruction.cycles = OPCODE_CB_MACHINE_CYCLES[opcode];
	instruction.prefixed = true;
	return instruction;
}

struct sm83_instruction sm83_decode(struct sm83_core *cpu)
{
	struct sm83_instruction instruction;

	if (cpu->bus == 0xCB) {
		instruction = cpu_decode_prefixed(
			cpu->memory.load8(cpu, cpu->pc + 1));
	} else {
		instruction = cpu_decode_non_prefixed(cpu->bus);
	}
	return instruction;
}

static char *sm83_resolve_operand(struct sm83_core *cpu, const char *op,
				  u16 indice)
{
	char *buffer;

	buffer = (char *)calloc(16, sizeof(char));
	if (!buffer)
		return NULL;
	if (!strcmp(op, "a16") || !strcmp(op, "n16")) {
		u16 segment = unsigned_16(cpu->memory.load8(cpu, indice + 1),
					  cpu->memory.load8(cpu, indice + 2));
		sprintf(buffer, "%s[$%04X]", op, segment);
	} else if (!strcmp(op, "a8") || !strcmp(op, "n8")) {
		sprintf(buffer, "%s[$%02X]", op,
			cpu->memory.load8(cpu, indice + 1));
	} else if (!strcmp(op, "e8")) {
		u8 byte = cpu->memory.load8(cpu, indice + 1);
		s8 offset = (s8)byte;
		sprintf(buffer, "%s[$%02X] [%d]", op, byte, offset);
	} else {
		sprintf(buffer, "%s", op);
	}
	return buffer;
}

void sm83_info(struct sm83_core *cpu)
{
	char *disasm = NULL;
	printf("  A = $%1$02X [%1$08b] |  F = $%2$02X [%2$08b]\n", cpu->a,
	       cpu->f);
	printf("  B = $%1$02X [%1$08b] |  C = $%2$02X [%2$08b]\n", cpu->b,
	       cpu->c);
	printf("  D = $%1$02X [%1$08b] |  E = $%2$02X [%2$08b]\n", cpu->d,
	       cpu->e);
	printf("  H = $%1$02X [%1$08b] |  L = $%2$02X [%2$08b]\n", cpu->h,
	       cpu->l);
	printf("     Z = %1$d | N = %2$d   |  SP = $%3$04X\n",
	       cpu_flag_is_set(cpu, FLAG_Z), cpu_flag_is_set(cpu, FLAG_N),
	       cpu->sp);
	printf("     H = %1$d | C = %2$d   |  PC = $%3$04X\n",
	       cpu_flag_is_set(cpu, FLAG_H), cpu_flag_is_set(cpu, FLAG_C),
	       cpu->pc);
	printf(" IME = %3d | HALT = %3d | DMA = %3d\n", cpu->ime, cpu->halted,
	       cpu->dma.scheduled);
	printf(" DIV = %3d | TIMA = %3d | M-cycles = %lu\n",
	       cpu->memory.load8(cpu, DIV), cpu->memory.load8(cpu, TIMA),
	       cpu->cycles);
	disasm = sm83_disassemble(cpu);
	printf("  %s\n", disasm);
	zfree(disasm);
}

void sm83_memory_io_debug(struct sm83_core *cpu)
{
	for (int i = 0xFF00; i <= 0xFFFF; i++) {
		u8 byte = cpu->memory.load8(cpu, i);
		printf("%04X : %02X [%08b] %d\n", i, byte, byte, byte);
	}
}

void sm83_memory_debug(struct sm83_core *cpu, u16 start, u16 end)
{
	for (int i = start; i <= end; i++) {
		if (cpu->memory.load8(cpu, start + i) != 0)
			printf("%02X", cpu->memory.load8(cpu, start + i));
		else
			printf("..");
		if ((i + 1) % 32 == 0 && i > 0)
			printf("\n");
		else if ((i + 1) % 8 == 0 && i > 0)
			printf(" ");
	}
	printf("\n");
}

char *sm83_disassemble(struct sm83_core *cpu)
{
	char *buffer;

	buffer = (char *)calloc(256, sizeof(char));
	if (!buffer)
		return NULL;
	sprintf(buffer + strlen(buffer), "00:%04X", cpu->index);
	for (int i = 0; i < cpu->instruction.length; i++) {
		sprintf(buffer + strlen(buffer), " %02X",
			cpu->memory.load8(cpu, cpu->index + i));
	}
	sprintf(buffer + strlen(buffer), " -> ");
	if (cpu->instruction.op1 && cpu->instruction.op2) {
		char *op1 = sm83_resolve_operand(cpu, cpu->instruction.op1,
						 cpu->index);
		char *op2 = sm83_resolve_operand(cpu, cpu->instruction.op2,
						 cpu->index);
		sprintf(buffer + strlen(buffer), "%s %s %s",
			cpu->instruction.mnemonic, op1, op2);
		zfree(op1);
		zfree(op2);
	} else if (cpu->instruction.op1) {
		char *op1 = sm83_resolve_operand(cpu, cpu->instruction.op1,
						 cpu->index);
		sprintf(buffer + strlen(buffer), "%s %s",
			cpu->instruction.mnemonic, op1);
		zfree(op1);
	} else {
		sprintf(buffer + strlen(buffer), "%s",
			cpu->instruction.mnemonic);
	}
	return buffer;
}
