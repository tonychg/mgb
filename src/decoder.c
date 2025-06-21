#include "cpu.h"

static const char *OP_TABLES_REPR[256] = {
	"NOP",	   "LDBC",     "LDBC_A",  "INCBC",    "INCB",	 "DECB",
	"LDB_",	   "RLCA_",    "LDISP",	  "ADDHL_BC", "LDA_BC",	 "DECBC",
	"INCC",	   "DECC",     "LDC_",	  "RRCA_",    "STOP",	 "LDDE",
	"LDDE_A",  "INCDE",    "INCD",	  "DECD",     "LDD_",	 "RLA_",
	"JR",	   "ADDHL_DE", "LDA_DE",  "DECDE",    "INCE",	 "DECE",
	"LDE_",	   "RRA_",     "JRNZ",	  "LDHL",     "LDIHLA",	 "INCHL",
	"INCH",	   "DECH",     "LDH_",	  "DAA",      "JRZ",	 "ADDHL_HL",
	"LDA_IHL", "DECHL",    "INCL",	  "DECL",     "LDL_",	 "CPL_",
	"JRNC",	   "LDSP",     "LDDHLA",  "INCSP",    "INC_HL",	 "DEC_HL",
	"LDHL_",   "SCF",      "JRC",	  "ADDHL_SP", "LDA_DHL", "DECSP",
	"INCA",	   "DECA",     "LDA_",	  "CCF",      "LDB_B",	 "LDB_C",
	"LDB_D",   "LDB_E",    "LDB_H",	  "LDB_L",    "LDB_HL",	 "LDB_A",
	"LDC_B",   "LDC_C",    "LDC_D",	  "LDC_E",    "LDC_H",	 "LDC_L",
	"LDC_HL",  "LDC_A",    "LDD_B",	  "LDD_C",    "LDD_D",	 "LDD_E",
	"LDD_H",   "LDD_L",    "LDD_HL",  "LDD_A",    "LDE_B",	 "LDE_C",
	"LDE_D",   "LDE_E",    "LDE_H",	  "LDE_L",    "LDE_HL",	 "LDE_A",
	"LDH_B",   "LDH_C",    "LDH_D",	  "LDH_E",    "LDH_H",	 "LDH_L",
	"LDH_HL",  "LDH_A",    "LDL_B",	  "LDL_C",    "LDL_D",	 "LDL_E",
	"LDL_H",   "LDL_L",    "LDL_HL",  "LDL_A",    "LDHL_B",	 "LDHL_C",
	"LDHL_D",  "LDHL_E",   "LDHL_H",  "LDHL_L",   "HALT",	 "LDHL_A",
	"LDA_B",   "LDA_C",    "LDA_D",	  "LDA_E",    "LDA_H",	 "LDA_L",
	"LDA_HL",  "LDA_A",    "ADDB",	  "ADDC",     "ADDD",	 "ADDE",
	"ADDH",	   "ADDL",     "ADDHL",	  "ADDA",     "ADCB",	 "ADCC",
	"ADCD",	   "ADCE",     "ADCH",	  "ADCL",     "ADCHL",	 "ADCA",
	"SUBB",	   "SUBC",     "SUBD",	  "SUBE",     "SUBH",	 "SUBL",
	"SUBHL",   "SUBA",     "SBCB",	  "SBCC",     "SBCD",	 "SBCE",
	"SBCH",	   "SBCL",     "SBCHL",	  "SBCA",     "ANDB",	 "ANDC",
	"ANDD",	   "ANDE",     "ANDH",	  "ANDL",     "ANDHL",	 "ANDA",
	"XORB",	   "XORC",     "XORD",	  "XORE",     "XORH",	 "XORL",
	"XORHL",   "XORA",     "ORB",	  "ORC",      "ORD",	 "ORE",
	"ORH",	   "ORL",      "ORHL",	  "ORA",      "CPB",	 "CPC",
	"CPD",	   "CPE",      "CPH",	  "CPL",      "CPHL",	 "CPA",
	"RETNZ",   "POPBC",    "JPNZ",	  "JP",	      "CALLNZ",	 "PUSHBC",
	"ADD",	   "RST00",    "RETZ",	  "RET",      "JPZ",	 "CB",
	"CALLZ",   "CALL",     "ADC",	  "RST08",    "RETNC",	 "POPDE",
	"JPNC",	   "ILL",      "CALLNC",  "PUSHDE",   "SUB",	 "RST10",
	"RETC",	   "RETI",     "JPC",	  "ILL",      "CALLC",	 "ILL",
	"SBC",	   "RST18",    "LDIOA",	  "POPHL",    "LDIOCA",	 "ILL",
	"ILL",	   "PUSHHL",   "AND",	  "RST20",    "ADDSP",	 "JPHL",
	"LDIA",	   "ILL",      "ILL",	  "ILL",      "XOR",	 "RST28",
	"LDAIO",   "POPAF",    "LDAIOC",  "DI",	      "ILL",	 "PUSHAF",
	"OR",	   "RST30",    "LDHL_SP", "LDSP_HL",  "LDAI",	 "EI",
	"ILL",	   "ILL",      "CP",	  "RST38",
};

static const char *OP_TABLES_CB_REPR[256] = {
	"RLCB",	 "RLCC",  "RLCD",  "RLCE",  "RLCH",  "RLCL",  "RLCHL",	"RLCA",
	"RRCB",	 "RRCC",  "RRCD",  "RRCE",  "RRCH",  "RRCL",  "RRCHL",	"RRCA",
	"RLB",	 "RLC",	  "RLD",   "RLE",   "RLH",   "RLL",   "RLHL",	"RLA",
	"RRB",	 "RRC",	  "RRD",   "RRE",   "RRH",   "RRL",   "RRHL",	"RRA",
	"SLAB",	 "SLAC",  "SLAD",  "SLAE",  "SLAH",  "SLAL",  "SLAHL",	"SLAA",
	"SRAB",	 "SRAC",  "SRAD",  "SRAE",  "SRAH",  "SRAL",  "SRAHL",	"SRAA",
	"SWAPB", "SWAPC", "SWAPD", "SWAPE", "SWAPH", "SWAPL", "SWAPHL", "SWAPA",
	"SRLB",	 "SRLC",  "SRLD",  "SRLE",  "SRLH",  "SRLL",  "SRLHL",	"SRLA",
	"BIT0B", "BIT0C", "BIT0D", "BIT0E", "BIT0H", "BIT0L", "BIT0HL", "BIT0A",
	"BIT1B", "BIT1C", "BIT1D", "BIT1E", "BIT1H", "BIT1L", "BIT1HL", "BIT1A",
	"BIT2B", "BIT2C", "BIT2D", "BIT2E", "BIT2H", "BIT2L", "BIT2HL", "BIT2A",
	"BIT3B", "BIT3C", "BIT3D", "BIT3E", "BIT3H", "BIT3L", "BIT3HL", "BIT3A",
	"BIT4B", "BIT4C", "BIT4D", "BIT4E", "BIT4H", "BIT4L", "BIT4HL", "BIT4A",
	"BIT5B", "BIT5C", "BIT5D", "BIT5E", "BIT5H", "BIT5L", "BIT5HL", "BIT5A",
	"BIT6B", "BIT6C", "BIT6D", "BIT6E", "BIT6H", "BIT6L", "BIT6HL", "BIT6A",
	"BIT7B", "BIT7C", "BIT7D", "BIT7E", "BIT7H", "BIT7L", "BIT7HL", "BIT7A",
	"RES0B", "RES0C", "RES0D", "RES0E", "RES0H", "RES0L", "RES0HL", "RES0A",
	"RES1B", "RES1C", "RES1D", "RES1E", "RES1H", "RES1L", "RES1HL", "RES1A",
	"RES2B", "RES2C", "RES2D", "RES2E", "RES2H", "RES2L", "RES2HL", "RES2A",
	"RES3B", "RES3C", "RES3D", "RES3E", "RES3H", "RES3L", "RES3HL", "RES3A",
	"RES4B", "RES4C", "RES4D", "RES4E", "RES4H", "RES4L", "RES4HL", "RES4A",
	"RES5B", "RES5C", "RES5D", "RES5E", "RES5H", "RES5L", "RES5HL", "RES5A",
	"RES6B", "RES6C", "RES6D", "RES6E", "RES6H", "RES6L", "RES6HL", "RES6A",
	"RES7B", "RES7C", "RES7D", "RES7E", "RES7H", "RES7L", "RES7HL", "RES7A",
	"SET0B", "SET0C", "SET0D", "SET0E", "SET0H", "SET0L", "SET0HL", "SET0A",
	"SET1B", "SET1C", "SET1D", "SET1E", "SET1H", "SET1L", "SET1HL", "SET1A",
	"SET2B", "SET2C", "SET2D", "SET2E", "SET2H", "SET2L", "SET2HL", "SET2A",
	"SET3B", "SET3C", "SET3D", "SET3E", "SET3H", "SET3L", "SET3HL", "SET3A",
	"SET4B", "SET4C", "SET4D", "SET4E", "SET4H", "SET4L", "SET4HL", "SET4A",
	"SET5B", "SET5C", "SET5D", "SET5E", "SET5H", "SET5L", "SET5HL", "SET5A",
	"SET6B", "SET6C", "SET6D", "SET6E", "SET6H", "SET6L", "SET6HL", "SET6A",
	"SET7B", "SET7C", "SET7D", "SET7E", "SET7H", "SET7L", "SET7HL", "SET7A",
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
		return OP_TABLES_REPR[opcode];
	else
		return OP_TABLES_CB_REPR[opcode];
}
