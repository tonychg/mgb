#ifndef _TEST_H
#define _TEST_H

#include <assert.h>
#include <stdbool.h>
#include "cpu.h"

struct test_case {
	struct cpu *initial;
	struct cpu *final;
	int failed;
	int index;
};

struct test_suite {
	char *name;
	int failed;
	int passed;
	int total;
	bool verbose;
};

void test_memory();
void test_cpu(bool verbose);
struct test_suite *test_opcode(int opcode, bool verbose, bool is_prefixed);

#endif
