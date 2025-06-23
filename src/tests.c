#include "cpu.h"
#include "memory.h"
#include "tests.h"
#include "alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

typedef struct TestCase {
	Cpu *initial;
	Cpu *final;
	int failed;
} TestCase;

typedef struct TestSuite {
	char *name;
	int failed;
	int passed;
	int total;
	bool verbose;
} TestSuite;

TestSuite *test_suite_init()
{
	TestSuite *suite;

	if ((suite = (TestSuite *)malloc(sizeof(TestSuite))) == NULL)
		return NULL;
	suite->name = NULL;
	suite->failed = 0;
	suite->total = 0;
	suite->verbose = false;
	return suite;
}

TestCase *test_case_init()
{
	TestCase *test;

	if ((test = (TestCase *)malloc(sizeof(TestCase))) == NULL)
		return NULL;
	if ((test->initial = cpu_init()) == NULL)
		return NULL;
	if ((test->initial->memory = memory_init()) == NULL)
		return NULL;
	if ((test->final = cpu_init()) == NULL)
		return NULL;
	if ((test->final->memory = memory_init()) == NULL)
		return NULL;
	test->initial->debug = false;
	test->final->debug = false;
	test->failed = 0;
	return test;
}

void test_case_release(TestCase *test)
{
	memory_release(test->initial->memory);
	memory_release(test->final->memory);
	cpu_release(test->initial);
	cpu_release(test->final);
	zfree(test);
}

size_t get_file_size(FILE *file)
{
	size_t length;

	fseek(file, 0L, SEEK_END);
	length = ftell(file);
	fseek(file, 0L, SEEK_SET);
	return length;
}

char *test_case_read(char *path)
{
	char *buffer;
	size_t length;
	FILE *file;

	if ((file = fopen(path, "r")) == NULL)
		return NULL;
	length = get_file_size(file);
	buffer = (char *)calloc(length, sizeof(char));
	if (buffer != NULL) {
		if (fread(buffer, sizeof(char), length, file) == 0) {
			zfree(buffer);
		}
	}
	fclose(file);
	return buffer;
}

void test_case_reset_cpu_from_case(Cpu *cpu, cJSON *state)
{
	cJSON *a = cJSON_GetObjectItemCaseSensitive(state, "a");
	cJSON *b = cJSON_GetObjectItemCaseSensitive(state, "b");
	cJSON *c = cJSON_GetObjectItemCaseSensitive(state, "c");
	cJSON *d = cJSON_GetObjectItemCaseSensitive(state, "d");
	cJSON *e = cJSON_GetObjectItemCaseSensitive(state, "e");
	cJSON *f = cJSON_GetObjectItemCaseSensitive(state, "f");
	cJSON *h = cJSON_GetObjectItemCaseSensitive(state, "h");
	cJSON *l = cJSON_GetObjectItemCaseSensitive(state, "l");
	cJSON *pc = cJSON_GetObjectItemCaseSensitive(state, "pc");
	cJSON *sp = cJSON_GetObjectItemCaseSensitive(state, "sp");
	cJSON *ram = cJSON_GetObjectItemCaseSensitive(state, "ram");
	cJSON *memory = NULL, *item = NULL;

	cpu->a = a->valueint;
	cpu->b = b->valueint;
	cpu->c = c->valueint;
	cpu->d = d->valueint;
	cpu->e = e->valueint;
	cpu->f = f->valueint;
	cpu->h = h->valueint;
	cpu->l = l->valueint;
	cpu->pc = pc->valueint;
	cpu->sp = sp->valueint;
	cJSON_ArrayForEach(memory, ram)
	{
		u16 address = 0;
		u16 value = 0;
		int i = 0;
		cJSON_ArrayForEach(item, memory)
		{
			if (i == 0)
				address = item->valueint;
			else
				value = item->valueint;
			i++;
		}
		cpu->memory->bus[address] = value;
	}
}

void assert_register(TestSuite *suite, TestCase *test, const char *reg,
		     int result, int expected)
{
	if (result != expected) {
		if (suite->verbose)
			printf("[KO] %s cpu->%s (result) %X != (expected) %X\n",
			       suite->name, reg, result, expected);
		test->failed++;
	}
}

void assert_memory(TestSuite *suite, TestCase *test, Memory *result,
		   Memory *expected)
{
	for (int addr = 0; addr < 0xFFFF; addr++) {
		if (result->bus[addr] != expected->bus[addr]) {
			if (suite->verbose)
				printf("[KO] %s memory[%X] (result) %X != (expected) %X\n",
				       suite->name, addr, result->bus[addr],
				       expected->bus[addr]);
			test->failed++;
		}
	}
}

void test_case_run_cycle(cJSON *cycle, Cpu *cpu)
{
	// cJSON *member = NULL;
	// int i = 0;
	//
	// cJSON_ArrayForEach(member, cycle)
	// {
	// 	if (i == 0)
	// 		cpu->pc = (u16)member->valueint;
	// 	else if (i == 1)
	// 		cpu->memory->bus[cpu->pc] = (u8)member->valueint;
	// 	i++;
	// }
	cpu_execute(cpu, cpu_fetch(cpu));
}

void test_case_assert(TestCase *test, TestSuite *suite)
{
	assert_register(suite, test, "a", test->initial->a, test->final->a);
	assert_register(suite, test, "b", test->initial->b, test->final->b);
	assert_register(suite, test, "c", test->initial->c, test->final->c);
	assert_register(suite, test, "d", test->initial->d, test->final->d);
	assert_register(suite, test, "e", test->initial->e, test->final->e);
	assert_register(suite, test, "f", test->initial->f, test->final->f);
	assert_register(suite, test, "h", test->initial->h, test->final->h);
	assert_register(suite, test, "l", test->initial->l, test->final->l);
	assert_register(suite, test, "pc", test->initial->pc, test->final->pc);
	assert_register(suite, test, "sp", test->initial->sp, test->final->sp);
	assert_memory(suite, test, test->initial->memory, test->final->memory);
	if (test->failed > 0)
		suite->failed++;
	else
		suite->passed++;
}

TestSuite *test_suite_run(char *path, TestSuite *suite)
{
	char *buffer;
	cJSON *json = NULL, *test_case = NULL, *cycle = NULL;

	if ((buffer = test_case_read(path)) == NULL)
		return NULL;
	json = cJSON_Parse(buffer);
	suite->total = cJSON_GetArraySize(json);
	suite->name = path;
	cJSON_ArrayForEach(test_case, json)
	{
		TestCase *test = test_case_init();
		if (test == NULL) {
			printf("Failed to init test case\n");
			exit(-1);
		}
		cJSON *initial =
			cJSON_GetObjectItemCaseSensitive(test_case, "initial");
		cJSON *cycles =
			cJSON_GetObjectItemCaseSensitive(test_case, "cycles");
		cJSON *final =
			cJSON_GetObjectItemCaseSensitive(test_case, "final");
		test_case_reset_cpu_from_case(test->initial, initial);
		cJSON_ArrayForEach(cycle, cycles)
		{
			test_case_run_cycle(cycle, test->initial);
		}
		test_case_reset_cpu_from_case(test->final, final);
		test_case_assert(test, suite);
		test_case_release(test);
	}
	cJSON_Delete(json);
	return suite;
}

void test_cpu(bool verbose)
{
	int total = 0;
	int total_failed = 0;
	int total_passed = 0;
	double percentage;
	int opcode;

	printf("# Testing cpu.c\n");
	for (opcode = 0x00; opcode <= 0x01; opcode++) {
		TestSuite *suite;
		char path[50];
		char *opcode_decoded = cpu_opcode_to_string(opcode);

		suite = test_suite_init();
		suite->verbose = verbose;
		if (opcode == 0xCB)
			continue;
		sprintf(path, "sm83/v1/%02x.json", opcode);
		if ((suite = test_suite_run(path, suite)) == NULL)
			continue;
		if (suite->passed != suite->total) {
			percentage =
				((double)suite->passed / suite->total) * 100;
			printf("[FAILED] %s %12s %2.0f%% passed %3d/%3d failed\n",
			       suite->name, opcode_decoded, percentage,
			       suite->failed, suite->total);
		}
		total += suite->total;
		total_failed += suite->failed;
		total_passed += suite->passed;
		zfree(suite);
	}
	percentage = ((double)total_failed / total) * 100;
	printf("Total %2.2f%% failed %d/%d passed\n", percentage, total_passed,
	       total);
}
