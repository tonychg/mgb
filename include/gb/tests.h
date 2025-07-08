#ifndef __TEST_H__
#define __TEST_H__

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include "cpu.h"

typedef struct TestCase {
	Cpu *initial;
	Cpu *final;
	int failed;
	int index;
} TestCase;

typedef struct TestSuite {
	char *name;
	int failed;
	int passed;
	int total;
	bool verbose;
} TestSuite;

void test_memory();
void test_cpu(bool verbose);
TestSuite *test_opcode(int opcode, bool verbose, bool is_prefixed);

#endif
